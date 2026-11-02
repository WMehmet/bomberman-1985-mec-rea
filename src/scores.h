#ifndef BOMBERMAN_SCORES_H
#define BOMBERMAN_SCORES_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_SCORE_ENTRIES 8

typedef struct {
    int score;
    int stage;
    char date[16]; // e.g. "31.10.2026"
} ScoreEntry;

void scores_init(void);
void scores_add(int score, int stage);
int scores_get_count(void);
const ScoreEntry *scores_get(int index);
void scores_draw_screen(uint32_t *fb, int sw, int frame_cnt);

#endif // BOMBERMAN_SCORES_H
