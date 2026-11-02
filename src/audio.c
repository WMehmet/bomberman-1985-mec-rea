#include "audio.h"
#include "settings.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define SAMPLE_RATE 44100

typedef struct {
    SoundEffect current_sound;
    int sample_index;
    int total_samples;
    uint16_t noise_lfsr;
    float phase;
} SynthState;

static SynthState g_synth = {0};
static SDL_AudioDeviceID g_audio_dev = 0;

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;
    int16_t *buffer = (int16_t *)stream;
    int samples = len / (int)sizeof(int16_t);

    for (int i = 0; i < samples; i++) {
        int16_t out_sample = 0;

        if (g_synth.current_sound != SND_NONE && g_synth.sample_index < g_synth.total_samples) {
            float progress = (float)g_synth.sample_index / (float)g_synth.total_samples;

            switch (g_synth.current_sound) {
                case SND_STEP: {
                    // Soft low click (triangle wave decaying rapidly over 30ms)
                    float freq = 120.0f;
                    g_synth.phase += freq / (float)SAMPLE_RATE;
                    if (g_synth.phase > 1.0f) g_synth.phase -= 1.0f;
                    float wave = (g_synth.phase < 0.5f) ? (4.0f * g_synth.phase - 1.0f) : (3.0f - 4.0f * g_synth.phase);
                    float env = 1.0f - progress;
                    out_sample = (int16_t)(wave * env * 4000.0f);
                    break;
                }
                case SND_PLANT_BOMB: {
                    // Quick high-to-mid frequency pulse (classic drop blip)
                    float freq = 880.0f - progress * 400.0f;
                    g_synth.phase += freq / (float)SAMPLE_RATE;
                    if (g_synth.phase > 1.0f) g_synth.phase -= 1.0f;
                    float wave = (g_synth.phase < 0.5f) ? 1.0f : -1.0f;
                    float env = 1.0f - progress;
                    out_sample = (int16_t)(wave * env * 6000.0f);
                    break;
                }
                case SND_EXPLOSION: {
                    // Authentic NES 15-bit shift-register noise burst with boom envelope
                    uint16_t b0 = g_synth.noise_lfsr & 1;
                    uint16_t b1 = (g_synth.noise_lfsr >> 1) & 1;
                    g_synth.noise_lfsr = (g_synth.noise_lfsr >> 1) | ((b0 ^ b1) << 14);
                    float noise_val = (b0 ? 1.0f : -1.0f);

                    // Low bass boom under noise
                    float rumble_freq = 65.0f * (1.0f - progress * 0.5f);
                    g_synth.phase += rumble_freq / (float)SAMPLE_RATE;
                    if (g_synth.phase > 1.0f) g_synth.phase -= 1.0f;
                    float rumble = (g_synth.phase < 0.5f) ? 1.0f : -1.0f;

                    float env = (progress < 0.05f) ? (progress / 0.05f) : powf(1.0f - progress, 1.4f);
                    out_sample = (int16_t)((noise_val * 0.7f + rumble * 0.3f) * env * 12000.0f);
                    break;
                }
                case SND_POWERUP: {
                    // Arpeggio chime (C5 -> E5 -> G5 -> C6)
                    float notes[] = { 523.25f, 659.25f, 783.99f, 1046.50f };
                    int note_idx = (int)(progress * 4.0f);
                    if (note_idx > 3) note_idx = 3;
                    float freq = notes[note_idx];
                    g_synth.phase += freq / (float)SAMPLE_RATE;
                    if (g_synth.phase > 1.0f) g_synth.phase -= 1.0f;
                    float wave = (g_synth.phase < 0.5f) ? 1.0f : -1.0f;
                    float env = 1.0f - (progress * 4.0f - (float)note_idx);
                    if (env < 0.0f) env = 0.0f;
                    out_sample = (int16_t)(wave * env * 7000.0f);
                    break;
                }
                case SND_DEATH: {
                    // Descending sorrowful chirp / slide
                    float freq = 600.0f - progress * 450.0f;
                    g_synth.phase += freq / (float)SAMPLE_RATE;
                    if (g_synth.phase > 1.0f) g_synth.phase -= 1.0f;
                    float wave = (g_synth.phase < 0.5f) ? 1.0f : -1.0f;
                    float env = 1.0f - progress;
                    out_sample = (int16_t)(wave * env * 8000.0f);
                    break;
                }
                default:
                    break;
            }

            g_synth.sample_index++;
        } else {
            g_synth.current_sound = SND_NONE;
        }

        buffer[i] = out_sample;
    }
}

void audio_init(void) {
    SDL_AudioSpec wanted, obtained;
    SDL_zero(wanted);
    wanted.freq = SAMPLE_RATE;
    wanted.format = AUDIO_S16SYS;
    wanted.channels = 1;
    wanted.samples = 512; // Low buffer size for ultra-low latency (<12ms)
    wanted.callback = audio_callback;

    g_synth.noise_lfsr = 0x7FFF;
    g_audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted, &obtained, 0);
    if (g_audio_dev > 0) {
        SDL_PauseAudioDevice(g_audio_dev, 0);
    }
}

void audio_play_sound(SoundEffect effect) {
    if (g_audio_dev == 0 || !g_settings.sound_enabled) return;

    SDL_LockAudioDevice(g_audio_dev);
    g_synth.current_sound = effect;
    g_synth.sample_index = 0;
    g_synth.phase = 0.0f;
    g_synth.noise_lfsr = 0x7FFF;

    switch (effect) {
        case SND_STEP:
            g_synth.total_samples = (int)(0.035f * SAMPLE_RATE);
            break;
        case SND_PLANT_BOMB:
            g_synth.total_samples = (int)(0.08f * SAMPLE_RATE);
            break;
        case SND_EXPLOSION:
            g_synth.total_samples = (int)(0.60f * SAMPLE_RATE);
            break;
        case SND_POWERUP:
            g_synth.total_samples = (int)(0.35f * SAMPLE_RATE);
            break;
        case SND_DEATH:
            g_synth.total_samples = (int)(0.90f * SAMPLE_RATE);
            break;
        default:
            g_synth.total_samples = (int)(0.1f * SAMPLE_RATE);
            break;
    }
    SDL_UnlockAudioDevice(g_audio_dev);
}

void audio_close(void) {
    if (g_audio_dev > 0) {
        SDL_CloseAudioDevice(g_audio_dev);
        g_audio_dev = 0;
    }
}
