#ifndef DIALOGUE_PLAYER_H
#define DIALOGUE_PLAYER_H

#include <stdint.h>
#include <stdbool.h>

void dialogue_start(const char* name, uint16_t name_length);
void dialogue_start_index(uint32_t conversation_index);

void dialogue_end(void);
bool dialogue_is_active(void);

void dialogue_ui_draw(int32_t x, int32_t y, int32_t width, int32_t height, const uint32_t* portrait);

#endif // !DIALOGUE_PLAYER_H