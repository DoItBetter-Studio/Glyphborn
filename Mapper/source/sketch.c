#include "sketch.h"
#include "maths/vec4.h"

extern int fb_mapview_width;
extern int fb_mapview_height;
extern uint32_t* framebuffer_mapview;
extern float* depthbuffer_mapview;

static Vec3* projected_buffer = NULL;
static size_t projected_capacity = 0;

static void sketch_draw_pixel(int x, int y, float z, uint32_t color)
{
	if (x < 0 || y < 0 || x >= fb_mapview_width || y >= fb_mapview_height)
		return;

	int index = y * fb_mapview_width + x;

	if (z < depthbuffer_mapview[index])
	{
		depthbuffer_mapview[index] = z;
		framebuffer_mapview[index] = color;
	}
}

static void sketch_draw_line(int x0, int y0, float z0, int x1, int y1, float z1, uint32_t color)
{
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;

	int steps = dx > -dy ? dx : -dy;
	float dz = steps > 0 ? (z1 - z0) / (float)steps : 0.0f;
	float z = z0;

	while (true)
	{
		sketch_draw_pixel(x0, y0, z, color);

		if (x0 == x1 && y0 == y1) break;

		int e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }

		z += dz;
	}
}

void sketch_draw_wireframe(RenderMesh* mesh, Mat4 model, Mat4 view, Mat4 projection, uint32_t color)
{
	Vec3* projected = malloc(sizeof(Vec3) * mesh->vertex_count);
	if (!projected) return;

	for (int i = 0; i < mesh->vertex_count; i++)
	{
		Vec4 v = { mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z, 1.0f };
		v = mat4_mul_vec4(model, v);
		v = mat4_mul_vec4(view, v);
		v = mat4_mul_vec4(projection, v);

		v.x /= v.w;
		v.y /= v.w;
		v.z /= v.w;

		projected[i].x = (v.x + 1.0f) * 0.5f * (float)fb_mapview_width;
		projected[i].y = (1.0f - (v.y + 1.0f) * 0.5f) * (float)fb_mapview_height; // Invert Y for top-down
		projected[i].z = v.z;
	}

	for (int i = 0; i < mesh->index_count; i += 3)
	{
		int i0 = mesh->indices[i + 0];
		int i1 = mesh->indices[i + 1];
		int i2 = mesh->indices[i + 2];

		sketch_draw_line((int)projected[i0].x, (int)projected[i0].y, projected[i0].z,
			(int)projected[i1].x, (int)projected[i1].y, projected[i1].z, color);
		sketch_draw_line((int)projected[i1].x, (int)projected[i1].y, projected[i1].z,
			(int)projected[i2].x, (int)projected[i2].y, projected[i2].z, color);
		sketch_draw_line((int)projected[i2].x, (int)projected[i2].y, projected[i2].z,
			(int)projected[i0].x, (int)projected[i0].y, projected[i0].z, color);
	}

	free(projected);
}

void sketch_draw_grid(int size_x, int y_index, int size_z, Mat4 view, Mat4 projection)
{
	Vec3* projected = malloc(sizeof(Vec3) * (size_x * size_z));
	if (!projected) return;

	int half_x = size_x / 2;
	int half_z = size_z / 2;

	for (int x = -half_x; x <= half_x; ++x)
	{
		Vec4 a = { (float)x, y_index, (float)-half_z, 1.0f};
		Vec4 b = { (float)x, y_index, (float)half_z, 1.0f };
		a = mat4_mul_vec4(view, a);
		b = mat4_mul_vec4(view, b);
		a = mat4_mul_vec4(projection, a);
		b = mat4_mul_vec4(projection, b);

		a.x /= a.w;	a.y /= a.w; a.z /= a.w;
		b.x /= b.w;	b.y /= b.w; b.z /= b.w;

		int x0 = (int)((a.x + 1.0f) * 0.5f * fb_mapview_width);
		int y0 = (int)((1.0f - (a.y + 1.0f) * 0.5f) * fb_mapview_height);
		float z0 = a.z;

		int x1 = (int)((b.x + 1.0f) * 0.5f * fb_mapview_width);
		int y1 = (int)((1.0f - (b.y + 1.0f) * 0.5f) * fb_mapview_height);
		float z1 = b.z;

		sketch_draw_line(x0, y0, z0, x1, y1, z1, 0xFFFF0000);
	}

	for (int z = -half_z; z <= half_z; ++z)
	{
		Vec4 a = { (float)-half_x, y_index, (float)z, 1.0f };
		Vec4 b = { (float)half_x, y_index, (float)z, 1.0f };
		a = mat4_mul_vec4(view, a);
		b = mat4_mul_vec4(view, b);
		a = mat4_mul_vec4(projection, a);
		b = mat4_mul_vec4(projection, b);

		a.x /= a.w;	a.y /= a.w; a.z /= a.w;
		b.x /= b.w;	b.y /= b.w; b.z /= b.w;

		int x0 = (int)((a.x + 1.0f) * 0.5f * fb_mapview_width);
		int y0 = (int)((1.0f - (a.y + 1.0f) * 0.5f) * fb_mapview_height);
		float z0 = a.z;

		int x1 = (int)((b.x + 1.0f) * 0.5f * fb_mapview_width);
		int y1 = (int)((1.0f - (b.y + 1.0f) * 0.5f) * fb_mapview_height);
		float z1 = b.z;

		sketch_draw_line(x0, y0, z0, x1, y1, z1, 0xFF0000FF);
	}

	free(projected);
}
