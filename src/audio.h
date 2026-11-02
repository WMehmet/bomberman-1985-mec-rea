#ifndef BOMBERMAN_AUDIO_H
#define BOMBERMAN_AUDIO_H

#include <stdbool.h>

typedef enum {
    SND_NONE = 0,
    SND_STEP,
    SND_PLANT_BOMB,
    SND_EXPLOSION,
    SND_POWERUP,
    SND_DEATH,
    SND_STAGE_START,
    SND_STAGE_CLEAR
} SoundEffect;

void audio_init(void);
void audio_play_sound(SoundEffect effect);
void audio_close(void);

#endif // BOMBERMAN_AUDIO_H
