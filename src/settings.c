#include "settings.h"
#include "game.h"
#include "font.h"
#include "audio.h"
#include <stdio.h>
#include <string.h>

Settings g_settings;

static const uint32_t NES_PAL[64] = {
    0xFF7C7C7C, 0xFF0000FC, 0xFF0000BC, 0xFF4428BC, 0xFF940084, 0xFFA80020, 0xFFA81000, 0xFF881400,
    0xFF503000, 0xFF007800, 0xFF006800, 0xFF005800, 0xFF004058, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFBCBCBC, 0xFF0078F8, 0xFF0058F8, 0xFF6844FC, 0xFFD800CC, 0xFFE40058, 0xFFF83800, 0xFFE45C10,
    0xFFAC7C00, 0xFF00B800, 0xFF00A800, 0xFF00A844, 0xFF008888, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFF8F8F8, 0xFF3CBCFC, 0xFF6888FC, 0xFF9878F8, 0xFFF878F8, 0xFFF85898, 0xFFF87858, 0xFFFCA044,
    0xFFF8B800, 0xFFB8F818, 0xFF58D854, 0xFF58F898, 0xFF00E8D8, 0xFF787878, 0xFF000000, 0xFF000000,
    0xFFFCFCFC, 0xFFA4E4FC, 0xFFB8B8F8, 0xFFD8B8F8, 0xFFF8B8F8, 0xFFF8A4C0, 0xFFF0D0B0, 0xFFFCE0A8,
    0xFFF8D878, 0xFFD8F878, 0xFFB8F8B8, 0xFFB8F8D8, 0xFF00FCFC, 0xFFF8D8F8, 0xFF000000, 0xFF000000
};

static const uint8_t PAL_MENU_TEXT[4] = { 0x0F, 0x10, 0x0F, 0x30 }; // Black, Grey shadow, Black, White text
static const uint8_t PAL_MENU_GOLD[4] = { 0x0F, 0x16, 0x0F, 0x28 }; // Black, Red shadow, Black, Gold text

enum MenuItem {
    MENU_RESUME = 0,
    MENU_UP,
    MENU_DOWN,
    MENU_LEFT,
    MENU_RIGHT,
    MENU_BOMB,
    MENU_DETONATE,
    MENU_SOUND,
    MENU_SCALE,
    MENU_SPEED,
    MENU_GODMODE,
    MENU_MAINMENU,
    MENU_RESTART,
    MENU_QUIT,
    MENU_COUNT
};

void settings_init(void) {
    g_settings.key_up       = SDL_SCANCODE_UP;
    g_settings.key_down     = SDL_SCANCODE_DOWN;
    g_settings.key_left     = SDL_SCANCODE_LEFT;
    g_settings.key_right    = SDL_SCANCODE_RIGHT;
    g_settings.key_bomb     = SDL_SCANCODE_Z;
    g_settings.key_detonate = SDL_SCANCODE_X;

    g_settings.sound_enabled = true;
    g_settings.window_scale  = 3;
    g_settings.fullscreen    = true;

    g_settings.speed_fast   = false;
    g_settings.invulnerable = false;

    g_settings.menu_active     = false;
    g_settings.return_to_main_menu = false;
    g_settings.quit_requested  = false;
    g_settings.selected_item   = 0;
    g_settings.rebinding_index = -1;
}

void settings_toggle_menu(void) {
    g_settings.menu_active = !g_settings.menu_active;
    g_settings.rebinding_index = -1;
}

static int s_settings_screen_w = DEFAULT_SCREEN_W;

void settings_set_screen_width(int sw) {
    if (sw >= 256 && sw <= MAX_SCREEN_W) {
        s_settings_screen_w = sw;
    }
}

static void apply_scale(SDL_Window *window) {
    if (g_settings.fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        SDL_SetWindowFullscreen(window, 0);
        int w = s_settings_screen_w * g_settings.window_scale;
        int h = NES_SCREEN_H * g_settings.window_scale;
        SDL_SetWindowSize(window, w, h);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
}


bool settings_handle_event(const SDL_Event *event, GameState *game, SDL_Window *window) {
    if (event->type != SDL_KEYDOWN) return false;

    SDL_Scancode sc = event->key.keysym.scancode;

    // If waiting for a new key binding
    if (g_settings.rebinding_index != -1) {
        if (sc == SDL_SCANCODE_ESCAPE) {
            // Cancel rebinding
            g_settings.rebinding_index = -1;
            return true;
        }

        switch (g_settings.rebinding_index) {
            case MENU_UP:       g_settings.key_up = sc; break;
            case MENU_DOWN:     g_settings.key_down = sc; break;
            case MENU_LEFT:     g_settings.key_left = sc; break;
            case MENU_RIGHT:    g_settings.key_right = sc; break;
            case MENU_BOMB:     g_settings.key_bomb = sc; break;
            case MENU_DETONATE: g_settings.key_detonate = sc; break;
        }
        g_settings.rebinding_index = -1;
        audio_play_sound(SND_PLANT_BOMB);
        return true;
    }

    // Toggle Menu on ESC
    if (sc == SDL_SCANCODE_ESCAPE) {
        settings_toggle_menu();
        return true;
    }

    if (!g_settings.menu_active) return false;

    // Navigate Menu Items
    if (sc == SDL_SCANCODE_UP || sc == SDL_SCANCODE_W) {
        g_settings.selected_item = (g_settings.selected_item - 1 + MENU_COUNT) % MENU_COUNT;
        audio_play_sound(SND_STEP);
        return true;
    }
    if (sc == SDL_SCANCODE_DOWN || sc == SDL_SCANCODE_S) {
        g_settings.selected_item = (g_settings.selected_item + 1) % MENU_COUNT;
        audio_play_sound(SND_STEP);
        return true;
    }

    // Select / Modify Active Item
    if (sc == SDL_SCANCODE_RETURN || sc == SDL_SCANCODE_SPACE || sc == SDL_SCANCODE_RIGHT || sc == SDL_SCANCODE_D) {
        switch (g_settings.selected_item) {
            case MENU_RESUME:
                g_settings.menu_active = false;
                break;

            case MENU_UP:
            case MENU_DOWN:
            case MENU_LEFT:
            case MENU_RIGHT:
            case MENU_BOMB:
            case MENU_DETONATE:
                g_settings.rebinding_index = g_settings.selected_item;
                break;

            case MENU_SOUND:
                g_settings.sound_enabled = !g_settings.sound_enabled;
                break;

            case MENU_SCALE:
                if (!g_settings.fullscreen) {
                    if (g_settings.window_scale == 2) g_settings.window_scale = 3;
                    else if (g_settings.window_scale == 3) g_settings.window_scale = 4;
                    else {
                        g_settings.fullscreen = true;
                    }
                } else {
                    g_settings.fullscreen = false;
                    g_settings.window_scale = 2;
                }
                apply_scale(window);
                break;

            case MENU_SPEED:
                g_settings.speed_fast = !g_settings.speed_fast;
                game->bonus_speed = g_settings.speed_fast;
                break;

            case MENU_GODMODE:
                g_settings.invulnerable = !g_settings.invulnerable;
                game->invulnerable_timer = g_settings.invulnerable ? 9999999 : 0;
                break;

            case MENU_MAINMENU:
                g_settings.menu_active = false;
                g_settings.return_to_main_menu = true;
                break;

            case MENU_RESTART:
                game_init(game, game->stage);
                g_settings.menu_active = false;
                break;

            case MENU_QUIT:
                g_settings.quit_requested = true;
                g_settings.menu_active = false;
                break;
        }
        audio_play_sound(SND_POWERUP);
        return true;
    }

    return true; // Consume event while in menu
}

static const char *get_key_name(SDL_Scancode sc) {
    switch (sc) {
        case SDL_SCANCODE_UP: return "YUKARI";
        case SDL_SCANCODE_DOWN: return "AŞAĞI";
        case SDL_SCANCODE_LEFT: return "SOL";
        case SDL_SCANCODE_RIGHT: return "SAĞ";
        case SDL_SCANCODE_SPACE: return "BOŞLUK";
        case SDL_SCANCODE_RETURN: return "ENTER";
        case SDL_SCANCODE_ESCAPE: return "ESC";
        case SDL_SCANCODE_LSHIFT: case SDL_SCANCODE_RSHIFT: return "SHIFT";
        case SDL_SCANCODE_LCTRL: case SDL_SCANCODE_RCTRL: return "CTRL";
        case SDL_SCANCODE_LALT: case SDL_SCANCODE_RALT: return "ALT";
        case SDL_SCANCODE_TAB: return "TAB";
        default: {
            const char *name = SDL_GetScancodeName(sc);
            if (!name || strlen(name) == 0) return "TUŞ";
            return name;
        }
    }
}

void settings_draw_menu(uint32_t *fb) {
    if (!g_settings.menu_active) return;
    int sw = s_settings_screen_w;

    // 1. Darken Background Overlay (Semi-transparent blend)
    for (int y = 0; y < NES_SCREEN_H; y++) {
        for (int x = 0; x < sw; x++) {
            uint32_t px = fb[y * sw + x];
            uint32_t r = ((px >> 16) & 0xFF) / 3;
            uint32_t g = ((px >> 8) & 0xFF) / 3;
            uint32_t b = (px & 0xFF) / 3;
            fb[y * sw + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
    }

    // 2. Draw Menu Box (Centered: width = 236px)
    int box_w = 236;
    int bx0 = (sw - box_w) / 2, by0 = 10;
    int bx1 = bx0 + box_w, by1 = 228;
    for (int y = by0; y <= by1; y++) {
        for (int x = bx0; x <= bx1; x++) {
            if (x == bx0 || x == bx1 || y == by0 || y == by1) {
                fb[y * sw + x] = NES_PAL[0x30]; // White border
            } else if (x == bx0 + 1 || x == bx1 - 1 || y == by0 + 1 || y == by1 - 1) {
                fb[y * sw + x] = NES_PAL[0x00]; // Dark grey inner border
            } else {
                fb[y * sw + x] = NES_PAL[0x0F]; // Solid black inside
            }
        }
    }

    // 3. Menu Title
    const char *title = "AYARLAR";
    font_draw_string(fb, sw, (sw - font_string_width(title)) / 2, by0 + 7, title, PAL_MENU_GOLD);

    // 4. Menu Items
    const int start_y = by0 + 23;
    const int line_h = 13;

    char line_buf[64];

    for (int i = 0; i < MENU_COUNT; i++) {
        int y = start_y + i * line_h;
        bool is_sel = (g_settings.selected_item == i);
        const uint8_t *pal = is_sel ? PAL_MENU_GOLD : PAL_MENU_TEXT;

        if (is_sel) {
            font_draw_char(fb, sw, bx0 + 6, y, '>', PAL_MENU_GOLD);
        }

        switch (i) {
            case MENU_RESUME:
                font_draw_string(fb, sw, bx0 + 18, y, "OYUNA DEVAM ET", pal);
                break;
            case MENU_UP:
                snprintf(line_buf, sizeof(line_buf), "YUKARI:     %s",
                    (g_settings.rebinding_index == i) ? "TUŞA BASIN" : get_key_name(g_settings.key_up));
                font_draw_string(fb, sw, bx0 + 18, y, line_buf, pal);
                break;
            case MENU_DOWN:
                snprintf(line_buf, sizeof(line_buf), "AŞAĞI:      %s",
                    (g_settings.rebinding_index == i) ? "TUŞA BASIN" : get_key_name(g_settings.key_down));
                font_draw_string(fb, sw, bx0 + 18, y, line_buf, pal);
                break;
            case MENU_LEFT:
                snprintf(line_buf, sizeof(line_buf), "SOL:        %s",
                    (g_settings.rebinding_index == i) ? "TUŞA BASIN" : get_key_name(g_settings.key_left));
                font_draw_string(fb, sw, bx0 + 18, y, line_buf, pal);
                break;
            case MENU_RIGHT:
                snprintf(line_buf, sizeof(line_buf), "SAĞ:        %s",
                    (g_settings.rebinding_index == i) ? "TUŞA BASIN" : get_key_name(g_settings.key_right));
                font_draw_string(fb, sw, bx0 + 18, y, line_buf, pal);
                break;
            case MENU_BOMB:
                snprintf(line_buf, sizeof(line_buf), "BOMBA:      %s",
                    (g_settings.rebinding_index == i) ? "TUŞA BASIN" : get_key_name(g_settings.key_bomb));
                font_draw_string(fb, sw, bx0 + 18, y, line_buf, pal);
                break;
            case MENU_DETONATE:
                snprintf(line_buf, sizeof(line_buf), "TETİK:      %s",
                    (g_settings.rebinding_index == i) ? "TUŞA BASIN" : get_key_name(g_settings.key_detonate));
                font_draw_string(fb, sw, bx0 + 18, y, line_buf, pal);
                break;
            case MENU_SOUND:
                snprintf(line_buf, sizeof(line_buf), "SES:        %s", g_settings.sound_enabled ? "AÇIK" : "KAPALI");
                font_draw_string(fb, sw, bx0 + 18, y, line_buf, pal);
                break;
            case MENU_SCALE:
                if (g_settings.fullscreen) {
                    font_draw_string(fb, sw, bx0 + 18, y, "EKRAN:      TAM EKRAN", pal);
                } else {
                    snprintf(line_buf, sizeof(line_buf), "EKRAN:      %dX", g_settings.window_scale);
                    font_draw_string(fb, sw, bx0 + 18, y, line_buf, pal);
                }
                break;
            case MENU_SPEED:
                snprintf(line_buf, sizeof(line_buf), "HIZ:        %s", g_settings.speed_fast ? "HIZLI" : "NORMAL");
                font_draw_string(fb, sw, bx0 + 18, y, line_buf, pal);
                break;
            case MENU_GODMODE:
                snprintf(line_buf, sizeof(line_buf), "ÖLÜMSÜZLÜK: %s", g_settings.invulnerable ? "AÇIK" : "KAPALI");
                font_draw_string(fb, sw, bx0 + 18, y, line_buf, pal);
                break;
            case MENU_MAINMENU:
                font_draw_string(fb, sw, bx0 + 18, y, "ANA MENÜYE DÖN", pal);
                break;
            case MENU_RESTART:
                font_draw_string(fb, sw, bx0 + 18, y, "BÖLÜMÜ YENİDEN BAŞLAT", pal);
                break;
            case MENU_QUIT:
                font_draw_string(fb, sw, bx0 + 18, y, "OYUNDAN ÇIK", pal);
                break;
        }
    }

    // 5. Bottom Help Bar
    const char *help = "YÖN: GEZİN   ENTER: SEÇ   ESC: ÇIKIŞ";
    font_draw_string(fb, sw, (sw - font_string_width(help)) / 2, by1 - 10, help, PAL_MENU_TEXT);
}
