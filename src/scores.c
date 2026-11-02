#include "scores.h"
#include "font.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

static ScoreEntry s_scores[MAX_SCORE_ENTRIES];
static int s_score_count = 0;
static char s_save_path[512] = "";

static void resolve_save_path(void) {
    if (s_save_path[0] != '\0') return;

    // First attempt: current working directory
    const char *local_name = "bomberman_scores.dat";
    FILE *f = fopen(local_name, "a+");
    if (f) {
        fclose(f);
        strncpy(s_save_path, local_name, sizeof(s_save_path) - 1);
        return;
    }

#ifdef _WIN32
    const char *appdata = getenv("APPDATA");
    if (appdata) {
        snprintf(s_save_path, sizeof(s_save_path), "%s\\bomberman_scores.dat", appdata);
        return;
    }
#else
    const char *home = getenv("HOME");
    if (home) {
        snprintf(s_save_path, sizeof(s_save_path), "%s/.bomberman_scores.dat", home);
        return;
    }
#endif

    strncpy(s_save_path, "bomberman_scores.dat", sizeof(s_save_path) - 1);
}

static void save_scores_to_disk(void) {
    resolve_save_path();
    FILE *f = fopen(s_save_path, "w");
    if (!f) return;

    for (int i = 0; i < s_score_count; i++) {
        fprintf(f, "%d %d %s\n", s_scores[i].score, s_scores[i].stage, s_scores[i].date);
    }
    fclose(f);
}

static void sort_scores(void) {
    for (int i = 0; i < s_score_count - 1; i++) {
        for (int j = i + 1; j < s_score_count; j++) {
            if (s_scores[j].score > s_scores[i].score) {
                ScoreEntry tmp = s_scores[i];
                s_scores[i] = s_scores[j];
                s_scores[j] = tmp;
            }
        }
    }
}

void scores_init(void) {
    resolve_save_path();
    s_score_count = 0;

    FILE *f = fopen(s_save_path, "r");
    if (f) {
        while (s_score_count < MAX_SCORE_ENTRIES) {
            int sc = 0, st = 0;
            char dt[16] = "";
            if (fscanf(f, "%d %d %15s", &sc, &st, dt) == 3) {
                s_scores[s_score_count].score = sc;
                s_scores[s_score_count].stage = st;
                snprintf(s_scores[s_score_count].date, sizeof(s_scores[s_score_count].date), "%s", dt);
                s_score_count++;
            } else {
                break;
            }
        }
        fclose(f);
    }

    // Default seeded scores if no existing records found
    if (s_score_count == 0) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char date_buf[16] = "31.10.2026";
        if (tm_info) {
            strftime(date_buf, sizeof(date_buf), "%d.%m.%Y", tm_info);
        }

        s_scores[0] = (ScoreEntry){ .score = 50000, .stage = 5, .date = "" };
        snprintf(s_scores[0].date, sizeof(s_scores[0].date), "%s", date_buf);

        s_scores[1] = (ScoreEntry){ .score = 30000, .stage = 3, .date = "" };
        snprintf(s_scores[1].date, sizeof(s_scores[1].date), "%s", date_buf);

        s_scores[2] = (ScoreEntry){ .score = 10000, .stage = 1, .date = "" };
        snprintf(s_scores[2].date, sizeof(s_scores[2].date), "%s", date_buf);

        s_score_count = 3;
        save_scores_to_disk();
    } else {
        sort_scores();
    }
}

void scores_add(int score, int stage) {
    if (score <= 0) return;

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char date_buf[16] = "31.10.2026";
    if (tm_info) {
        strftime(date_buf, sizeof(date_buf), "%d.%m.%Y", tm_info);
    }

    if (s_score_count < MAX_SCORE_ENTRIES) {
        s_scores[s_score_count].score = score;
        s_scores[s_score_count].stage = stage;
        snprintf(s_scores[s_score_count].date, sizeof(s_scores[s_score_count].date), "%s", date_buf);
        s_score_count++;
    } else {
        // If higher than lowest score, replace it
        if (score > s_scores[s_score_count - 1].score) {
            s_scores[s_score_count - 1].score = score;
            s_scores[s_score_count - 1].stage = stage;
            snprintf(s_scores[s_score_count - 1].date, sizeof(s_scores[s_score_count - 1].date), "%s", date_buf);
        } else {
            return;
        }
    }

    sort_scores();
    save_scores_to_disk();
}

int scores_get_count(void) {
    return s_score_count;
}

const ScoreEntry *scores_get(int index) {
    if (index >= 0 && index < s_score_count) {
        return &s_scores[index];
    }
    return NULL;
}

void scores_draw_screen(uint32_t *fb, int sw, int frame_cnt) {
    // 1. Darken background
    for (int y = 0; y < NES_SCREEN_H; y++) {
        for (int x = 0; x < sw; x++) {
            fb[y * sw + x] = 0xFF000000;
        }
    }

    // 2. Decorative frame
    int box_w = (sw > 300) ? 280 : 246;
    int bx0 = (sw - box_w) / 2, by0 = 14;
    int bx1 = bx0 + box_w, by1 = NES_SCREEN_H - 14;

    for (int y = by0; y <= by1; y++) {
        for (int x = bx0; x <= bx1; x++) {
            if (x == bx0 || x == bx1 || y == by0 || y == by1) {
                fb[y * sw + x] = 0xFFFCA044; // Gold border
            } else if (x == bx0 + 1 || x == bx1 - 1 || y == by0 + 1 || y == by1 - 1) {
                fb[y * sw + x] = 0xFF7C7C7C; // Inner shadow
            }
        }
    }

    static const uint8_t PAL_GOLD[4] = { 0x0F, 0x16, 0x0F, 0x28 };
    static const uint8_t PAL_TEXT[4] = { 0x0F, 0x10, 0x0F, 0x30 };
    static const uint8_t PAL_HIGHLIGHT[4] = { 0x0F, 0x16, 0x0F, 0x36 };

    // Title
    const char *title = "SKOR TABLOSU";
    font_draw_string(fb, sw, (sw - font_string_width(title)) / 2, by0 + 12, title, PAL_GOLD);

    // Table Header
    const char *header = "SIRA   TARİH       SEVİYE    PUAN";
    int header_x = (sw - font_string_width(header)) / 2;
    font_draw_string(fb, sw, header_x, by0 + 32, header, PAL_HIGHLIGHT);

    // Divider line
    for (int x = header_x; x < header_x + font_string_width(header); x++) {
        fb[(by0 + 42) * sw + x] = 0xFF7C7C7C;
    }

    // List entries
    int start_y = by0 + 48;
    int row_h = 14;

    char row_buf[64];
    for (int i = 0; i < s_score_count; i++) {
        int y = start_y + i * row_h;
        snprintf(row_buf, sizeof(row_buf), " %d.   %-10s   SEV %02d   %07d",
                 i + 1, s_scores[i].date, s_scores[i].stage, s_scores[i].score);

        const uint8_t *pal = (i == 0) ? PAL_GOLD : PAL_TEXT;
        font_draw_string(fb, sw, header_x, y, row_buf, pal);
    }

    // Bottom prompt
    if ((frame_cnt / 30) % 2 == 0) {
        const char *prompt = "ESC VEYA ENTER: GERİ";
        font_draw_string(fb, sw, (sw - font_string_width(prompt)) / 2, by1 - 16, prompt, PAL_TEXT);
    }
}
