#ifndef BOMBERMAN_FONT_H
#define BOMBERMAN_FONT_H

#include <stdint.h>
#include <stdbool.h>

void font_draw_char(uint32_t *fb, int sw, int x, int y, uint32_t c, const uint8_t *palette);
void font_draw_string(uint32_t *fb, int sw, int x, int y, const char *str, const uint8_t *palette);
int font_string_width(const char *str);

#endif // BOMBERMAN_FONT_H
