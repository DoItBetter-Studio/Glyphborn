#include "ui/ui_render.h"
#include "render.h"
#include "font/ascii_tileset.h"

void ui_draw_image(int x, int y, int width, int height, const unsigned char* image_data, const unsigned char* palette, int depth)
{
	int mask = (1 << depth) - 1;

	int total_pixels = width * height;
	int data_index = 0;

	for (int j = 0; j < height && (y + j) < fb_height; j++)
	{
		for (int i = 0; i < width && (x + i) < fb_width; i++)
		{
			if (data_index >= total_pixels)
				return;

			int pixel_byte_index = (data_index * depth) / 8;
			int bit_shift = 8 - depth - ((data_index * depth) % 8);

			uint8_t byte = image_data[pixel_byte_index];
			uint8_t index = (byte >> bit_shift) & mask;

			uint8_t r = palette[index * 4 + 0];
			uint8_t g = palette[index * 4 + 1];
			uint8_t b = palette[index * 4 + 2];
			uint8_t a = palette[index * 4 + 3];

			uint32_t color = (a << 24) | (r << 16) | (g << 8) | b;
			framebuffer_ui[(y + j) * fb_width + (x + i)] = color;

			data_index++;
		}
	}
}

void ui_draw_text(int x, int y, const char* text)
{
	ui_draw_text_colored(x, y, text, 0xFF000000);
}

void ui_draw_text_colored(int x, int y, const char* text, uint32_t color)
{
	int cursor_x = x;
	int cursor_y = y;

	while (*text)
	{
		char c = *text++;

		if (c == '\n')
		{
			cursor_x = x;
			cursor_y += ASCII_TILESET_GLYPH_HEIGHT;
			continue;
		}

		int glyph_index = (unsigned char)c;
		int glyph_col = glyph_index % ASCII_TILESET_SHEET_COLUMNS;
		int glyph_row = glyph_index / ASCII_TILESET_SHEET_COLUMNS;

		const int sheet_width = ASCII_TILESET_SHEET_COLUMNS * ASCII_TILESET_GLYPH_WIDTH;
		const int mask = (1 << ASCII_TILESET_BITDEPTH) - 1;

		int draw_width = ascii_tileset_widths[(unsigned char)c];

		for (int j = 0; j < ASCII_TILESET_GLYPH_HEIGHT && (cursor_y + j) < fb_height; ++j)
		{
			for (int i = 0; i < draw_width && (cursor_x + i) < fb_width; ++i)
			{
				int pixel_index = j * sheet_width + i + glyph_col * ASCII_TILESET_GLYPH_WIDTH + glyph_row * sheet_width * ASCII_TILESET_GLYPH_HEIGHT;
				int bit_index = pixel_index * ASCII_TILESET_BITDEPTH;

				int byte_index = bit_index / 8;
				int bit_shift = 8 - ASCII_TILESET_BITDEPTH - (bit_index % 8);

				uint8_t byte = ascii_tileset[byte_index];
				uint8_t index = (byte >> bit_shift) & mask;

				if (index == 0)
					continue;

				framebuffer_ui[(cursor_y + j) * fb_width + (cursor_x + i)] = color;
			}
		}

		cursor_x += draw_width + ASCII_TILESET_SPACING;
	}
}

int ui_text_width(const char* text)
{
	int label_width = 0;
	for (const char* c = text; *c; ++c)
		label_width += ascii_tileset_widths[(unsigned char)*c] + ASCII_TILESET_SPACING;

	return label_width;
}

int ui_text_height(const char* text, int max_width)
{
	int line_width = 0;
	int total_height = ASCII_TILESET_GLYPH_HEIGHT;

	for (const char* c = text; *c; ++c)
	{
		if (*c == '\n')
		{
			line_width = 0;
			total_height += ASCII_TILESET_GLYPH_HEIGHT;
			continue;
		}

		int glyph_width = ascii_tileset_widths[(unsigned char)*c] + ASCII_TILESET_SPACING;

		if (line_width + glyph_width > max_width)
		{
			line_width = 0;
			total_height += ASCII_TILESET_GLYPH_HEIGHT;
		}

		line_width += glyph_width;
	}

	return total_height;
}

static void blit_tiled_region(int dst_x, int dst_y, int dst_w, int dst_h, int src_x, int src_y, int src_w, int src_h, const unsigned char* pixels, const unsigned char* palette, int image_w, int image_h, int depth)
{
	int mask = (1 << depth) - 1;

	for (int dy = 0; dy < dst_h; ++dy) {
		for (int dx = 0; dx < dst_w; ++dx) {
			// Tile source coordinates
			int tx = dx % src_w;
			int ty = dy % src_h;
			int src_px_x = src_x + tx;
			int src_px_y = src_y + ty;

			// Bounds check
			if (src_px_x >= image_w || src_px_y >= image_h)
				continue;

			int pixel_index = src_px_y * image_w + src_px_x;
			int byte_index = (pixel_index * depth) / 8;
			int bit_shift = 8 - depth - ((pixel_index * depth) % 8);

			uint8_t byte = pixels[byte_index];
			uint8_t index = (byte >> bit_shift) & mask;

			uint8_t r = palette[index * 4 + 0];
			uint8_t g = palette[index * 4 + 1];
			uint8_t b = palette[index * 4 + 2];
			uint8_t a = palette[index * 4 + 3];

			int dst_px_x = dst_x + dx;
			int dst_px_y = dst_y + dy;

			if (dst_px_x < 0 || dst_px_x >= fb_width || dst_px_y < 0 || dst_px_y >= fb_height)
				continue;

			uint32_t dst = framebuffer_ui[dst_px_y * fb_width + dst_px_x];

			uint8_t src_r = r, src_g = g, src_b = b, src_a = a;
			uint8_t dst_r = (dst >> 16) & 0xFF;
			uint8_t dst_g = (dst >> 8) & 0xFF;
			uint8_t dst_b = dst & 0xFF;
			uint8_t dst_a = (dst >> 24) & 0xFF;

			float alpha = src_a / 255.0f;
			uint8_t out_r = (uint8_t)(src_r * alpha + dst_r * (1.0f - alpha));
			uint8_t out_g = (uint8_t)(src_g * alpha + dst_g * (1.0f - alpha));
			uint8_t out_b = (uint8_t)(src_b * alpha + dst_b * (1.0f - alpha));
			uint8_t out_a = 255; // Or blend alpha if you want

			framebuffer_ui[dst_px_y * fb_width + dst_px_x] =
				(out_a << 24) | (out_r << 16) | (out_g << 8) | out_b;
		}
	}
}

void ui_draw_nineslice(int dst_x, int dst_y, int dst_w, int dst_h, const unsigned char* image_data, const unsigned char* palette, int depth, int src_w, int src_h, int slice_left, int slice_top, int slice_right, int slice_bottom)
{
	int center_src_w = src_w - slice_left - slice_right;
	int center_src_h = src_h - slice_top - slice_bottom;

	int center_dst_w = dst_w - slice_left - slice_right;
	int center_dst_h = dst_h - slice_top - slice_bottom;

	// Corners
	blit_tiled_region(dst_x, dst_y, slice_left, slice_top, 0, 0, slice_left, slice_top, image_data, palette, src_w, src_h, depth); // top-left
	blit_tiled_region(dst_x + dst_w - slice_right, dst_y, slice_right, slice_top, src_w - slice_right, 0, slice_right, slice_top, image_data, palette, src_w, src_h, depth); // top-right
	blit_tiled_region(dst_x, dst_y + dst_h - slice_bottom, slice_left, slice_bottom, 0, src_h - slice_bottom, slice_left, slice_bottom, image_data, palette, src_w, src_h, depth); // bottom-left
	blit_tiled_region(dst_x + dst_w - slice_right, dst_y + dst_h - slice_bottom, slice_right, slice_bottom, src_w - slice_right, src_h - slice_bottom, slice_right, slice_bottom, image_data, palette, src_w, src_h, depth); // bottom-right

	// Edges
	blit_tiled_region(dst_x + slice_left, dst_y, center_dst_w, slice_top, slice_left, 0, center_src_w, slice_top, image_data, palette, src_w, src_h, depth); // top
	blit_tiled_region(dst_x + slice_left, dst_y + dst_h - slice_bottom, center_dst_w, slice_bottom, slice_left, src_h - slice_bottom, center_src_w, slice_bottom, image_data, palette, src_w, src_h, depth); // bottom
	blit_tiled_region(dst_x, dst_y + slice_top, slice_left, center_dst_h, 0, slice_top, slice_left, center_src_h, image_data, palette, src_w, src_h, depth); // left
	blit_tiled_region(dst_x + dst_w - slice_right, dst_y + slice_top, slice_right, center_dst_h, src_w - slice_right, slice_top, slice_right, center_src_h, image_data, palette, src_w, src_h, depth); // right

	// Center
	blit_tiled_region(dst_x + slice_left, dst_y + slice_top, center_dst_w, center_dst_h, slice_left, slice_top, center_src_w, center_src_h, image_data, palette, src_w, src_h, depth);
}
