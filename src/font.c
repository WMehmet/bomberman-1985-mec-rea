#include "font.h"
#include "chr_rom.h"
#include "types.h"
#include <string.h>

#define CHR_ROM _tmp_bomberman_disasm_BOMBER_CHR

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

// Custom 8x8 1-bit bitmaps for Turkish letters & common punctuation
// Automatic drop shadow (1px down, 1px right) will be rendered to match NES style.
static const uint8_t GLYPH_C_CEDILLA[8] = {
    0b00111100,
    0b01100110,
    0b11000000,
    0b11000000,
    0b01100110,
    0b00111100,
    0b00011000,
    0b00010000
};

static const uint8_t GLYPH_G_BREVE[8] = {
    0b01011010,
    0b00000000,
    0b00111100,
    0b01100000,
    0b01101110,
    0b01100110,
    0b00111110,
    0b00000000
};

static const uint8_t GLYPH_I_DOTTED[8] = {
    0b00011000,
    0b00000000,
    0b00111100,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00111100,
    0b00000000
};

static const uint8_t GLYPH_O_UMLAUT[8] = {
    0b01100110,
    0b00000000,
    0b00111100,
    0b01100110,
    0b01100110,
    0b01100110,
    0b00111100,
    0b00000000
};

static const uint8_t GLYPH_S_CEDILLA[8] = {
    0b00111100,
    0b01100000,
    0b00111100,
    0b00000110,
    0b01100110,
    0b00111100,
    0b00011000,
    0b00010000
};

static const uint8_t GLYPH_U_UMLAUT[8] = {
    0b01100110,
    0b00000000,
    0b01100110,
    0b01100110,
    0b01100110,
    0b01100110,
    0b00111100,
    0b00000000
};

static const uint8_t GLYPH_AMPERSAND[8] = {
    0b00110000,
    0b01001000,
    0b00110000,
    0b01001100,
    0b10010010,
    0b01101100,
    0b00000000,
    0b00000000
};

static const uint8_t GLYPH_DOT[8] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00110000,
    0b00110000,
    0b00000000
};

static const uint8_t GLYPH_COLON[8] = {
    0b00000000,
    0b00110000,
    0b00110000,
    0b00000000,
    0b00110000,
    0b00110000,
    0b00000000,
    0b00000000
};

static const uint8_t GLYPH_DASH[8] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b01111110,
    0b01111110,
    0b00000000,
    0b00000000,
    0b00000000
};

static const uint8_t GLYPH_SLASH[8] = {
    0b00000110,
    0b00001100,
    0b00011000,
    0b00110000,
    0b01100000,
    0b11000000,
    0b00000000,
    0b00000000
};

static const uint8_t GLYPH_EXCLAMATION[8] = {
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00000000,
    0b00011000,
    0b00011000,
    0b00000000
};

static void draw_custom_glyph(uint32_t *fb, int sw, int x, int y, const uint8_t bmp[8], const uint8_t *palette) {
    for (int r = 0; r < 8; r++) {
        int py = y + r;
        if (py < 0 || py >= NES_SCREEN_H) continue;
        for (int c = 0; c < 8; c++) {
            int px = x + c;
            if (px < 0 || px >= sw) continue;

            int fg = (bmp[r] >> (7 - c)) & 1;
            int sh = 0;
            if (r > 0 && c > 0) {
                sh = (bmp[r - 1] >> (7 - (c - 1))) & 1;
            }

            if (fg) {
                fb[py * sw + px] = NES_PALETTE[palette[3]];
            } else if (sh) {
                fb[py * sw + px] = NES_PALETTE[palette[1]];
            }
        }
    }
}

static void draw_chr_tile(uint32_t *fb, int sw, int x, int y, int tile_idx, const uint8_t *palette) {
    if (tile_idx < 0 || tile_idx >= 512) return;
    const uint8_t *tile_data = &CHR_ROM[tile_idx * 16];

    for (int r = 0; r < 8; r++) {
        int py = y + r;
        if (py < 0 || py >= NES_SCREEN_H) continue;
        uint8_t p0 = tile_data[r];
        uint8_t p1 = tile_data[r + 8];

        for (int c = 0; c < 8; c++) {
            int px = x + c;
            if (px < 0 || px >= sw) continue;

            uint8_t ci = (((p1 >> (7 - c)) & 1) << 1) | ((p0 >> (7 - c)) & 1);
            if (ci == 0) continue; // transparent background

            fb[py * sw + px] = NES_PALETTE[palette[ci]];
        }
    }
}

void font_draw_char(uint32_t *fb, int sw, int x, int y, uint32_t c, const uint8_t *palette) {
    // Digits
    if (c >= '0' && c <= '9') {
        draw_chr_tile(fb, sw, x, y, 256 + 0x30 + (c - '0'), palette);
        return;
    }

    // Letters (A-Z)
    if (c >= 'A' && c <= 'Z') {
        draw_chr_tile(fb, sw, x, y, 256 + 0x41 + (c - 'A'), palette);
        return;
    }

    // Lowercase ASCII (a-z) -> Map to uppercase
    if (c >= 'a' && c <= 'z') {
        draw_chr_tile(fb, sw, x, y, 256 + 0x41 + (c - 'a'), palette);
        return;
    }

    // NES Arrow Cursor
    if (c == '>') {
        draw_chr_tile(fb, sw, x, y, 256 + 0x40, palette);
        return;
    }

    // Turkish & Symbols
    switch (c) {
        // Turkish Letters (Uppercase & Lowercase)
        case 0x00C7: case 0x00E7: // Ç, ç
            draw_custom_glyph(fb, sw, x, y, GLYPH_C_CEDILLA, palette);
            break;
        case 0x011E: case 0x011F: // Ğ, ğ
            draw_custom_glyph(fb, sw, x, y, GLYPH_G_BREVE, palette);
            break;
        case 0x0130: case 'i':    // İ, i (dotted capital I)
            draw_custom_glyph(fb, sw, x, y, GLYPH_I_DOTTED, palette);
            break;
        case 0x0131: case 'I':    // ı, I (dotless I -> standard 'I')
            draw_chr_tile(fb, sw, x, y, 256 + 0x41 + ('I' - 'A'), palette);
            break;
        case 0x00D6: case 0x00F6: // Ö, ö
            draw_custom_glyph(fb, sw, x, y, GLYPH_O_UMLAUT, palette);
            break;
        case 0x015E: case 0x015F: // Ş, ş
            draw_custom_glyph(fb, sw, x, y, GLYPH_S_CEDILLA, palette);
            break;
        case 0x00DC: case 0x00FC: // Ü, ü
            draw_custom_glyph(fb, sw, x, y, GLYPH_U_UMLAUT, palette);
            break;

        // Punctuation & Symbols
        case '&':
            draw_custom_glyph(fb, sw, x, y, GLYPH_AMPERSAND, palette);
            break;
        case '.':
            draw_custom_glyph(fb, sw, x, y, GLYPH_DOT, palette);
            break;
        case ':':
            draw_custom_glyph(fb, sw, x, y, GLYPH_COLON, palette);
            break;
        case '-':
            draw_custom_glyph(fb, sw, x, y, GLYPH_DASH, palette);
            break;
        case '/':
            draw_custom_glyph(fb, sw, x, y, GLYPH_SLASH, palette);
            break;
        case '!':
            draw_custom_glyph(fb, sw, x, y, GLYPH_EXCLAMATION, palette);
            break;
        case ' ':
        default:
            // Blank space
            break;
    }
}

static uint32_t decode_utf8(const char **p) {
    const unsigned char *s = (const unsigned char *)*p;
    if (!*s) return 0;
    if (*s < 0x80) {
        (*p)++;
        return *s;
    } else if ((*s & 0xE0) == 0xC0) {
        uint32_t c = ((*s & 0x1F) << 6) | (s[1] & 0x3F);
        *p += 2;
        return c;
    } else if ((*s & 0xF0) == 0xE0) {
        uint32_t c = ((*s & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        *p += 3;
        return c;
    }
    (*p)++;
    return *s;
}

void font_draw_string(uint32_t *fb, int sw, int x, int y, const char *str, const uint8_t *palette) {
    if (!str) return;
    const char *p = str;
    while (*p) {
        uint32_t cp = decode_utf8(&p);
        if (!cp) break;
        font_draw_char(fb, sw, x, y, cp, palette);
        x += 8;
    }
}

int font_string_width(const char *str) {
    if (!str) return 0;
    int count = 0;
    const char *p = str;
    while (*p) {
        uint32_t cp = decode_utf8(&p);
        if (!cp) break;
        count++;
    }
    return count * 8;
}
