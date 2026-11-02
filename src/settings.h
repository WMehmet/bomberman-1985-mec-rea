#ifndef BOMBERMAN_SETTINGS_H
#define BOMBERMAN_SETTINGS_H

#include "types.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct {
    SDL_Scancode key_up;
    SDL_Scancode key_down;
    SDL_Scancode key_left;
    SDL_Scancode key_right;
    SDL_Scancode key_bomb;
    SDL_Scancode key_detonate;

    bool sound_enabled;
    int window_scale; // 2, 3, 4, or 0 for fullscreen
    bool fullscreen;

    bool speed_fast;
    bool invulnerable;

    bool menu_active;
    bool return_to_main_menu;
    bool quit_requested;
    int selected_item;
    int rebinding_index; // -1 if not rebinding
} Settings;

extern Settings g_settings;

void settings_init(void);
void settings_set_screen_width(int sw);
void settings_toggle_menu(void);
bool settings_handle_event(const SDL_Event *event, GameState *game, SDL_Window *window);
void settings_draw_menu(uint32_t *fb);

#endif // BOMBERMAN_SETTINGS_H
