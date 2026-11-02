#include "game.h"
#include "audio.h"
#include "settings.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Level Enemy Distribution (From MONSTER_TAB in original 6502 code: all 50 stages)
static const uint8_t STAGE_ENEMIES[MAX_STAGES][MAX_ENEMIES] = {
    { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 }, // Stage 1: 6 Valcom
    { 1, 1, 1, 1, 2, 2, 0, 0, 0, 0 }, // Stage 2: 4 Valcom, 2 O'Neal
    { 1, 1, 2, 2, 2, 2, 0, 0, 0, 0 }, // Stage 3: 2 Valcom, 4 O'Neal
    { 2, 2, 2, 2, 2, 2, 0, 0, 0, 0 }, // Stage 4: 6 O'Neal
    { 2, 2, 2, 2, 3, 3, 0, 0, 0, 0 }, // Stage 5: 4 O'Neal, 2 Dahl
    { 2, 2, 3, 3, 3, 3, 0, 0, 0, 0 }, // Stage 6: 2 O'Neal, 4 Dahl
    { 3, 3, 3, 3, 3, 3, 0, 0, 0, 0 }, // Stage 7: 6 Dahl
    { 3, 3, 3, 3, 4, 4, 0, 0, 0, 0 }, // Stage 8: 4 Dahl, 2 Minvo
    { 3, 3, 4, 4, 4, 4, 0, 0, 0, 0 }, // Stage 9: 2 Dahl, 4 Minvo
    { 4, 4, 4, 4, 4, 4, 0, 0, 0, 0 }, // Stage 10: 6 Minvo
    { 4, 4, 4, 4, 4, 5, 0, 0, 0, 0 }, // Stage 11: 5 Minvo, 1 Doria
    { 4, 4, 4, 4, 5, 5, 0, 0, 0, 0 }, // Stage 12: 4 Minvo, 2 Doria
    { 4, 4, 4, 5, 5, 5, 0, 0, 0, 0 }, // Stage 13: 3 Minvo, 3 Doria
    { 4, 4, 5, 5, 5, 5, 0, 0, 0, 0 }, // Stage 14: 2 Minvo, 4 Doria
    { 4, 5, 5, 5, 5, 5, 0, 0, 0, 0 }, // Stage 15: 1 Minvo, 5 Doria
    { 5, 5, 5, 5, 5, 5, 0, 0, 0, 0 }, // Stage 16: 6 Doria
    { 5, 5, 5, 5, 5, 6, 0, 0, 0, 0 }, // Stage 17: 5 Doria, 1 Ovape
    { 5, 5, 5, 5, 6, 6, 0, 0, 0, 0 }, // Stage 18: 4 Doria, 2 Ovape
    { 5, 5, 5, 6, 6, 6, 0, 0, 0, 0 }, // Stage 19: 3 Doria, 3 Ovape
    { 5, 5, 6, 6, 6, 6, 0, 0, 0, 0 }, // Stage 20: 2 Doria, 4 Ovape
    { 5, 6, 6, 6, 6, 6, 7, 0, 0, 0 }, // Stage 21: 1 Doria, 5 Ovape, 1 Pass
    { 6, 6, 6, 6, 6, 6, 7, 7, 0, 0 }, // Stage 22: 6 Ovape, 2 Pass
    { 6, 6, 6, 6, 6, 7, 7, 7, 0, 0 }, // Stage 23: 5 Ovape, 3 Pass
    { 6, 6, 6, 6, 7, 7, 7, 7, 0, 0 }, // Stage 24: 4 Ovape, 4 Pass
    { 6, 6, 6, 7, 7, 7, 7, 7, 0, 0 }, // Stage 25: 3 Ovape, 5 Pass
    { 2, 2, 4, 4, 4, 4, 5, 5, 0, 0 }, // Stage 26: 2 O'Neal, 4 Minvo, 2 Doria
    { 2, 4, 4, 4, 5, 5, 5, 0, 0, 0 }, // Stage 27: 1 O'Neal, 3 Minvo, 3 Doria
    { 4, 4, 4, 4, 5, 5, 5, 6, 0, 0 }, // Stage 28: 4 Minvo, 3 Doria, 1 Ovape
    { 4, 4, 5, 5, 5, 5, 6, 6, 0, 0 }, // Stage 29: 2 Minvo, 4 Doria, 2 Ovape
    { 5, 5, 5, 5, 5, 5, 6, 6, 0, 0 }, // Stage 30: 6 Doria, 2 Ovape
    { 5, 5, 5, 5, 5, 6, 6, 6, 0, 0 }, // Stage 31: 5 Doria, 3 Ovape
    { 5, 5, 5, 5, 6, 6, 6, 6, 0, 0 }, // Stage 32: 4 Doria, 4 Ovape
    { 5, 5, 5, 6, 6, 6, 6, 6, 0, 0 }, // Stage 33: 3 Doria, 5 Ovape
    { 5, 5, 6, 6, 6, 6, 6, 6, 0, 0 }, // Stage 34: 2 Doria, 6 Ovape
    { 5, 6, 6, 6, 6, 6, 6, 7, 0, 0 }, // Stage 35: 1 Doria, 6 Ovape, 1 Pass
    { 6, 6, 6, 6, 6, 6, 7, 7, 0, 0 }, // Stage 36: 6 Ovape, 2 Pass
    { 6, 6, 6, 6, 6, 7, 7, 7, 0, 0 }, // Stage 37: 5 Ovape, 3 Pass
    { 6, 6, 6, 6, 7, 7, 7, 7, 0, 0 }, // Stage 38: 4 Ovape, 4 Pass
    { 6, 6, 6, 7, 7, 7, 7, 7, 0, 0 }, // Stage 39: 3 Ovape, 5 Pass
    { 6, 6, 7, 7, 7, 7, 7, 7, 0, 0 }, // Stage 40: 2 Ovape, 6 Pass
    { 6, 7, 7, 7, 7, 7, 7, 8, 0, 0 }, // Stage 41: 1 Ovape, 6 Pass, 1 Pontan
    { 7, 7, 7, 7, 7, 7, 8, 8, 0, 0 }, // Stage 42: 6 Pass, 2 Pontan
    { 7, 7, 7, 7, 7, 8, 8, 8, 0, 0 }, // Stage 43: 5 Pass, 3 Pontan
    { 7, 7, 7, 7, 8, 8, 8, 8, 0, 0 }, // Stage 44: 4 Pass, 4 Pontan
    { 7, 7, 7, 8, 8, 8, 8, 8, 0, 0 }, // Stage 45: 3 Pass, 5 Pontan
    { 7, 7, 8, 8, 8, 8, 8, 8, 0, 0 }, // Stage 46: 2 Pass, 6 Pontan
    { 7, 8, 8, 8, 8, 8, 8, 5, 0, 0 }, // Stage 47: 1 Pass, 6 Pontan, 1 Doria
    { 8, 8, 8, 8, 8, 8, 5, 5, 0, 0 }, // Stage 48: 6 Pontan, 2 Doria
    { 8, 8, 8, 8, 7, 7, 7, 7, 0, 0 }, // Stage 49: 4 Pontan, 4 Pass
    { 6, 6, 5, 7, 7, 7, 7, 7, 8, 8 }, // Stage 50: 2 Ovape, 1 Doria, 5 Pass, 2 Pontan
};

// 6502 Authentic PRNG (Exact RAND subroutine in BMAN.NAS)
static uint8_t nes_rand(GameState *game) {
    uint8_t a = game->seed[0];
    a = (a << 2) | (a >> 6);
    a ^= 0x41;
    a = (a << 2) | (a >> 6);
    a ^= 0x93;
    a += game->seed[1];
    game->seed[0] = a;

    a = (a << 2) | (a >> 6);
    a ^= 0x12;
    a = (a << 2) | (a >> 6);
    a += game->seed[2];
    game->seed[1] = a;

    a += game->seed[0];
    game->seed[2]++;
    if (game->seed[2] == 0) {
        game->seed[3] += 0x1D;
    }
    return a ^ game->seed[3];
}

static void rand_coords(GameState *game, int *out_x, int *out_y) {
    int attempts = 0;
    while (attempts < 500) {
        attempts++;
        uint8_t rx = nes_rand(game);
        int x = (rx >> 2) & 0x1F;
        if (x == 0 || x >= PLAYFIELD_COLS - 1) continue;

        uint8_t ry = nes_rand(game);
        int y = (ry >> 3) & 0x0F;
        if (y == 0 || y >= PLAYFIELD_ROWS - 1) continue;

        if (game->map[y][x] != MAP_EMPTY) continue;
        if (x < 3 && y < 3) continue; // Player spawn safe zone

        *out_x = x;
        *out_y = y;
        return;
    }

    // Deterministic fallback: collect all available empty cells outside safe zone
    int candidates_x[PLAYFIELD_COLS * PLAYFIELD_ROWS];
    int candidates_y[PLAYFIELD_COLS * PLAYFIELD_ROWS];
    int count = 0;

    for (int cy = 1; cy < PLAYFIELD_ROWS - 1; cy++) {
        for (int cx = 1; cx < PLAYFIELD_COLS - 1; cx++) {
            if (cx < 3 && cy < 3) continue;
            if (game->map[cy][cx] == MAP_EMPTY) {
                candidates_x[count] = cx;
                candidates_y[count] = cy;
                count++;
            }
        }
    }

    if (count > 0) {
        int idx = (int)(nes_rand(game) % count);
        *out_x = candidates_x[idx];
        *out_y = candidates_y[idx];
    } else {
        *out_x = 1;
        *out_y = 1;
    }
}


static bool is_solid_for_player(const GameState *game, int x, int y) {
    if (x < 0 || x >= PLAYFIELD_COLS || y < 0 || y >= PLAYFIELD_ROWS) return true;
    uint8_t t = game->map[y][x];
    if (t == MAP_CONCRETE) return true;
    if (t == MAP_BRICK || t == MAP_HIDDEN_EXIT || t == MAP_HIDDEN_BONUS) {
        return !game->bonus_noclip;
    }
    if (t == MAP_BOMB) {
        return !game->bonus_bombwalk;
    }
    return false;
}

static bool is_solid_for_enemy(const GameState *game, int type, int x, int y) {
    if (x < 0 || x >= PLAYFIELD_COLS || y < 0 || y >= PLAYFIELD_ROWS) return true;
    uint8_t t = game->map[y][x];
    if (t == MAP_CONCRETE) return true;
    if (t == MAP_BOMB) return true; // Enemies cannot cross bombs (trapped / stunned)
    if (t == MAP_BRICK || t == MAP_HIDDEN_EXIT || t == MAP_HIDDEN_BONUS) {
        if (type == ENEMY_DORIA || type == ENEMY_OVAPE || type == ENEMY_PASS || type == ENEMY_PONTAN) return false;
        return true;
    }
    return false;
}

// 6502 Subroutine: ADJUST_BOMBMAN_VPOS (Grid Corner Slide)
static void adjust_bombman_vpos(Player *p) {
    if (p->v < TILE_HALFSIZE) p->v++;
    else if (p->v > TILE_HALFSIZE) p->v--;
}

// 6502 Subroutine: ADJUST_BOMBMAN_HPOS (Grid Corner Slide)
static void adjust_bombman_hpos(Player *p) {
    if (p->u < TILE_HALFSIZE) p->u++;
    else if (p->u > TILE_HALFSIZE) p->u--;
}

static void move_player(GameState *game, int dir) {
    Player *p = &game->player;
    p->dir = dir;
    p->moving = true;

    switch (dir) {
        case DIR_RIGHT:
            if (p->u < TILE_HALFSIZE) {
                p->u++;
            } else {
                if (!is_solid_for_player(game, p->x + 1, p->y)) {
                    adjust_bombman_vpos(p);
                    p->u++;
                    if (p->u >= TILE_SIZE) {
                        p->u = 0;
                        p->x++;
                    }
                }
            }
            break;

        case DIR_LEFT:
            if (p->u > TILE_HALFSIZE) {
                p->u--;
            } else {
                if (!is_solid_for_player(game, p->x - 1, p->y)) {
                    adjust_bombman_vpos(p);
                    p->u--;
                    if (p->u < 0) {
                        p->u = TILE_SIZE - 1;
                        p->x--;
                    }
                }
            }
            break;

        case DIR_DOWN:
            if (p->v < TILE_HALFSIZE) {
                p->v++;
            } else {
                if (!is_solid_for_player(game, p->x, p->y + 1)) {
                    adjust_bombman_hpos(p);
                    p->v++;
                    if (p->v >= TILE_SIZE) {
                        p->v = 0;
                        p->y++;
                    }
                }
            }
            break;

        case DIR_UP:
            if (p->v > TILE_HALFSIZE) {
                p->v--;
            } else {
                if (!is_solid_for_player(game, p->x, p->y - 1)) {
                    adjust_bombman_hpos(p);
                    p->v--;
                    if (p->v < 0) {
                        p->v = TILE_SIZE - 1;
                        p->y--;
                    }
                }
            }
            break;
    }
}

static void explode_bomb(GameState *game, int b_idx);

static void drop_bomb(GameState *game) {
    Player *p = &game->player;
    int gx = p->x;
    int gy = p->y;

    if (game->map[gy][gx] != MAP_EMPTY) return;

    int active_bombs = 0;
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (game->bombs[i].active) active_bombs++;
    }
    if (active_bombs >= game->max_bombs) return;

    for (int i = 0; i < MAX_BOMBS; i++) {
        if (!game->bombs[i].active) {
            game->bombs[i].active = true;
            game->bombs[i].x = gx;
            game->bombs[i].y = gy;
            game->bombs[i].time_left = 160; // 160 frames fuse (~2.66s)
            game->bombs[i].time_elapsed = 0;
            game->bombs[i].power = game->bomb_power;
            game->map[gy][gx] = MAP_BOMB;

            adjust_bombman_hpos(p);
            adjust_bombman_vpos(p);

            audio_play_sound(SND_PLANT_BOMB);
            break;
        }
    }
}

static void spawn_fire(GameState *game, int x, int y, int dir, bool is_brick, bool is_cap) {
    for (int i = 0; i < MAX_FIRES; i++) {
        if (!game->fires[i].active) {
            game->fires[i].active = true;
            game->fires[i].is_brick = is_brick;
            game->fires[i].is_cap = is_cap;
            game->fires[i].x = x;
            game->fires[i].y = y;
            game->fires[i].dir = dir;
            game->fires[i].timer = is_brick ? 28 : 36;
            game->fires[i].anim_frame = 0;
            break;
        }
    }
}

static void explode_bomb(GameState *game, int b_idx) {
    if (!game->bombs[b_idx].active) return;
    game->bombs[b_idx].active = false;

    int bx = game->bombs[b_idx].x;
    int by = game->bombs[b_idx].y;
    int power = game->bombs[b_idx].power;

    game->map[by][bx] = MAP_EMPTY;
    spawn_fire(game, bx, by, DIR_NONE, false, false);
    audio_play_sound(SND_EXPLOSION);

    int dx[] = { 0, 1, 0, -1, 0 };
    int dy[] = { 0, 0, -1, 0, 1 };

    for (int d = 1; d <= 4; d++) {
        for (int step = 1; step <= power; step++) {
            int tx = bx + dx[d] * step;
            int ty = by + dy[d] * step;

            if (tx < 0 || tx >= PLAYFIELD_COLS || ty < 0 || ty >= PLAYFIELD_ROWS) break;

            uint8_t cell = game->map[ty][tx];
            if (cell == MAP_CONCRETE) {
                break;
            } else if (cell == MAP_BRICK || cell == MAP_HIDDEN_EXIT || cell == MAP_HIDDEN_BONUS) {
                spawn_fire(game, tx, ty, d, true, false);
                if (cell == MAP_HIDDEN_EXIT) {
                    game->map[ty][tx] = MAP_EXIT;
                } else if (cell == MAP_HIDDEN_BONUS) {
                    game->map[ty][tx] = MAP_BONUS;
                } else {
                    game->map[ty][tx] = MAP_EMPTY;
                }
                break;
            } else if (cell == MAP_BOMB) {
                for (int ob = 0; ob < MAX_BOMBS; ob++) {
                    if (game->bombs[ob].active && game->bombs[ob].x == tx && game->bombs[ob].y == ty) {
                        explode_bomb(game, ob);
                        break;
                    }
                }
                break;
            } else {
                bool is_cap = (step == power);
                spawn_fire(game, tx, ty, d, false, is_cap);
            }
        }
    }
}

static void update_bombs(GameState *game) {
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (!game->bombs[i].active) continue;
        game->bombs[i].time_elapsed++;
        if (!game->bonus_remote) {
            game->bombs[i].time_left--;
            if (game->bombs[i].time_left <= 0) {
                explode_bomb(game, i);
            }
        }
    }
}

static void update_fires(GameState *game) {
    for (int i = 0; i < MAX_FIRES; i++) {
        if (!game->fires[i].active) continue;
        game->fires[i].timer--;
        if (game->fires[i].is_brick) {
            game->fires[i].anim_frame = 6 - (game->fires[i].timer / 4);
            if (game->fires[i].anim_frame < 0) game->fires[i].anim_frame = 0;
            if (game->fires[i].anim_frame > 6) game->fires[i].anim_frame = 6;
        }
        if (game->fires[i].timer <= 0) {
            game->fires[i].active = false;
        }
    }
}

static void trigger_player_death(GameState *game) {
    if (game->player.dying || game->invulnerable_timer > 0 || g_settings.invulnerable) return;
    game->player.dying = true;
    game->player.death_frame = 12; // Start of shocked/dying animation
    game->player.death_timer = 16; // 16 frames per stage (from NES ROM line 2042)
    audio_play_sound(SND_DEATH);
}

static void update_enemies(GameState *game) {
    Player *p = &game->player;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &game->enemies[i];
        if (e->type == ENEMY_NONE) continue;

        // 1. Defeated Score Popup State
        if (e->state == ENEMY_STATE_SCORE) {
            e->score_timer--;
            if (e->score_timer <= 0) {
                e->state = ENEMY_STATE_DEAD;
                e->type = ENEMY_NONE;
            }
            continue;
        }

        // 2. Burning / Melting Stun State (from NES ROM lines 1214..1230 & 3050..3070)
        if (e->state == ENEMY_STATE_STUN) {
            e->stun_timer--;
            // Cycle through melting frames (32..43)
            if (e->stun_timer % 4 == 0) {
                e->frame++;
            }
            if (e->stun_timer <= 0) {
                // Transition to Score Popup
                e->state = ENEMY_STATE_SCORE;
                e->score_timer = 30; // 30 frames display
                game->score += e->score_val;
            }
            continue; // Cannot move or harm player while stunned
        }

        // 3. Normal Enemy AI Movement & Throttling
        bool can_step = false;
        if (e->type == ENEMY_VALCOM) {
            can_step = (game->frame_cnt & 1) == 0; // 30 px/s
        } else if (e->type == ENEMY_ONEAL || e->type == ENEMY_DAHL) {
            can_step = (game->frame_cnt & 3) != 0; // 45 px/s
        } else if (e->type == ENEMY_MINVO || e->type == ENEMY_PASS || e->type == ENEMY_PONTAN) {
            can_step = true; // 60 px/s
        } else if (e->type == ENEMY_DORIA) {
            can_step = (game->frame_cnt & 3) == 0; // 15 px/s
        } else if (e->type == ENEMY_OVAPE) {
            can_step = (game->frame_cnt & 1) == 0; // 30 px/s
        } else {
            can_step = (game->frame_cnt & 1) == 0;
        }

        if (!can_step) continue;

        e->anim_timer++;
        if (e->anim_timer % 8 == 0) {
            e->frame = (e->frame + 1) % 4;
        }

        // AI Decision Point
        if (e->u == TILE_HALFSIZE && e->v == TILE_HALFSIZE) {
            if ((e->type == ENEMY_ONEAL || e->type == ENEMY_PASS || e->type == ENEMY_PONTAN) && e->x == p->x) {
                e->dir = (p->y > e->y) ? DIR_DOWN : DIR_UP;
            } else if ((e->type == ENEMY_DAHL || e->type == ENEMY_PASS || e->type == ENEMY_PONTAN) && e->y == p->y) {
                e->dir = (p->x > e->x) ? DIR_RIGHT : DIR_LEFT;
            } else if (e->step_counter <= 0) {
                uint8_t r = nes_rand(game);
                e->step_counter = ((r & 0x18) << 1) + 16;
                e->dir = (r & 3) + 1;
            }
        }

        e->step_counter--;

        bool blocked = false;
        if (e->dir == DIR_RIGHT) {
            if (e->u < TILE_HALFSIZE) e->u++;
            else {
                if (is_solid_for_enemy(game, e->type, e->x + 1, e->y)) blocked = true;
                else {
                    e->u++;
                    if (e->u >= TILE_SIZE) { e->u = 0; e->x++; }
                }
            }
        } else if (e->dir == DIR_LEFT) {
            if (e->u > TILE_HALFSIZE) e->u--;
            else {
                if (is_solid_for_enemy(game, e->type, e->x - 1, e->y)) blocked = true;
                else {
                    e->u--;
                    if (e->u < 0) { e->u = TILE_SIZE - 1; e->x--; }
                }
            }
        } else if (e->dir == DIR_DOWN) {
            if (e->v < TILE_HALFSIZE) e->v++;
            else {
                if (is_solid_for_enemy(game, e->type, e->x, e->y + 1)) blocked = true;
                else {
                    e->v++;
                    if (e->v >= TILE_SIZE) { e->v = 0; e->y++; }
                }
            }
        } else if (e->dir == DIR_UP) {
            if (e->v > TILE_HALFSIZE) e->v--;
            else {
                if (is_solid_for_enemy(game, e->type, e->x, e->y - 1)) blocked = true;
                else {
                    e->v--;
                    if (e->v < 0) { e->v = TILE_SIZE - 1; e->y--; }
                }
            }
        }

        if (blocked) {
            e->step_counter = 0;
            if (e->dir == DIR_RIGHT) e->dir = DIR_LEFT;
            else if (e->dir == DIR_LEFT) e->dir = DIR_RIGHT;
            else if (e->dir == DIR_DOWN) e->dir = DIR_UP;
            else if (e->dir == DIR_UP) e->dir = DIR_DOWN;
        }

        // Enemy-Player Collision (NES ROM lines 2822..2834)
        if (!p->dying && game->invulnerable_timer == 0) {
            if (e->x == p->x && e->y == p->y) {
                trigger_player_death(game);
            }
        }
    }
}

static void check_explosions_damage(GameState *game) {
    Player *p = &game->player;

    for (int i = 0; i < MAX_FIRES; i++) {
        if (!game->fires[i].active || game->fires[i].is_brick) continue;
        int fx = game->fires[i].x;
        int fy = game->fires[i].y;

        // Player fire damage
        if (!p->dying && !game->bonus_firesuit && game->invulnerable_timer == 0) {
            if (p->x == fx && p->y == fy) {
                trigger_player_death(game);
            }
        }

        // Enemy fire damage: triggers stun and melting
        for (int e = 0; e < MAX_ENEMIES; e++) {
            Enemy *en = &game->enemies[e];
            if (en->type != ENEMY_NONE && en->state == ENEMY_STATE_NORMAL) {
                if (en->x == fx && en->y == fy) {
                    en->state = ENEMY_STATE_STUN;
                    en->stun_timer = 32; // ~0.53 seconds melting stun
                    en->frame = 0;
                    en->score_val = (en->type == ENEMY_VALCOM) ? 100 :
                                    (en->type == ENEMY_ONEAL)  ? 200 :
                                    (en->type == ENEMY_DAHL)   ? 400 :
                                    (en->type == ENEMY_MINVO)  ? 800 :
                                    (en->type == ENEMY_DORIA)  ? 1000 :
                                    (en->type == ENEMY_OVAPE)  ? 2000 :
                                    (en->type == ENEMY_PASS)   ? 4000 : 8000;
                    audio_play_sound(SND_POWERUP);
                }
            }
        }
    }
}

static void update_camera_scroll(GameState *game) {
    int sw = game->screen_w > 0 ? game->screen_w : DEFAULT_SCREEN_W;
    int max_scroll = (PLAYFIELD_COLS * TILE_SIZE) - sw;
    if (max_scroll < 0) max_scroll = 0;

    int px = game->player.x * TILE_SIZE + game->player.u;
    int target_scroll = px - (sw / 2);

    if (target_scroll < 0) target_scroll = 0;
    if (target_scroll > max_scroll) target_scroll = max_scroll;
    game->h_scroll = target_scroll;
}

void game_setup_stage(GameState *game) {
    // 1. Rebuild standard NES grid map
    for (int r = 0; r < PLAYFIELD_ROWS; r++) {
        for (int c = 0; c < PLAYFIELD_COLS; c++) {
            if (r == 0 || r == PLAYFIELD_ROWS - 1 || c == 0 || c == PLAYFIELD_COLS - 1) {
                game->map[r][c] = MAP_CONCRETE;
            } else if ((r % 2 == 0) && (c % 2 == 0)) {
                game->map[r][c] = MAP_CONCRETE;
            } else {
                game->map[r][c] = MAP_EMPTY;
            }
        }
    }

    // 2. Procedural hidden exit door and hidden bonus powerup
    int exit_x, exit_y, bonus_x, bonus_y;
    rand_coords(game, &exit_x, &exit_y);
    game->map[exit_y][exit_x] = MAP_HIDDEN_EXIT;

    rand_coords(game, &bonus_x, &bonus_y);
    game->map[bonus_y][bonus_x] = MAP_HIDDEN_BONUS;

    // 3. Procedural brick generation (50 + 2 * stage, capped at 120)
    int brick_count = 50 + 2 * game->stage;
    if (brick_count > 120) brick_count = 120;
    for (int i = 0; i < brick_count; i++) {
        int bx, by;
        rand_coords(game, &bx, &by);
        game->map[by][bx] = MAP_BRICK;
    }

    // 4. Reset bombs and fires
    memset(game->bombs, 0, sizeof(game->bombs));
    memset(game->fires, 0, sizeof(game->fires));

    // 5. Reset player position and movement state
    game->player.x = 1;
    game->player.u = TILE_HALFSIZE;
    game->player.y = 1;
    game->player.v = TILE_HALFSIZE;
    game->player.dir = DIR_DOWN;
    game->player.death_frame = 12;
    game->player.dying = false;
    game->player.death_timer = 0;
    game->player.moving = false;
    game->player.frame = 0;

    // 6. Spawn authentic enemy roster for this stage
    memset(game->enemies, 0, sizeof(game->enemies));
    int st_idx = (game->stage - 1) % MAX_STAGES;
    if (st_idx < 0) st_idx = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        uint8_t etype = STAGE_ENEMIES[st_idx][i];
        if (etype == 0) continue;

        int ex = 5, ey = 1;
        int spawn_attempts = 0;
        while (spawn_attempts < 200) {
            spawn_attempts++;
            rand_coords(game, &ex, &ey);
            if (ex >= 5) break;
        }
        if (ex < 5) {
            for (int cy = 1; cy < PLAYFIELD_ROWS - 1; cy++) {
                for (int cx = 3; cx < PLAYFIELD_COLS - 1; cx++) {
                    if (game->map[cy][cx] == MAP_EMPTY) {
                        ex = cx;
                        ey = cy;
                        break;
                    }
                }
                if (ex >= 3) break;
            }
        }


        game->enemies[i].type = etype;
        game->enemies[i].state = ENEMY_STATE_NORMAL;
        game->enemies[i].x = ex;
        game->enemies[i].u = TILE_HALFSIZE;
        game->enemies[i].y = ey;
        game->enemies[i].v = TILE_HALFSIZE;
        game->enemies[i].dir = (nes_rand(game) & 3) + 1;
        game->enemies[i].step_counter = 32;
        game->enemies[i].frame = 0;
        game->enemies[i].anim_timer = 0;
    }

    // 7. Reset level timers and flags
    game->time_left = 200;
    game->second_timer = 0;
    game->invulnerable_timer = g_settings.invulnerable ? 9999999 : 120; // 2 seconds invulnerability on spawn, or godmode
    if (g_settings.speed_fast) {
        game->bonus_speed = true;
    }
    game->stage_clear = false;
    game->stage_clear_timer = 0;
    game->game_over = false;

    update_camera_scroll(game);
}

void game_init(GameState *game, int stage) {
    int cur_sw = game->screen_w > 0 ? game->screen_w : DEFAULT_SCREEN_W;
    memset(game, 0, sizeof(GameState));
    game->screen_w = cur_sw;

    // Entropy seeding: On real NES, the random seed comes from power-on uninitialized RAM
    // plus the number of VBlank frames the title screen waits for the player to press START.
    // Cross-platform high-resolution entropy:
    uint64_t perf = SDL_GetPerformanceCounter();
    time_t now = time(NULL);
    uint32_t entropy = (uint32_t)(perf ^ ((uint64_t)now << 16) ^ ((uintptr_t)game >> 4));
    game->seed[0] = (uint8_t)(entropy & 0xFF);
    game->seed[1] = (uint8_t)((entropy >> 8) & 0xFF);
    game->seed[2] = (uint8_t)((entropy >> 16) & 0xFF);
    game->seed[3] = (uint8_t)((entropy >> 24) & 0xFF);

    // Ensure non-zero valid seed
    if (game->seed[0] == 0 && game->seed[1] == 0) game->seed[0] = 0x55;
    if (game->seed[2] == 0 && game->seed[3] == 0) game->seed[2] = 0x1D;

    // Cycle PRNG non-deterministic times matching NES title screen frames
    int title_ticks = (int)(perf % 256) + 13;
    for (int i = 0; i < title_ticks; i++) {
        nes_rand(game);
    }

    game->stage = stage;
    game->lives = 3;
    game->score = 0;
    game->bomb_power = 1;
    game->max_bombs = 1;

    game_setup_stage(game);
}

void game_next_stage(GameState *game) {
    // Authentic NES Next Stage:
    // Retain score, powerups, continue the PRNG sequence, and award +1 Life!
    game->stage++;
    if (game->lives < 9) {
        game->lives++;
    }
    game_setup_stage(game);
}

void game_tick(GameState *game, uint8_t input_mask) {
    game->frame_cnt++;

    // Game Over: freeze all simulation, wait for player to press R or use menu
    if (game->game_over) {
        return;
    }

    if (game->stage_clear) {
        game->stage_clear_timer++;
        if (game->stage_clear_timer >= 120) {
            game_next_stage(game);
        }
        return;
    }

    // Authentic Player Death Stun & Melting (NES ROM lines 2039..2050)
    if (game->player.dying) {
        game->player.death_timer--;
        if (game->player.death_timer <= 0) {
            game->player.death_timer = 16; // 16 frames per melt stage
            game->player.death_frame++;
            if (game->player.death_frame > 19) {
                game->lives--;
                if (game->lives <= 0) {
                    game->game_over = true;
                } else {
                    // Authentic NES stage restart on death: reload stage with full enemy roster
                    game->bomb_power = 1;
                    game->max_bombs = 1;
                    game->bonus_remote = false;
                    game->bonus_bombwalk = false;
                    game->bonus_noclip = false;
                    game->bonus_firesuit = false;
                    game->bonus_speed = false;
                    game_setup_stage(game);
                }
            }
        }
        // While dying, still update bombs and fires so the world remains alive
        update_bombs(game);
        update_fires(game);
        update_enemies(game);
        return;
    }

    // Timer countdown: only decrement when alive and time > 0
    game->second_timer++;
    if (game->second_timer >= 60) {
        game->second_timer = 0;
        if (game->time_left > 0) {
            game->time_left--;
            if (game->time_left == 0) {
                // Time's up! Kill the player once (NES authentic behavior)
                trigger_player_death(game);
            }
        }
    }

    if (game->invulnerable_timer > 0) {
        game->invulnerable_timer--;
    }

    game->player.moving = false;
    bool can_move = game->bonus_speed || ((game->frame_cnt & 3) != 0);

    if (can_move) {
        if (input_mask & BTN_RIGHT) move_player(game, DIR_RIGHT);
        else if (input_mask & BTN_LEFT) move_player(game, DIR_LEFT);
        else if (input_mask & BTN_DOWN) move_player(game, DIR_DOWN);
        else if (input_mask & BTN_UP) move_player(game, DIR_UP);
    }

    if (game->player.moving) {
        if ((game->frame_cnt & 3) == 2) {
            game->player.frame++;
            if ((game->player.frame % 4) == 0) {
                audio_play_sound(SND_STEP);
            }
        }
    }

    if (input_mask & BTN_A) {
        drop_bomb(game);
    }

    if ((input_mask & BTN_B) && game->bonus_remote) {
        for (int i = 0; i < MAX_BOMBS; i++) {
            if (game->bombs[i].active) {
                explode_bomb(game, i);
            }
        }
    }

    uint8_t cur_tile = game->map[game->player.y][game->player.x];
    if (cur_tile == MAP_BONUS) {
        game->map[game->player.y][game->player.x] = MAP_EMPTY;
        game->bomb_power++;
        game->max_bombs++;
        game->score += 1000;
        audio_play_sound(SND_POWERUP);
    } else if (cur_tile == MAP_EXIT) {
        bool enemies_left = false;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (game->enemies[i].type != ENEMY_NONE) enemies_left = true;
        }
        if (!enemies_left) {
            game->stage_clear = true;
            game->stage_clear_timer = 0;
            audio_play_sound(SND_STAGE_CLEAR);
        }
    }

    update_bombs(game);
    update_fires(game);
    update_enemies(game);

    if ((game->frame_cnt & 3) == 0) {
        check_explosions_damage(game);
    }

    update_camera_scroll(game);
}
