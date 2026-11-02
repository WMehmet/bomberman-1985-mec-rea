#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
#else
  #define _POSIX_C_SOURCE 199309L
  #include <unistd.h>
#endif

#include "types.h"
#include "game.h"
#include "graphics.h"
#include "audio.h"
#include "settings.h"
#include "scores.h"
#include "icon_data.h"

typedef enum {
    STATE_MAIN_MENU = 0,
    STATE_PLAYING,
    STATE_SETTINGS_IN_MENU,
    STATE_SCORES
} AppState;

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WINDOW_SCALE 3
#define WINDOW_WIDTH  (DEFAULT_SCREEN_W * WINDOW_SCALE)
#define WINDOW_HEIGHT (NES_SCREEN_H * WINDOW_SCALE)

static int get_current_screen_w(SDL_Window *window) {
    int win_w = 0, win_h = 0;
    SDL_GetWindowSize(window, &win_w, &win_h);
    if (win_h <= 0) win_h = 240;
    float aspect = (float)win_w / (float)win_h;
    int sw = (int)roundf(240.0f * aspect);
    if (sw % 2 != 0) sw++;
    if (sw < 256) sw = 256;
    if (sw > PLAYFIELD_COLS * TILE_SIZE) sw = PLAYFIELD_COLS * TILE_SIZE;
    return sw;
}

static uint64_t get_time_ns(void) {
    static uint64_t perf_freq = 0;
    if (perf_freq == 0) {
        perf_freq = SDL_GetPerformanceFrequency();
    }
    uint64_t counter = SDL_GetPerformanceCounter();
    return (uint64_t)(((double)counter / (double)perf_freq) * 1000000000.0);
}

static void sleep_ns(uint64_t ns) {
#ifdef _WIN32
    DWORD ms = (DWORD)(ns / 1000000ULL);
    if (ms > 0) {
        Sleep(ms);
    }
#else
    struct timespec sleep_req;
    sleep_req.tv_sec = (time_t)(ns / 1000000000ULL);
    sleep_req.tv_nsec = (long)(ns % 1000000000ULL);
    nanosleep(&sleep_req, NULL);
#endif
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "0");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Bomberman Başlatma Hatası", SDL_GetError(), NULL);
            return 1;
        }
    }

    settings_init();

    SDL_Window *window = SDL_CreateWindow(
        "Bomberman 1985 MEC REA",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | (g_settings.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)
    );

    if (!window) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Pencere Hatası", SDL_GetError(), NULL);
        SDL_Quit();
        return 1;
    }

    // Set embedded application icon (works on both Linux and Windows window titlebar/dock)
    SDL_Surface *icon_surface = SDL_CreateRGBSurfaceWithFormatFrom(
        (void *)ICON_RGBA_PIXELS,
        ICON_WIDTH,
        ICON_HEIGHT,
        32,
        ICON_WIDTH * 4,
        SDL_PIXELFORMAT_RGBA32
    );
    if (icon_surface) {
        SDL_SetWindowIcon(window, icon_surface);
        SDL_FreeSurface(icon_surface);
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED
    );

    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }

    if (!renderer) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Grafik Motoru Hatası", SDL_GetError(), window);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int cur_sw = get_current_screen_w(window);
    graphics_set_screen_width(cur_sw);
    settings_set_screen_width(cur_sw);
    SDL_RenderSetLogicalSize(renderer, cur_sw, NES_SCREEN_H);

    SDL_Texture *frame_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        cur_sw, NES_SCREEN_H
    );

    if (!frame_texture) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    uint32_t *framebuffer = (uint32_t *)malloc(MAX_SCREEN_W * NES_SCREEN_H * sizeof(uint32_t));
    if (!framebuffer) {
        fprintf(stderr, "Failed to allocate framebuffer memory\n");
        return 1;
    }

    graphics_init();
    audio_init();
    scores_init();

    GameState game;
    game.screen_w = cur_sw;
    game_init(&game, 1);

    bool running = true;
    AppState app_state = STATE_MAIN_MENU;
    int menu_selected = 0;
    bool score_recorded = false;
    uint32_t app_frame_cnt = 0;
    uint8_t prev_buttons = 0;

    const uint64_t FRAME_DURATION_NS = 16639267ULL; // 60.0988 Hz NTSC
    uint64_t next_frame_time = get_time_ns();

    printf("\n=======================================================\n");
    printf("                Bomberman 1985 MEC REA                \n");
    printf("=======================================================\n");
    printf(" Menü:\n");
    printf("   1. OYUNU BAŞLAT\n");
    printf("   2. OYUN AYARLARI\n");
    printf("   3. SKORLAR (Tarih Kayıtlı)\n");
    printf("   4. ÇIKIŞ\n");
    printf(" Kontroller:\n");
    printf("   Ok Tuşları / WASD : Hareket / Menü Gezinme\n");
    printf("   ENTER / BOŞLUK / Z: Seçim / Bomba Bırak\n");
    printf("   X                 : Uzaktan Tetikleme (Bonus varken)\n");
    printf("   R                 : Bölümü Yeniden Başlat\n");
    printf("   ESC               : Ayarlar Menüsü / Ana Menüye Dönüş\n");
    printf("   F11               : Tam Ekran / Pencere Modu\n");
    printf("=======================================================\n\n");

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                    event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    int new_sw = get_current_screen_w(window);
                    if (new_sw != cur_sw) {
                        cur_sw = new_sw;
                        game.screen_w = cur_sw;
                        graphics_set_screen_width(cur_sw);
                        settings_set_screen_width(cur_sw);
                        SDL_RenderSetLogicalSize(renderer, cur_sw, NES_SCREEN_H);
                        if (frame_texture) SDL_DestroyTexture(frame_texture);
                        frame_texture = SDL_CreateTexture(
                            renderer,
                            SDL_PIXELFORMAT_ARGB8888,
                            SDL_TEXTUREACCESS_STREAMING,
                            cur_sw, NES_SCREEN_H
                        );
                    }
                }
            } else if (event.type == SDL_KEYDOWN) {
                SDL_Keycode sym = event.key.keysym.sym;

                if (sym == SDLK_F11) {
                    g_settings.fullscreen = !g_settings.fullscreen;
                    if (g_settings.fullscreen) {
                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    } else {
                        SDL_SetWindowFullscreen(window, 0);
                        int w = cur_sw * g_settings.window_scale;
                        int h = NES_SCREEN_H * g_settings.window_scale;
                        SDL_SetWindowSize(window, w, h);
                        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                    }
                    int new_sw = get_current_screen_w(window);
                    if (new_sw != cur_sw) {
                        cur_sw = new_sw;
                        game.screen_w = cur_sw;
                        graphics_set_screen_width(cur_sw);
                        settings_set_screen_width(cur_sw);
                        SDL_RenderSetLogicalSize(renderer, cur_sw, NES_SCREEN_H);
                        if (frame_texture) SDL_DestroyTexture(frame_texture);
                        frame_texture = SDL_CreateTexture(
                            renderer,
                            SDL_PIXELFORMAT_ARGB8888,
                            SDL_TEXTUREACCESS_STREAMING,
                            cur_sw, NES_SCREEN_H
                        );
                    }
                    continue;
                }

                if (app_state == STATE_MAIN_MENU) {
                    if (sym == SDLK_UP || sym == SDLK_w) {
                        menu_selected = (menu_selected - 1 + 4) % 4;
                        audio_play_sound(SND_STEP);
                    } else if (sym == SDLK_DOWN || sym == SDLK_s) {
                        menu_selected = (menu_selected + 1) % 4;
                        audio_play_sound(SND_STEP);
                    } else if (sym == SDLK_RETURN || sym == SDLK_SPACE || sym == SDLK_z) {
                        if (menu_selected == 0) {
                            game_init(&game, 1);
                            score_recorded = false;
                            app_state = STATE_PLAYING;
                            audio_play_sound(SND_POWERUP);
                            printf(" Yeni Oyun Başlatıldı (Seviye 1)\n");
                        } else if (menu_selected == 1) {
                            g_settings.menu_active = true;
                            app_state = STATE_SETTINGS_IN_MENU;
                            audio_play_sound(SND_POWERUP);
                        } else if (menu_selected == 2) {
                            app_state = STATE_SCORES;
                            audio_play_sound(SND_POWERUP);
                        } else if (menu_selected == 3) {
                            running = false;
                        }
                    } else if (sym == SDLK_ESCAPE) {
                        running = false;
                    }
                    continue;
                }

                if (app_state == STATE_SETTINGS_IN_MENU) {
                    settings_handle_event(&event, &game, window);
                    if (g_settings.quit_requested) {
                        running = false;
                    } else if (g_settings.return_to_main_menu || !g_settings.menu_active) {
                        g_settings.return_to_main_menu = false;
                        g_settings.menu_active = false;
                        app_state = STATE_MAIN_MENU;
                    }
                    continue;
                }

                if (app_state == STATE_SCORES) {
                    if (sym == SDLK_ESCAPE || sym == SDLK_RETURN || sym == SDLK_SPACE || sym == SDLK_z) {
                        app_state = STATE_MAIN_MENU;
                        audio_play_sound(SND_STEP);
                    }
                    continue;
                }

                if (app_state == STATE_PLAYING) {
                    if (game.game_over) {
                        if (!score_recorded && game.score > 0) {
                            scores_add(game.score, game.stage);
                            score_recorded = true;
                        }
                        if (sym == SDLK_r || sym == SDLK_RETURN || sym == SDLK_SPACE || sym == SDLK_z) {
                            game_init(&game, 1);
                            score_recorded = false;
                            printf(" Oyun Baştan Başlatıldı\n");
                            continue;
                        } else if (sym == SDLK_ESCAPE) {
                            app_state = STATE_MAIN_MENU;
                            continue;
                        }
                    } else if (sym == SDLK_r && !g_settings.menu_active) {
                        game_init(&game, game.stage);
                        printf(" Seviye Yeniden Başlatıldı (PRNG Seed: 0x%02X 0x%02X 0x%02X 0x%02X)\n",
                               game.seed[0], game.seed[1], game.seed[2], game.seed[3]);
                        continue;
                    }

                    // Settings in-game (ESC toggles it, or inputs while menu is open)
                    settings_handle_event(&event, &game, window);
                    if (g_settings.quit_requested) {
                        running = false;
                        continue;
                    }
                    if (g_settings.return_to_main_menu) {
                        if (!score_recorded && game.score > 0) {
                            scores_add(game.score, game.stage);
                            score_recorded = true;
                        }
                        g_settings.return_to_main_menu = false;
                        g_settings.menu_active = false;
                        app_state = STATE_MAIN_MENU;
                        continue;
                    }
                    continue;
                }
            }
        }

        app_frame_cnt++;

        if (app_state == STATE_MAIN_MENU) {
            graphics_draw_main_menu(framebuffer, cur_sw, app_frame_cnt, menu_selected);
        } else if (app_state == STATE_SETTINGS_IN_MENU) {
            graphics_draw_main_menu(framebuffer, cur_sw, app_frame_cnt, menu_selected);
            settings_draw_menu(framebuffer);
        } else if (app_state == STATE_SCORES) {
            scores_draw_screen(framebuffer, cur_sw, app_frame_cnt);
        } else if (app_state == STATE_PLAYING) {
            if (g_settings.return_to_main_menu) {
                if (!score_recorded && game.score > 0) {
                    scores_add(game.score, game.stage);
                    score_recorded = true;
                }
                g_settings.return_to_main_menu = false;
                g_settings.menu_active = false;
                app_state = STATE_MAIN_MENU;
                graphics_draw_main_menu(framebuffer, cur_sw, app_frame_cnt, menu_selected);
            } else {
                if (!g_settings.menu_active) {
                    const Uint8 *keys = SDL_GetKeyboardState(NULL);
                    uint8_t input_mask = 0;

                    if (keys[g_settings.key_right] || keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) input_mask |= BTN_RIGHT;
                    if (keys[g_settings.key_left]  || keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A]) input_mask |= BTN_LEFT;
                    if (keys[g_settings.key_down]  || keys[SDL_SCANCODE_DOWN]  || keys[SDL_SCANCODE_S]) input_mask |= BTN_DOWN;
                    if (keys[g_settings.key_up]    || keys[SDL_SCANCODE_UP]    || keys[SDL_SCANCODE_W]) input_mask |= BTN_UP;

                    bool bomb_down = keys[g_settings.key_bomb] || keys[SDL_SCANCODE_Z] || keys[SDL_SCANCODE_SPACE];
                    if (bomb_down && !(prev_buttons & BTN_A)) {
                        input_mask |= BTN_A;
                    }
                    if (bomb_down) {
                        prev_buttons |= BTN_A;
                    } else {
                        prev_buttons &= ~BTN_A;
                    }

                    if (keys[g_settings.key_detonate] || keys[SDL_SCANCODE_X]) {
                        input_mask |= BTN_B;
                    }

                    game_tick(&game, input_mask);

                    if (game.game_over && !score_recorded && game.score > 0) {
                        scores_add(game.score, game.stage);
                        score_recorded = true;
                    }
                }

                graphics_render_frame(&game, framebuffer);

                if (g_settings.menu_active) {
                    settings_draw_menu(framebuffer);
                }
            }
        }

        // 4. Update Screen
        SDL_UpdateTexture(frame_texture, NULL, framebuffer, cur_sw * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, frame_texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // 5. Zero-lag Hybrid Frame Pacing
        next_frame_time += FRAME_DURATION_NS;
        uint64_t current_time = get_time_ns();

        if (current_time < next_frame_time) {
            uint64_t sleep_duration = next_frame_time - current_time;
            if (sleep_duration > 2000000ULL) {
                sleep_ns(sleep_duration - 1500000ULL);
            }
            while (get_time_ns() < next_frame_time) {
                #if defined(__x86_64__) || defined(_M_X64)
                __builtin_ia32_pause();
                #endif
            }
        } else {
            next_frame_time = current_time;
        }
    }

    free(framebuffer);
    SDL_DestroyTexture(frame_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    audio_close();
    SDL_Quit();

    return 0;
}
