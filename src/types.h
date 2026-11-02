#ifndef BOMBERMAN_TYPES_H
#define BOMBERMAN_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// Screen & Timing Constants (NES NTSC)
#define NES_SCREEN_W       256
#define NES_SCREEN_H       240
#define MAX_SCREEN_W       512
#define DEFAULT_SCREEN_W   428
#define NES_FPS            60.0988
#define NES_FRAME_US       16639

#define HUD_HEIGHT         24
#define PLAYFIELD_ROWS     13
#define PLAYFIELD_COLS     31
#define TILE_SIZE          16
#define TILE_HALFSIZE       8

#define MAX_BOMBS          10
#define MAX_FIRES          80
#define MAX_ENEMIES        10
#define MAX_STAGES         50

// Tile Types
#define MAP_EMPTY          0
#define MAP_CONCRETE       1
#define MAP_BRICK          2
#define MAP_BOMB           3
#define MAP_HIDDEN_EXIT    4
#define MAP_HIDDEN_BONUS   5
#define MAP_BONUS          6
#define MAP_EXIT           8

// Enemy Types (1..8) & States (9..11)
#define ENEMY_NONE         0
#define ENEMY_VALCOM       1  // Balloon (Slow, random wander)
#define ENEMY_ONEAL        2  // Onion (Fast, vertical track)
#define ENEMY_DAHL         3  // Barrel (Fast, horizontal track)
#define ENEMY_MINVO        4  // Kolobok (Fastest)
#define ENEMY_DORIA        5  // Cloud (Phases through bricks)
#define ENEMY_OVAPE        6  // Ghost (Phases through bricks)
#define ENEMY_PASS         7  // Boar
#define ENEMY_PONTAN       8  // Coin

// Enemy Life Cycle / Stun States from 6502 ROM
#define ENEMY_STATE_NORMAL 0
#define ENEMY_STATE_STUN   1  // Burning / melting in place
#define ENEMY_STATE_SCORE  2  // Showing score popup ("100", "200", etc.)
#define ENEMY_STATE_DEAD   3

// Directions
#define DIR_NONE           0
#define DIR_RIGHT          1
#define DIR_UP             2
#define DIR_LEFT           3
#define DIR_DOWN           4

// Bomb State
typedef struct {
    bool active;
    int x;
    int y;
    int time_left;     // Starts at 160 frames (~2.66s)
    int time_elapsed;
    int power;
} Bomb;

// Fire Flame Particle State
typedef struct {
    bool active;
    bool is_brick;
    bool is_cap;
    int x;
    int y;
    int dir;
    int timer;
    int anim_frame;
} Fire;

// Enemy Entity State
typedef struct {
    int type;          // Base type (ENEMY_VALCOM..)
    int state;         // ENEMY_STATE_NORMAL, STUN, SCORE
    int x;             // Grid X
    int u;             // Sub-tile X offset (0..15, center=8)
    int y;             // Grid Y
    int v;             // Sub-tile Y offset (0..15, center=8)
    int dir;           // Facing direction
    int frame;         // Animation frame (0..3 for walk, 32..43 for melt)
    int anim_timer;    // Tick accumulator
    int step_counter;  // Steps remaining in current direction
    int stun_timer;    // Frames remaining in stun/melting
    int score_val;     // Score awarded (e.g. 100, 200, 400)
    int score_timer;   // Score popup display timer (20 frames)
} Enemy;

// Player Entity State
typedef struct {
    int x;             // Grid X
    int u;             // Sub-tile X offset (0..15, center=8)
    int y;             // Grid Y
    int v;             // Sub-tile Y offset (0..15, center=8)
    int dir;           // Facing direction
    int frame;         // Walk animation step
    bool moving;
    bool dying;        // Dying / Stun state
    int death_frame;   // 12..20 (melting progression from NES ROM)
    int death_timer;   // 16-frame countdown per death stage
} Player;

// Full Game State
typedef struct {
    uint32_t seed[4];
    uint32_t frame_cnt;

    int stage;
    int score;
    int lives;
    int time_left;
    int second_timer;

    // Power-ups
    int bomb_power;
    int max_bombs;
    bool bonus_speed;
    bool bonus_noclip;
    bool bonus_remote;
    bool bonus_bombwalk;
    bool bonus_firesuit;
    int invulnerable_timer;

    uint8_t map[PLAYFIELD_ROWS][PLAYFIELD_COLS];

    Player player;
    Bomb bombs[MAX_BOMBS];
    Fire fires[MAX_FIRES];
    Enemy enemies[MAX_ENEMIES];

    int screen_w;
    int h_scroll;
    bool game_over;
    bool stage_clear;
    int stage_clear_timer;
} GameState;

#endif // BOMBERMAN_TYPES_H
