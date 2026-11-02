#include "graphics.h"
#include "font.h"
#include "chr_rom.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHR_ROM _tmp_bomberman_disasm_BOMBER_CHR

// Canonical NES 64-color Master Palette (ARGB8888)
static const uint32_t NES_PALETTE[64] = {
    0xFF7C7C7C, 0xFF0000FC, 0xFF0000BC, 0xFF4428BC, 0xFF940084, 0xFFA80020, 0xFFA81000, 0xFF881400,
    0xFF503000, 0xFF007800, 0xFF006800, 0xFF005800, 0xFF004058, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFBCBCBC, 0xFF0078F8, 0xFF0058F8, 0xFF6844FC, 0xFFD800CC, 0xFFE40058, 0xFFF83800, 0xFFE45C10,
    0xFFAC7C00, 0xFF00B800, 0xFF00A800, 0xFF00A844, 0xFF008888, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFF8F8F8, 0xFF3CBCFC, 0xFF6888FC, 0xFF9878F8, 0xFFF878F8, 0xFFF85898, 0xFFF87858, 0xFFFCA044,
    0xFFF8B800, 0xFFB8F818, 0xFF58D854, 0xFF58F898, 0xFF00E8D8, 0xFF787878, 0xFF000000, 0xFF000000,
    0xFFFCFCFC, 0xFFA4E4FC, 0xFFB8B8F8, 0xFFD8B8F8, 0xFFF8B8F8, 0xFFF8A4C0, 0xFFF0D0B0, 0xFFFCE0A8,
    0xFFF8D878, 0xFFD8F878, 0xFFB8F8B8, 0xFFB8F8D8, 0xFF00FCFC, 0xFFF8D8F8, 0xFF000000, 0xFF000000
};

// Exact NES Palette Definitions (From STARTPAL & STAGEPAL in BMAN.NAS)
static const uint8_t PAL_HUD[4]      = { 0x0F, 0x10, 0x0F, 0x30 }; // Black, Grey shadow, Black, White text
static const uint8_t PAL_CONCRETE[4] = { 0x19, 0x0F, 0x10, 0x30 }; // Green, Black, Grey, White
static const uint8_t PAL_BRICK[4]    = { 0x19, 0x16, 0x26, 0x36 }; // Green, Dark Red, Coral Brick, Light Highlight
static const uint8_t PAL_FIRE[4]     = { 0x19, 0x0F, 0x16, 0x28 }; // Green, Black, Red, Yellow
static const uint8_t PAL_BOMBER[4]   = { 0x19, 0x30, 0x21, 0x26 }; // Transp, White helmet, Blue body, Pink face
static const uint8_t PAL_VALCOM[4]   = { 0x19, 0x0F, 0x26, 0x30 }; // Transp, Black outline, Coral balloon, White eyes
static const uint8_t PAL_ONEAL[4]    = { 0x19, 0x0F, 0x21, 0x30 }; // Transp, Black outline, Blue body, White eyes
static const uint8_t PAL_DAHL[4]     = { 0x19, 0x0F, 0x15, 0x30 }; // Transp, Black outline, Red/Purple barrel, White bands

// Exact 16x16 Metatile Definitions from TILE_MAP in BMAN.NAS (4 8x8 tiles each)
static const uint8_t TILE_FLOOR[4]       = { 0x5F, 0x5F, 0x5F, 0x5F };
static const uint8_t TILE_CONCRETE[4]    = { 0x64, 0x65, 0x66, 0x67 };
static const uint8_t TILE_BRICK[4]       = { 0x68, 0x69, 0x6A, 0x6B };

// 7 Stages of Crumbling Bricks (from TILE_MAP)
static const uint8_t TILE_CRUMBLE[7][4] = {
    { 0x6C, 0x6D, 0x6E, 0x6F },
    { 0x70, 0x71, 0x72, 0x73 },
    { 0x74, 0x75, 0x76, 0x77 },
    { 0x78, 0x79, 0x7A, 0x7B },
    { 0x7C, 0x7D, 0x7E, 0x7F },
    { 0x5F, 0x5F, 0x5F, 0x5F },
    { 0x5F, 0x5F, 0x5F, 0x5F }
};

// 3 Bomb Pulsing Stages (from TILE_MAP lines 4157..4159)
static const uint8_t TILE_BOMB[3][4] = {
    { 0x80, 0x81, 0x82, 0x83 }, // Bomb 1
    { 0x84, 0x85, 0x86, 0x87 }, // Bomb 2
    { 0x88, 0x89, 0x8A, 0x8B }  // Bomb 3
};

// Fire Flames (Center, Rays, End Caps)
static const uint8_t TILE_FIRE_CENTER[4] = { 0x8C, 0x8D, 0x8E, 0x8F };
static const uint8_t TILE_FIRE_HORIZ[4]  = { 0x20, 0x20, 0x21, 0x21 };
static const uint8_t TILE_FIRE_VERT[4]   = { 0x22, 0x23, 0x22, 0x23 };
static const uint8_t TILE_FIRE_RCAP[4]   = { 0x20, 0x9D, 0x21, 0x9F };
static const uint8_t TILE_FIRE_LCAP[4]   = { 0x9C, 0x20, 0x9E, 0x21 };
static const uint8_t TILE_FIRE_UCAP[4]   = { 0x9C, 0x9D, 0x22, 0x23 };
static const uint8_t TILE_FIRE_DCAP[4]   = { 0x22, 0x23, 0x9E, 0x9F };

// Exit Door & Powerup Items (from TILE_MAP lines 4189..4195)
static const uint8_t TILE_EXIT_DOOR[4]   = { 0x3C, 0x3D, 0x3E, 0x3F };
static const uint8_t TILE_POWERUP[4]     = { 0x02, 0x03, 0x12, 0x13 }; // Flame/Bomb item

static int s_screen_w = DEFAULT_SCREEN_W;

void graphics_set_screen_width(int sw) {
    if (sw >= 256 && sw <= MAX_SCREEN_W) {
        s_screen_w = sw;
    }
}

// Draw an 8x8 Tile from CHR-ROM (Pattern Table 0 or 1)
static void draw_chr_8x8(uint32_t *fb, int sx, int sy, int tile_idx, const uint8_t *palette, bool transparent_zero, bool flip_x, bool flip_y) {
    if (tile_idx < 0 || tile_idx >= 512) return;
    const uint8_t *tile_data = &CHR_ROM[tile_idx * 16];

    for (int r = 0; r < 8; r++) {
        int py = sy + (flip_y ? (7 - r) : r);
        if (py < 0 || py >= NES_SCREEN_H) continue;

        uint8_t plane0 = tile_data[r];
        uint8_t plane1 = tile_data[r + 8];

        for (int c = 0; c < 8; c++) {
            int px = sx + (flip_x ? (7 - c) : c);
            if (px < 0 || px >= s_screen_w) continue;

            uint8_t ci = (((plane1 >> (7 - c)) & 1) << 1) | ((plane0 >> (7 - c)) & 1);
            if (transparent_zero && ci == 0) continue;

            fb[py * s_screen_w + px] = NES_PALETTE[palette[ci]];
        }
    }
}

// Draw a 16x16 Metatile using 4 8x8 CHR Tiles from Pattern Table 1 (offset 256..511)
static void draw_metatile_16x16(uint32_t *fb, int sx, int sy, const uint8_t tiles[4], const uint8_t *palette) {
    draw_chr_8x8(fb, sx,     sy,     256 + tiles[0], palette, false, false, false);
    draw_chr_8x8(fb, sx + 8, sy,     256 + tiles[1], palette, false, false, false);
    draw_chr_8x8(fb, sx,     sy + 8, 256 + tiles[2], palette, false, false, false);
    draw_chr_8x8(fb, sx + 8, sy + 8, 256 + tiles[3], palette, false, false, false);
}

// Draw a 16x16 Sprite from Pattern Table 0 (offset 0..255)
static void draw_sprite_16x16(uint32_t *fb, int sx, int sy, int sprite_index, const uint8_t *palette, bool flip_x, bool flip_y) {
    int col = (sprite_index & 7) * 2;
    int row = (sprite_index >> 3) * 32;
    int spr_id = row + col;

    int tl = spr_id;
    int tr = spr_id + 1;
    int bl = spr_id + 16;
    int br = spr_id + 17;

    if (!flip_x) {
        draw_chr_8x8(fb, sx,     sy,     tl, palette, true, false, flip_y);
        draw_chr_8x8(fb, sx + 8, sy,     tr, palette, true, false, flip_y);
        draw_chr_8x8(fb, sx,     sy + 8, bl, palette, true, false, flip_y);
        draw_chr_8x8(fb, sx + 8, sy + 8, br, palette, true, false, flip_y);
    } else {
        draw_chr_8x8(fb, sx,     sy,     tr, palette, true, true, flip_y);
        draw_chr_8x8(fb, sx + 8, sy,     tl, palette, true, true, flip_y);
        draw_chr_8x8(fb, sx,     sy + 8, br, palette, true, true, flip_y);
        draw_chr_8x8(fb, sx + 8, sy + 8, bl, palette, true, true, flip_y);
    }
}

// Score popup e.g. "100", "200", "400" floating text above defeated enemy
static void draw_score_popup(uint32_t *fb, int sx, int sy, int score) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", score);
    int len = (int)strlen(buf);
    int start_x = sx + 8 - (len * 4);
    for (int i = 0; i < len; i++) {
        font_draw_char(fb, s_screen_w, start_x + i * 8, sy - 6, (uint32_t)buf[i], PAL_HUD);
    }
}

void graphics_init(void) {
    // CHR-ROM initialized
}

// Render complete frame: HUD + Playfield World
void graphics_render_frame(const GameState *game, uint32_t *fb) {
    int sw = (game->screen_w > 0) ? game->screen_w : s_screen_w;
    s_screen_w = sw;

    // 1. Draw Static Top HUD (Scanlines 0..23 px)
    for (int i = 0; i < sw * HUD_HEIGHT; i++) {
        fb[i] = NES_PALETTE[0x0F]; // Solid black
    }

    char hud_buf[32];
    if (sw >= 380) {
        snprintf(hud_buf, sizeof(hud_buf), "SÜRE %d", game->time_left);
        font_draw_string(fb, sw, 16, 8, hud_buf, PAL_HUD);

        snprintf(hud_buf, sizeof(hud_buf), "PUAN %07d", game->score);
        font_draw_string(fb, sw, (sw / 2) - 100, 8, hud_buf, PAL_HUD);

        snprintf(hud_buf, sizeof(hud_buf), "SEVİYE %d", game->stage);
        font_draw_string(fb, sw, (sw / 2) + 20, 8, hud_buf, PAL_HUD);

        snprintf(hud_buf, sizeof(hud_buf), "CAN %d", game->lives);
        font_draw_string(fb, sw, sw - 64, 8, hud_buf, PAL_HUD);
    } else {
        snprintf(hud_buf, sizeof(hud_buf), "SÜRE %d", game->time_left);
        font_draw_string(fb, sw, 8, 8, hud_buf, PAL_HUD);

        snprintf(hud_buf, sizeof(hud_buf), "%07d", game->score);
        font_draw_string(fb, sw, (sw / 2) - 48, 8, hud_buf, PAL_HUD);

        snprintf(hud_buf, sizeof(hud_buf), "SEV %d", game->stage);
        font_draw_string(fb, sw, (sw / 2) + 20, 8, hud_buf, PAL_HUD);

        snprintf(hud_buf, sizeof(hud_buf), "CAN %d", game->lives);
        font_draw_string(fb, sw, sw - 56, 8, hud_buf, PAL_HUD);
    }

    int h_scroll = game->h_scroll;

    // 2. Draw 31x13 Map Grid using authentic CHR-ROM metatiles
    for (int row = 0; row < PLAYFIELD_ROWS; row++) {
        int sy = HUD_HEIGHT + row * TILE_SIZE;
        for (int col = 0; col < PLAYFIELD_COLS; col++) {
            int sx = col * TILE_SIZE - h_scroll;
            if (sx + TILE_SIZE <= 0 || sx >= sw) continue;

            uint8_t cell = game->map[row][col];
            switch (cell) {
                case MAP_CONCRETE:
                    draw_metatile_16x16(fb, sx, sy, TILE_CONCRETE, PAL_CONCRETE);
                    break;
                case MAP_BRICK:
                case MAP_HIDDEN_EXIT:
                case MAP_HIDDEN_BONUS:
                    draw_metatile_16x16(fb, sx, sy, TILE_BRICK, PAL_BRICK);
                    break;
                case MAP_EXIT:
                    draw_metatile_16x16(fb, sx, sy, TILE_EXIT_DOOR, PAL_CONCRETE);
                    break;
                case MAP_BONUS:
                    draw_metatile_16x16(fb, sx, sy, TILE_POWERUP, PAL_FIRE);
                    break;
                default:
                    // Green grass floor
                    draw_metatile_16x16(fb, sx, sy, TILE_FLOOR, PAL_CONCRETE);
                    break;
            }
        }
    }

    // 3. Draw Bombs (pulsing sizes)
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (!game->bombs[i].active) continue;
        int sx = game->bombs[i].x * TILE_SIZE - h_scroll;
        int sy = HUD_HEIGHT + game->bombs[i].y * TILE_SIZE;
        if (sx + TILE_SIZE > 0 && sx < sw) {
            int step = (game->bombs[i].time_elapsed / 16) % 4;
            int b_idx = (step == 0) ? 1 : (step == 1) ? 2 : (step == 2) ? 1 : 0;
            draw_metatile_16x16(fb, sx, sy, TILE_BOMB[b_idx], PAL_FIRE);
        }
    }

    // 4. Draw Fires and Crumbling Bricks
    for (int i = 0; i < MAX_FIRES; i++) {
        const Fire *f = &game->fires[i];
        if (!f->active) continue;
        int sx = f->x * TILE_SIZE - h_scroll;
        int sy = HUD_HEIGHT + f->y * TILE_SIZE;
        if (sx + TILE_SIZE <= 0 || sx >= sw) continue;

        if (f->is_brick) {
            // Authentic 7-stage crumbling brickwork from TILE_MAP
            int stage = f->anim_frame;
            if (stage < 0) stage = 0;
            if (stage > 6) stage = 6;
            draw_metatile_16x16(fb, sx, sy, TILE_CRUMBLE[stage], PAL_BRICK);
        } else {
            // Directional explosion plumes
            if (f->dir == DIR_NONE) {
                draw_metatile_16x16(fb, sx, sy, TILE_FIRE_CENTER, PAL_FIRE);
            } else if (f->dir == DIR_RIGHT) {
                draw_metatile_16x16(fb, sx, sy, f->is_cap ? TILE_FIRE_RCAP : TILE_FIRE_HORIZ, PAL_FIRE);
            } else if (f->dir == DIR_LEFT) {
                draw_metatile_16x16(fb, sx, sy, f->is_cap ? TILE_FIRE_LCAP : TILE_FIRE_HORIZ, PAL_FIRE);
            } else if (f->dir == DIR_UP) {
                draw_metatile_16x16(fb, sx, sy, f->is_cap ? TILE_FIRE_UCAP : TILE_FIRE_VERT, PAL_FIRE);
            } else {
                draw_metatile_16x16(fb, sx, sy, f->is_cap ? TILE_FIRE_DCAP : TILE_FIRE_VERT, PAL_FIRE);
            }
        }
    }

    // 5. Draw Enemies (Valcom, O'Neal, Dahl with walking, stun and melting states)
    for (int i = 0; i < MAX_ENEMIES; i++) {
        const Enemy *e = &game->enemies[i];
        if (e->type == ENEMY_NONE) continue;

        int sx = e->x * TILE_SIZE + e->u - TILE_HALFSIZE - h_scroll;
        int sy = HUD_HEIGHT + e->y * TILE_SIZE + e->v - TILE_HALFSIZE;

        if (sx + TILE_SIZE <= 0 || sx >= sw) continue;

        if (e->state == ENEMY_STATE_SCORE) {
            draw_score_popup(fb, sx, sy, e->score_val);
            continue;
        }

        if (e->state == ENEMY_STATE_STUN) {
            // Stunned / Melting state from 6502 ROM (Sprite 0x1B shocked face or 0x14..0x17 melt)
            int spr = (e->frame == 0) ? 0x1B : (0x14 + ((e->frame - 1) % 4));
            draw_sprite_16x16(fb, sx, sy, spr, PAL_VALCOM, false, false);
            continue;
        }

        // Normal Walking State
        int walk_step = e->frame % 4;
        static const uint8_t anim_steps[4] = { 0, 1, 2, 1 };
        int spr = (0x18 + (e->type - 1) * 4) + anim_steps[walk_step];
        const uint8_t *pal = PAL_VALCOM;
        if (e->type == ENEMY_VALCOM) pal = PAL_VALCOM;
        else if (e->type == ENEMY_ONEAL) pal = PAL_ONEAL;
        else if (e->type == ENEMY_DAHL) pal = PAL_DAHL;
        else if (e->type == ENEMY_MINVO) pal = PAL_VALCOM;
        else if (e->type == ENEMY_DORIA) pal = PAL_DAHL;
        else if (e->type == ENEMY_OVAPE) pal = PAL_ONEAL;
        else if (e->type == ENEMY_PASS) pal = PAL_VALCOM;
        else if (e->type == ENEMY_PONTAN) pal = PAL_FIRE;
        draw_sprite_16x16(fb, sx, sy, spr, pal, (e->dir == DIR_LEFT), false);
    }

    // 6. Draw Bomberman (Walking & Authentic 8-stage Death Melting Progression)
    const Player *p = &game->player;
    int psx = p->x * TILE_SIZE + p->u - TILE_HALFSIZE - h_scroll;
    int psy = HUD_HEIGHT + p->y * TILE_SIZE + p->v - TILE_HALFSIZE;

    if (p->dying) {
        // Frames 12..18 map to Sprites 9..15 from BOMBER_ANIM in BMAN.NAS
        int stage = p->death_frame - 12;
        if (stage < 0) stage = 0;
        if (stage > 6) stage = 6;
        int spr = 9 + stage;
        draw_sprite_16x16(fb, psx, psy, spr, PAL_BOMBER, false, false);
    } else {
        if (game->invulnerable_timer == 0 || (game->frame_cnt % 4 < 2)) {
            int spr = 3; // default down
            bool flip_x = false;
            int walk_step = p->frame % 4;
            static const uint8_t walk_h[4] = { 0, 1, 2, 1 };
            static const uint8_t walk_d[4] = { 3, 4, 5, 4 };
            static const uint8_t walk_u[4] = { 6, 7, 8, 7 };

            if (p->dir == DIR_RIGHT) {
                spr = walk_h[walk_step];
                flip_x = true;
            } else if (p->dir == DIR_LEFT) {
                spr = walk_h[walk_step];
                flip_x = false;
            } else if (p->dir == DIR_DOWN) {
                spr = walk_d[walk_step];
            } else if (p->dir == DIR_UP) {
                spr = walk_u[walk_step];
            }

            draw_sprite_16x16(fb, psx, psy, spr, PAL_BOMBER, flip_x, false);
        }
    }

    // 7. Clear Bottom 8-pixel margin (232..239)
    for (int py = 232; py < NES_SCREEN_H; py++) {
        for (int px = 0; px < sw; px++) {
            fb[py * sw + px] = NES_PALETTE[0x0F];
        }
    }

    // 8. Game Over Overlay
    if (game->game_over) {
        // Darken background
        for (int y = HUD_HEIGHT; y < 232; y++) {
            for (int x = 0; x < sw; x++) {
                uint32_t px = fb[y * sw + x];
                uint32_t r = ((px >> 16) & 0xFF) / 4;
                uint32_t g = ((px >> 8) & 0xFF) / 4;
                uint32_t b = (px & 0xFF) / 4;
                fb[y * sw + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
            }
        }
        const char *go_txt = "OYUN BİTTİ";
        font_draw_string(fb, sw, (sw - font_string_width(go_txt)) / 2, 112, go_txt, PAL_HUD);

        if ((game->frame_cnt / 30) % 2 == 0) {
            const char *press_txt = "YENİDEN: R   ANA MENÜ: ESC";
            font_draw_string(fb, sw, (sw - font_string_width(press_txt)) / 2, 128, press_txt, PAL_HUD);
        }
    }
}

void graphics_draw_intro(uint32_t *fb, int sw, int frame_cnt) {
    s_screen_w = sw;
    for (int i = 0; i < sw * NES_SCREEN_H; i++) {
        fb[i] = NES_PALETTE[0x0F]; // Solid black
    }

    // Outer decorative border
    int bx0 = 12, by0 = 12;
    int bx1 = sw - 13, by1 = NES_SCREEN_H - 13;
    for (int y = by0; y <= by1; y++) {
        for (int x = bx0; x <= bx1; x++) {
            if (x == bx0 || x == bx1 || y == by0 || y == by1) {
                fb[y * sw + x] = NES_PALETTE[0x28]; // Gold border
            }
        }
    }

    static const uint8_t PAL_GOLD[4] = { 0x0F, 0x16, 0x0F, 0x28 };

    // Title
    const char *title1 = "BOMBERMAN 1985";
    const char *title2 = "MEC REA EDITION";
    font_draw_string(fb, sw, (sw - font_string_width(title1)) / 2, by0 + 14, title1, PAL_GOLD);
    font_draw_string(fb, sw, (sw - font_string_width(title2)) / 2, by0 + 26, title2, PAL_HUD);

    // Center animated Bomberman & Bomb
    int walk_step = (frame_cnt / 10) % 4;
    static const uint8_t walk_d[4] = { 3, 4, 5, 4 };
    draw_sprite_16x16(fb, (sw / 2) - 18, by0 + 44, walk_d[walk_step], PAL_BOMBER, false, false);

    int bomb_step = (frame_cnt / 16) % 3;
    draw_metatile_16x16(fb, (sw / 2) + 6, by0 + 44, TILE_BOMB[bomb_step], PAL_FIRE);

    // Credits
    const char *c1 = "EMİNE HAKSEVER";
    const char *c2 = "&";
    const char *c3 = "MEHMET ERTUĞRUL ÇAKMAK";
    const char *c4 = "TARAFINDAN MODERNİZE EDİLMİŞTİR";

    font_draw_string(fb, sw, (sw - font_string_width(c1)) / 2, by0 + 74, c1, PAL_HUD);
    font_draw_string(fb, sw, (sw - font_string_width(c2)) / 2, by0 + 88, c2, PAL_GOLD);
    font_draw_string(fb, sw, (sw - font_string_width(c3)) / 2, by0 + 102, c3, PAL_HUD);
    font_draw_string(fb, sw, (sw - font_string_width(c4)) / 2, by0 + 124, c4, PAL_GOLD);

    // Blinking prompt
    if ((frame_cnt / 30) % 2 == 0) {
        const char *prompt = "BAŞLAMAK İÇİN BİR TUŞA BASIN";
        font_draw_string(fb, sw, (sw - font_string_width(prompt)) / 2, by1 - 22, prompt, PAL_HUD);
    }
}

void graphics_draw_main_menu(uint32_t *fb, int sw, int frame_cnt, int selected_item) {
    s_screen_w = sw;
    for (int i = 0; i < sw * NES_SCREEN_H; i++) {
        fb[i] = NES_PALETTE[0x0F]; // Solid black
    }

    // Outer decorative border
    int bx0 = 12, by0 = 10;
    int bx1 = sw - 13, by1 = NES_SCREEN_H - 11;
    for (int y = by0; y <= by1; y++) {
        for (int x = bx0; x <= bx1; x++) {
            if (x == bx0 || x == bx1 || y == by0 || y == by1) {
                fb[y * sw + x] = NES_PALETTE[0x28]; // Gold border
            } else if (x == bx0 + 1 || x == bx1 - 1 || y == by0 + 1 || y == by1 - 1) {
                fb[y * sw + x] = NES_PALETTE[0x00]; // Dark grey inner
            }
        }
    }

    static const uint8_t PAL_GOLD[4] = { 0x0F, 0x16, 0x0F, 0x28 };

    // Title
    const char *title1 = "BOMBERMAN 1985";
    const char *title2 = "MEC REA EDITION";
    font_draw_string(fb, sw, (sw - font_string_width(title1)) / 2, by0 + 12, title1, PAL_GOLD);
    font_draw_string(fb, sw, (sw - font_string_width(title2)) / 2, by0 + 24, title2, PAL_HUD);

    // Center animated Bomberman & Bomb
    int walk_step = (frame_cnt / 10) % 4;
    static const uint8_t walk_d[4] = { 3, 4, 5, 4 };
    draw_sprite_16x16(fb, (sw / 2) - 18, by0 + 40, walk_d[walk_step], PAL_BOMBER, false, false);

    int bomb_step = (frame_cnt / 16) % 3;
    draw_metatile_16x16(fb, (sw / 2) + 6, by0 + 40, TILE_BOMB[bomb_step], PAL_FIRE);

    // Credits
    const char *c1 = "EMİNE HAKSEVER & MEHMET ERTUĞRUL ÇAKMAK";
    const char *c2 = "TARAFINDAN MODERNİZE EDİLMİŞTİR";
    font_draw_string(fb, sw, (sw - font_string_width(c1)) / 2, by0 + 68, c1, PAL_HUD);
    font_draw_string(fb, sw, (sw - font_string_width(c2)) / 2, by0 + 82, c2, PAL_GOLD);

    // Divider line
    int div_x0 = (sw - 230) / 2;
    int div_x1 = div_x0 + 230;
    for (int x = div_x0; x <= div_x1; x++) {
        fb[(by0 + 98) * sw + x] = NES_PALETTE[0x10]; // Grey divider line
    }

    // Menu Items
    const char *menu_items[4] = {
        "OYUNU BAŞLAT",
        "OYUN AYARLARI",
        "SKORLAR",
        "ÇIKIŞ"
    };

    int menu_start_y = by0 + 108;
    int line_h = 16;
    int menu_box_x = (sw - 140) / 2;

    for (int i = 0; i < 4; i++) {
        int y = menu_start_y + i * line_h;
        bool is_sel = (selected_item == i);
        const uint8_t *pal = is_sel ? PAL_GOLD : PAL_HUD;

        if (is_sel) {
            font_draw_char(fb, sw, menu_box_x - 14, y, '>', PAL_GOLD);
        }
        font_draw_string(fb, sw, menu_box_x, y, menu_items[i], pal);
    }

    // Bottom Help Bar
    const char *help = "YÖN TUŞLARI: GEZİN   ENTER: SEÇ";
    font_draw_string(fb, sw, (sw - font_string_width(help)) / 2, by1 - 18, help, PAL_HUD);
}
