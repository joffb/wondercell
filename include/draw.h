// Wondercell
// Joe Kennedy - 2023

#pragma once
#include <wonderful.h>

#define BAIZE_PALETTE 1
#define CHECKERBOARD_PALETTE 2
#define CARDS_PALETTE 12

#define YOU_WIN_TILES 0xE0
#define CURSOR_TILES 0x5
#define BAIZE_TILES 0x7
#define CHECKERBOARD_TILES 0x1

extern uint8_t camera_y;

void init_video();

void hide_screen();
void show_title_screen();
void show_game_screen();

void copy_title_screen_gfx();
void copy_checkerboard_gfx();
void copy_card_tile_gfx();
void copy_you_win_gfx();
void copy_text_gfx();
void copy_baize_gfx();
void copy_palettes();

void clear_card_layer();
void draw_checkerboard();
void draw_baize();
void draw_empty_freecells();
void draw_empty_foundations();

void draw_title_screen();
void draw_menu();

void set_up_you_win_sprites();

void reset_drawn_cursor();
void draw_cursor();
void copy_card_tiles_to_sprites(uint8_t x, uint8_t y);
void clear_card_tiles(uint8_t x, uint8_t y);
void draw_card_tiles(uint8_t card, uint8_t x, uint8_t y, uint8_t full_card);
void draw_empty_card(uint8_t x, uint8_t y);
