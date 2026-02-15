/*
 * sketch.c - Software rasterizer and simple debugging drawing utilities
 *
 * Provides triangle rasterization, wireframe drawing, grid drawing and helpers
 * used by the render pipeline. Operates on `framebuffer_game` and uses the
 * shared `depthbuffer` for per-pixel depth tests.
 */

#include "sketch.h"
#include "render.h"
#include "maths/vec4.h"
#include "lighting/directional_light.h"
#include <string.h>
#include <math.h>

#include <stdio.h>

/* External depth buffer used for depth testing (defined in platform renderer)
 * Each pixel holds the current minimum depth; lower values are closer.
 */
extern uint32_t framebuffer_game[FB_WIDTH * FB_HEIGHT];
extern float depthbuffer[FB_WIDTH * FB_HEIGHT];

extern DirectionalLight sun;

static FILE* debug_log = NULL;

static bool show_uvs = false;

/* =================================
 * Internal raster vertex
  ================================== */
typedef struct {
	float x, y;
	float z_over_w;
	float u_over_w;
	float v_over_w;
	float inv_w;
} RasterVert;

/* =================================
 * Utility
  ================================== */
static inline float edge(float ax, float ay, float bx, float by, float cx, float cy)
{
	return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

void sketch_show_uvs(bool showUVs)
{
	show_uvs = showUVs;
}

void sketch_clear(uint32_t clear_color)
{
	for (int i = 0; i < FB_WIDTH * FB_HEIGHT; i++)
	{
		framebuffer_game[i] = clear_color;
		depthbuffer[i] = 1.0f;
	}
}

typedef struct {
    Vec4 p;   // clip-space position
    float u;
    float v;
} ClipVert;

typedef struct {
    ClipVert v[3];
} ClipTri;

static ClipVert clip_lerp(const ClipVert* a, const ClipVert* b, float t)
{
    ClipVert r;

    r.p.x = a->p.x + t * (b->p.x - a->p.x);
    r.p.y = a->p.y + t * (b->p.y - a->p.y);
    r.p.z = a->p.z + t * (b->p.z - a->p.z);
    r.p.w = a->p.w + t * (b->p.w - a->p.w);

    r.u = a->u + t * (b->u - a->u);
    r.v = a->v + t * (b->v - a->v);

    return r;
}

// Clip a single triangle against the near plane z = -w in clip space.
// Returns 0, 1, or 2 output triangles in 'out'.
static int clip_triangle_near(const ClipVert in[3], ClipTri out[2])
{
    float d[3];
    int inside[3];
    int inside_count = 0;

    for (int i = 0; i < 3; i++) {
        d[i] = in[i].p.z + in[i].p.w;   // >0 = inside
        inside[i] = d[i] > 0.0f;
        if (inside[i]) inside_count++;
    }

    if (inside_count == 0)
        return 0;

    if (inside_count == 3) {
        out[0].v[0] = in[0];
        out[0].v[1] = in[1];
        out[0].v[2] = in[2];
        return 1;
    }

    // helper function for interpolation
    ClipVert (*interp)(const ClipVert*, const ClipVert*, float) = clip_lerp;

    if (inside_count == 1) {
        int i0 = inside[0] ? 0 : (inside[1] ? 1 : 2);
        int i1 = (i0 + 1) % 3;
        int i2 = (i0 + 2) % 3;

        float t1 = d[i0] / (d[i0] - d[i1]);
        float t2 = d[i0] / (d[i0] - d[i2]);

        out[0].v[0] = in[i0];
        out[0].v[1] = interp(&in[i0], &in[i1], t1);
        out[0].v[2] = interp(&in[i0], &in[i2], t2);

        return 1;
    }

    if (inside_count == 2) {
        int o  = inside[0] ? (inside[1] ? 2 : 1) : 0;
        int i0 = (o + 1) % 3;
        int i1 = (o + 2) % 3;

        float t0 = d[i0] / (d[i0] - d[o]);
        float t1 = d[i1] / (d[i1] - d[o]);

        ClipVert v0 = in[i0];
        ClipVert v1 = in[i1];
        ClipVert v2 = interp(&in[i0], &in[o], t0);
        ClipVert v3 = interp(&in[i1], &in[o], t1);

        // First triangle
        out[0].v[0] = v0;
        out[0].v[1] = v1;
        out[0].v[2] = v2;

        // Second triangle
        out[1].v[0] = v1;
        out[1].v[1] = v3;
        out[1].v[2] = v2;

        return 2;
    }

    return 0;
}

static void draw_triangle(RasterVert a, RasterVert b, RasterVert c, const RasterMesh* mesh, float light)
{
	float area = edge(a.x, a.y, b.x, b.y, c.x, c.y);
	if (fabsf(area) < 1e-6f) return;

	int minX = (int)fmaxf(0.0f, floorf(fminf(a.x, fminf(b.x, c.x))));
	int maxX = (int)fminf(FB_WIDTH - 1.0f, ceilf(fmaxf(a.x, fmaxf(b.x, c.x))));
	int minY = (int)fmaxf(0.0f, floorf(fminf(a.y, fminf(b.y, c.y))));
	int maxY = (int)fminf(FB_HEIGHT - 1.0f, ceilf(fmaxf(a.y, fmaxf(b.y, c.y))));

	if (!mesh->pixels || mesh->tex_width == 0 || mesh->tex_height == 0) {
		if (debug_log) {
			fprintf(debug_log, "Invalid texture! Skipping mesh\n");
			fflush(debug_log);
		}
		return;
	}

	for (int y = minY; y <= maxY; y++)
	for (int x = minX; x <= maxX; x++)
	{
		float px = x + 0.5f;
		float py = y + 0.5f;

		float w0 = edge(b.x, b.y, c.x, c.y, px, py) / area;
		float w1 = edge(c.x, c.y, a.x, a.y, px, py) / area;
		float w2 = 1.0f - w0 - w1;

		if (w0 < 0 || w1 < 0 || w2 < 0)
			continue;

		// Interpolate depth (already perspective-corrected)
		float z = a.z_over_w * w0 + b.z_over_w * w1 + c.z_over_w * w2;

		// Map from [-1, 1] to [0, 1] for depth buffer
		z = 0.5f * z + 0.5f;

		int idx = y * FB_WIDTH + x;
		if (z >= depthbuffer[idx]) continue;
		depthbuffer[idx] = z;

		// Perspective-correct interpolation for UVs
		float inv_w = 
			a.inv_w * w0 +
			b.inv_w * w1 +
			c.inv_w * w2;

		if (inv_w <= 0.0f) continue;

		float u =
			(a.u_over_w * w0 +
			 b.u_over_w * w1 +
			 c.u_over_w * w2) / inv_w;

		float v =
			(a.v_over_w * w0 +
			 b.v_over_w * w1 +
			 c.v_over_w * w2) / inv_w;

		u = fminf(fmaxf(u, 0.0f), 1.0f);
		v = fminf(fmaxf(v, 0.0f), 1.0f);

		int tx = (int)(u * (mesh->tex_width  - 1));
		int ty = (int)(v * (mesh->tex_height - 1));

		if (!show_uvs)
		{
			uint32_t tex = mesh->pixels[ty * mesh->tex_width + tx];

			uint8_t r = (tex >> 16) & 0xFF;
			uint8_t g = (tex >> 8) & 0xFF;
			uint8_t b = tex & 0xFF;

			r = (uint8_t)(r * light);
			g = (uint8_t)(g * light);
			b = (uint8_t)(b * light);

			framebuffer_game[idx] = (0xFF << 24) | (r << 16) | (g << 8) | b;
		}
		else
		{
			uint8_t ru = (uint8_t)(u * 255);
			uint8_t gv = (uint8_t)(v * 255);
			uint32_t color = (ru << 16) | (gv << 8) | 0xFF;
			framebuffer_game[idx] = color;
		}
	}
}

static RasterVert clipvert_to_rastervert(const ClipVert* cv)
{
    RasterVert r;

    Vec4 p = cv->p;
    float inv_w = 1.0f / p.w;

    float x_ndc = p.x * inv_w;
    float y_ndc = p.y * inv_w;
    float z_ndc = p.z * inv_w;

    r.x = (x_ndc + 1.0f) * 0.5f * FB_WIDTH;
    r.y = (1.0f - (y_ndc + 1.0f) * 0.5f) * FB_HEIGHT;

    r.inv_w    = inv_w;
    r.z_over_w = z_ndc;                // already perspective-correct
    r.u_over_w = cv->u * inv_w;
    r.v_over_w = cv->v * inv_w;

    return r;
}

static bool once = false;

void sketch_draw_mesh(const RasterMesh* mesh, Mat4 model, Mat4 view, Mat4 projection)
{
    if (!debug_log) {
        debug_log = fopen("sketch_debug.txt", "w");
        if (debug_log) {
            fprintf(debug_log, "=== Glyphborn Debug Log ===\n");
            fflush(debug_log);
        }
    }

    if (!once) {
        once = true;
        if (debug_log) {
            fprintf(debug_log, "sketch_draw_mesh called\n");
            fflush(debug_log);
        }
    }

    for (uint32_t i = 0; i < mesh->index_count; i += 3)
    {
		Vec3 wp[3];
		for (int k = 0; k < 3; ++k)
		{
			const RasterVertex* rv = &mesh->vertices[mesh->indices[i + k]];

			Vec4 p = { rv->x, rv->y, rv->z, 1.0f };
			p = mat4_mul_vec4(model, p);

			wp[k] = (Vec3){ p.x, p.y, p.z };
		}

		Vec3 e1 = vec3_sub(wp[1], wp[0]);
		Vec3 e2 = vec3_sub(wp[2], wp[0]);
		Vec3 normal = vec3_cross(e1, e2);

		float light_factor;

		float len_sq = normal.x * normal.x + normal.y * normal.y + normal.z * normal.z;
		if (len_sq < 1e-6f) {
			light_factor = sun.ambient;
		}
		else
		{
			normal = vec3_normalize(normal);

			float ndotl = vec3_dot(normal, vec3_neg(sun.dir));
			float diffuse = fmaxf(0.0f, ndotl);

			light_factor = sun.ambient + diffuse * sun.intensity;
			light_factor = fminf(fmaxf(light_factor, 0.0f), 1.0f);
		}

        // 1) Build three ClipVerts in clip space
        ClipVert in[3];
        for (int k = 0; k < 3; ++k) {
            const RasterVertex* rv = &mesh->vertices[mesh->indices[i + k]];

            Vec4 p = { rv->x, rv->y, rv->z, 1.0f };
            p = mat4_mul_vec4(model, p);
            p = mat4_mul_vec4(view,  p);
            p = mat4_mul_vec4(projection, p);  // clip space

            in[k].p = p;
            in[k].u = rv->u;
            in[k].v = rv->v;
        }

        // 2) Clip against near plane
        ClipTri clipped[2];
        int tri_count = clip_triangle_near(in, clipped);
        if (tri_count == 0)
            continue;

        // 3) For each resulting triangle, convert to RasterVert and draw
        for (int t = 0; t < tri_count; ++t) {
            RasterVert a = clipvert_to_rastervert(&clipped[t].v[0]);
            RasterVert b = clipvert_to_rastervert(&clipped[t].v[1]);
            RasterVert c = clipvert_to_rastervert(&clipped[t].v[2]);

            draw_triangle(a, b, c, mesh, light_factor);
        }
    }
}