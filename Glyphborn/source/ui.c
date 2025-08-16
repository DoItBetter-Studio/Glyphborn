#include "ui.h"
#include <stdlib.h>
#include <string.h>

void ui_init(UISystem* ui)
{
	ui->count = 0;
	ui->focused_element = -1;
	ui->input_locked = false;
	memset(ui->elements, 0, sizeof(ui->elements));
}

void ui_add_element(UISystem* ui, UIElement* element)
{
	if (ui->count >= MAX_UI_ELEMENTS) return;
	ui->elements[ui->count++] = element;
}

void ui_handle_navigation(UISystem* ui, int nav_dx, int nav_dy, bool activate)
{
	if (ui->count == 0) return;

	int focusable_indices[MAX_UI_ELEMENTS];
	int focusable_count = 0;
	for (int i = 0; i < ui->count; ++i)
	{
		UIElement* element = ui->elements[i];
		if (element-> visible && element->type == UI_BUTTON)
		{
			focusable_indices[focusable_count++] = i;
		}
	}

	if (focusable_count == 0) return;

	int current_focus = 0;
	for (int i = 0; i < focusable_count; ++i)
	{
		if (ui->focused_element == focusable_indices[i])
		{
			current_focus = i;
			break;
		}
	}

	if (nav_dy != 0)
	{
		int old_focus_index = focusable_indices[current_focus];
		UIButton* old_btn = (UIButton*)ui->elements[old_focus_index]->user_data;
		if (old_btn) old_btn->focused = false; // Unfocus old button

		current_focus += nav_dy;
		if (current_focus < 0) current_focus = 0;
		if (current_focus >= focusable_count) current_focus = focusable_count - 1;

		ui->focused_element = focusable_indices[current_focus];
	}

	UIElement* focused_element = ui->elements[ui->focused_element];
	if (!focused_element || !focused_element->user_data) return;

	UIButton* focused_button = (UIButton*)focused_element->user_data;
	if (focused_button) focused_button->focused = true; // Focus new button

	if (activate && focused_button && focused_button->on_click)
	{
		focused_button->pressed = true; // Set pressed state
		focused_button->on_click(focused_button);
	}
}

void ui_update(UISystem* ui, float delta_time, int mouse_x, int mouse_y, bool mouse_pressed, int nav_dx, int nav_dy, bool activate)
{
	ui_handle_navigation(ui, nav_dx, nav_dy, activate); // No navigation input by default

	for (int i = 0; i < ui->count; ++i)
	{
		UIElement* element = ui->elements[i];
		if (!element->visible) continue;

		if (element->update_func)
			element->update_func(element, delta_time);
		
		bool ignore_mouse = false;
		if (element->type == UI_BUTTON && ((UIButton*)element->user_data)->focused)
			ignore_mouse = true; // Ignore mouse input for focused button to prevent conflicts

		if (element->handle_input_func)
			element->handle_input_func(element, mouse_x, mouse_y, mouse_pressed);
	}
}

void ui_render(UISystem* ui)
{
	for (int i = 0; i < ui->count; ++i)
	{
		UIElement* element = ui->elements[i];
		if (!element->visible) continue;
		if (element->render_func)
			element->render_func(element);
	}
}

void ui_clear(UISystem* ui)
{
	for (int i = 0; i < ui->count; ++i)
	{
		free(ui->elements[i]);
	}
	ui->count = 0;
}

// -------------------- Button Element --------------------
static void button_render(UIElement* element)
{
	UIButton* btn = (UIButton*)element->user_data;

	uint32_t color = btn->bg_color;
	if (btn->focused)
		color = 0xFFAAAAAA; // Yellow for focused

	render_draw_rect(element->x, element->y, element->width, element->height, color);
	int text_x = element->x + 4;
	int text_y = element->y + 4;
	render_draw_text_colored(text_x, text_y, btn->text, btn->fg_color);
}

static void button_update(UIElement* element, float delta_time)
{
}

static void button_input(UIElement* element, int mouse_x, int mouse_y, bool mouse_down)
{
	UIButton* btn = (UIButton*)element->user_data;
	bool inside = mouse_x >= element->x && mouse_x < element->x + element->width &&
		mouse_y >= element->y && mouse_y < element->y + element->height;
	if (inside && mouse_down)
	{
		btn->pressed = true;
		if (btn->on_click)
			btn->on_click(btn);
	}
	else
	{
		btn->pressed = false;
	}
}

UIElement* ui_create_button(int x, int y, int width, int height, const char* text, uint32_t bg_color, uint32_t fg_color, void (*on_click)(void* user_data))
{
	UIElement* element = malloc(sizeof(UIElement));
	UIButton* button = malloc(sizeof(UIButton));
	button->text = text;
	button->bg_color = bg_color;
	button->fg_color = fg_color;
	button->pressed = false;
	button->on_click = on_click;

	element->type = UI_BUTTON;
	element->x = x;
	element->y = y;
	element->width = width;
	element->height = height;
	element->visible = true;
	element->render_func = button_render;
	element->update_func = button_update;
	element->handle_input_func = button_input;
	element->user_data = button;
	return element;
}

// -------------------- Label Element --------------------
static void label_render(UIElement* element)
{
	UILabel* label = (UILabel*)element->user_data;
	render_draw_text_colored(element->x, element->y, label->text, label->color);
}

UIElement* ui_create_label(int x, int y, const char* text, uint32_t color)
{
	UIElement* element = malloc(sizeof(UIElement));
	UILabel* label = malloc(sizeof(UILabel));
	label->text = text;
	label->color = color;

	element->type = UI_LABEL;
	element->x = x;
	element->y = y;
	element->width = 0; // Width is not used for labels
	element->height = 0; // Height is not used for labels
	element->visible = true;
	element->render_func = label_render;
	element->update_func = NULL; // Labels do not need updates
	element->handle_input_func = NULL; // Labels do not handle input
	element->user_data = label;
	return element;
}

// -------------------- Panel Element --------------------
static void panel_render(UIElement* element)
{
	UIPanel* panel = (UIPanel*)element->user_data;
	render_draw_rect(element->x, element->y, element->width, element->height, panel->bg_color);
}

UIElement* ui_create_panel(int x, int y, int width, int height, uint32_t bg_color)
{
	UIElement* element = malloc(sizeof(UIElement));
	UIPanel* panel = malloc(sizeof(UIPanel));
	panel->bg_color = bg_color;

	element->type = UI_PANEL;
	element->x = x;
	element->y = y;
	element->width = width;
	element->height = height;
	element->visible = true;
	element->render_func = panel_render;
	element->update_func = NULL; // Panels do not need updates
	element->handle_input_func = NULL; // Panels do not handle input
	element->user_data = panel;
	return element;
}
