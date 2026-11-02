#ifndef BOMBERMAN_GRAPHICS_H
#define BOMBERMAN_GRAPHICS_H

#include "types.h"
#include <stdint.h>

void graphics_init(void);
void graphics_set_screen_width(int sw);
void graphics_render_frame(const GameState *game, uint32_t *fb);
void graphics_draw_intro(uint32_t *fb, int sw, int frame_cnt);
void graphics_draw_main_menu(uint32_t *fb, int sw, int frame_cnt, int selected_item);

#endif // BOMBERMAN_GRAPHICS_H
