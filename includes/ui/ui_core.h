#ifndef UI_CORE_H
#define UI_CORE_H

#include <stdbool.h>

void ui_begin_frame(int mouse_x, int mouse_y, bool mouse_down, bool nav_activate);
void ui_end_frame(void);
int  ui_gen_id(void);

#endif // !UI_CORE_H