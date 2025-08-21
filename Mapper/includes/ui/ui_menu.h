#ifndef UI_MENU_H
#define UI_MENU_H

#include <stdint.h>
#include <stdbool.h>

typedef bool (*MenuAction)(void);

typedef struct
{
	const char* label;
	const char* shortcut;
	MenuAction action;
} UIMenuItem;

typedef struct
{
	const char* label;
	UIMenuItem* items;
	int item_count;
	bool open;
} UIMenu;

void ui_menu_bar(UIMenu* menus, int menu_count);

#endif // !UI_MENU_H
