#ifndef BOMBERMAN_GAME_H
#define BOMBERMAN_GAME_H

#include "types.h"
#include <stdint.h>

#define BTN_RIGHT  0x01
#define BTN_LEFT   0x02
#define BTN_DOWN   0x04
#define BTN_UP     0x08
#define BTN_START  0x10
#define BTN_SELECT 0x20
#define BTN_B      0x40 // Detonate (Remote)
#define BTN_A      0x80 // Drop Bomb

void game_init(GameState *game, int stage);
void game_setup_stage(GameState *game);
void game_next_stage(GameState *game);
void game_tick(GameState *game, uint8_t input_mask);

#endif // BOMBERMAN_GAME_H
