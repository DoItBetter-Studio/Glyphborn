#include "sketch.h"

static void sketch_draw_pixel(int x, int y, uint32_t color)
{
	if (x >= 0 && x < FB_WIDTH && y >= 0 && y < FB_HEIGHT)
	{
		framebuffer_game[y * FB_WIDTH + x] = color;
	}
}

static void sketch_draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;
	while (true)
	{
		sketch_draw_pixel(x0, y0, color);
		if (x0 == x1 && y0 == y1) break;
		int e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
	}
}

void sketch_draw_wireframe(RenderMesh* mesh, Mat4 model, Mat4 view, Mat4 projection, uint32_t color)
{
	Vec3 projected[MAX_VERTICES];

	for (int i = 0; i < mesh->vertex_count; i++)
	{
		Vec4 v = { mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z, 1.0f };
		v = mat4_mul_vec4(model, v);
		v = mat4_mul_vec4(view, v);
		v = mat4_mul_vec4(projection, v);

		v.x /= v.w;
		v.y /= v.w;

		projected[i].x = (v.x + 1.0f) * 0.5f * FB_WIDTH;
		projected[i].y = (1.0f - (v.y + 1.0f) * 0.5f) * FB_HEIGHT; // Invert Y for top-down
	}

	for (int i = 0; i < mesh->edge_count; i++)
	{
		int v0 = mesh->edges[i][0];
		int v1 = mesh->edges[i][1];
		sketch_draw_line((int)projected[v0].x, (int)projected[v0].y,
			(int)projected[v1].x, (int)projected[v1].y, color);
	}
}