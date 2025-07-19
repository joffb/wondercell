// Wondercell
// Joe Kennedy - 2023

#include <stdint.h>
#include <string.h>
#include <ws.h>
#include <wonderful.h>
#include <wsx/lzsa.h>

#include "iram.h"
#include "draw.h"
#include "card.h"

#include "graphics/baize.h"
#include "graphics/cards.h"
#include "graphics/text.h"
#include "graphics/title_screen.h"
#include "graphics/you_win.h"

#include "menu_tilemap_bin.h"

uint8_t camera_y;
static uint8_t drawn_cursor_x;
static uint16_t drawn_cursor_y;

#ifdef __WONDERFUL_WWITCH__
#define ws_gdma_copyp memcpy
#endif

// #define FORCE_MONO

void init_video()
{
	hide_screen();

#ifndef FORCE_MONO
	if (ws_system_set_mode(WS_MODE_COLOR_4BPP))
#else
	if (0)
#endif
	{
		// 4bpp planar gfx, set black color
		WS_DISPLAY_COLOR_MEM(0)[0] = 0x000;
	}
	else
	{
		// mono gfx, set black color
		outportw(WS_SCR_PAL_0_PORT, WS_DISPLAY_MONO_PALETTE(0, 0, 0, 0));

		// initialize display LUT
		// colors 7,6,5,4 used by baize
		// colors 7,3,1,0 used by UI
		ws_display_set_shade_lut(WS_DISPLAY_SHADE_LUT(0, 2, 4, 6, 12, 13, 14, 15));
	}

	// set base addresses for screens 1 and 2
	outportb(WS_SCR_BASE_PORT, WS_SCR_BASE_ADDR1(screen_1) | WS_SCR_BASE_ADDR2(screen_2));

	// reset scroll registers to 0
	outportb(WS_SCR1_SCRL_X_PORT, 0);
	outportb(WS_SCR1_SCRL_Y_PORT, 0);
	outportb(WS_SCR2_SCRL_X_PORT, 0);
	outportb(WS_SCR2_SCRL_Y_PORT, 0);

	// set sprite base address
	outportb(WS_SPR_BASE_PORT, WS_SPR_BASE_ADDR(sprites));
	
	// don't render any sprites for now
	outportb(WS_SPR_COUNT_PORT, 0);
}

void hide_screen()
{
	// disable all video output for now
	outportw(WS_DISPLAY_CTRL_PORT, 0);
}

void show_title_screen()
{
	// enable just screen_1 at title screen
	outportw(WS_DISPLAY_CTRL_PORT, WS_DISPLAY_CTRL_SCR1_ENABLE | WS_DISPLAY_CTRL_SCR2_ENABLE);
}

void show_game_screen()
{
	// enable all tile layers and sprites
	outportw(WS_DISPLAY_CTRL_PORT, WS_DISPLAY_CTRL_SCR1_ENABLE | WS_DISPLAY_CTRL_SCR2_ENABLE | WS_DISPLAY_CTRL_SPR_ENABLE);
}

void copy_checkerboard_gfx()
{
	if (ws_system_is_color_active())
		ws_gdma_copyp(WS_TILE_4BPP_MEM(0), gfx_cards_tiles, 0x10 * WS_DISPLAY_TILE_SIZE_4BPP);
	else
		memcpy(WS_TILE_MEM(0), gfx_cards_mono_tiles, 0x10 * WS_DISPLAY_TILE_SIZE);
}

void copy_title_screen_gfx()
{
	if (ws_system_is_color_active())
		wsx_lzsa2_decompress(WS_TILE_4BPP_MEM(16), gfx_title_screen_tiles);
	else
		wsx_lzsa2_decompress(WS_TILE_MEM(16), gfx_title_screen_mono_tiles);
}

void copy_text_gfx()
{
	if (ws_system_is_color_active())
		wsx_lzsa2_decompress(WS_TILE_4BPP_MEM(160), gfx_text_tiles);
	else
		wsx_lzsa2_decompress(WS_TILE_MEM(160), gfx_text_mono_tiles);
}

void copy_card_tile_gfx()
{
	if (ws_system_is_color_active())
		ws_gdma_copyp(WS_TILE_4BPP_MEM(0), gfx_cards_tiles, 160 * WS_DISPLAY_TILE_SIZE_4BPP);
	else
		memcpy(WS_TILE_MEM(0), gfx_cards_mono_tiles, 160 * WS_DISPLAY_TILE_SIZE);
}

void copy_you_win_gfx()
{
	if (ws_system_is_color_active())
		wsx_lzsa2_decompress(WS_TILE_4BPP_MEM(YOU_WIN_TILES), gfx_you_win_tiles);
	else
		wsx_lzsa2_decompress(WS_TILE_MEM(YOU_WIN_TILES), gfx_you_win_mono_tiles);
}

void copy_baize_gfx()
{
	if (ws_system_is_color_active())
		ws_gdma_copyp(WS_TILE_4BPP_MEM(BAIZE_TILES), gfx_baize_tiles, 9 * WS_DISPLAY_TILE_SIZE_4BPP);
	else
		memcpy(WS_TILE_MEM(BAIZE_TILES), gfx_baize_mono_tiles, 9 * WS_DISPLAY_TILE_SIZE);
}


// copy palettes to vram
void copy_palettes()
{
    if (ws_system_is_color_active())
    {
        ws_gdma_copyp(WS_DISPLAY_COLOR_MEM(BAIZE_PALETTE), gfx_baize_palette, 32);
        ws_gdma_copyp(WS_DISPLAY_COLOR_MEM(CARDS_PALETTE), gfx_cards_palette, 32);
        ws_gdma_copyp(WS_DISPLAY_COLOR_MEM(CHECKERBOARD_PALETTE), gfx_cards_palette, 32);
    }
    else
    {
        outportw(WS_SCR_PAL_PORT(BAIZE_PALETTE), WS_DISPLAY_MONO_PALETTE(7, 6, 5, 4));
        outportw(WS_SCR_PAL_PORT(CARDS_PALETTE), WS_DISPLAY_MONO_PALETTE(1, 7, 3, 0));
        outportw(WS_SCR_PAL_PORT(CHECKERBOARD_PALETTE), WS_DISPLAY_MONO_PALETTE(7, 7, 3, 2));
    }
}

void clear_card_layer()
{
    ws_screen_fill_tiles(screen_2, WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE), 0, 0, WS_SCREEN_WIDTH_TILES, WS_SCREEN_HEIGHT_TILES);
}

// draw the checkerboard background onto screen 1 page 2
void draw_checkerboard()
{
    uint16_t index;

	for (index = 0; index < WS_SCREEN_WIDTH_TILES * WS_SCREEN_HEIGHT_TILES; index++)
	{
		screen_1_page_2[index] = (CHECKERBOARD_TILES + (index % 2) + (((index / 32) % 2) * 2)) | WS_SCREEN_ATTR_PALETTE(CHECKERBOARD_PALETTE);
	}
}

// draw the green baize background onto screen 1
void draw_baize()
{
    uint16_t index;

	for (index = 0; index < WS_SCREEN_WIDTH_TILES * WS_SCREEN_HEIGHT_TILES; index++)
	{
		screen_1[index] = (BAIZE_TILES + (index % 3) + (((index / 32) % 3) * 3)) | WS_SCREEN_ATTR_PALETTE(BAIZE_PALETTE);
	}
}

// draw dotted lines for empty freecellls
void draw_empty_freecells()
{
    uint8_t i;

	// draw freecells
	for (i = 0; i < 4; i++)
	{
		draw_empty_card(cursor_area_tx[AREA_FREECELLS] + (i * 3), cursor_area_ty[AREA_FREECELLS]);
	}
}

// draw dotted lines for empty foundations
void draw_empty_foundations()
{
    uint8_t i, tx, ty;
    uint16_t index;

	// draw foundations
	for (i = 0; i < 4; i++)
	{
        tx = cursor_area_tx[AREA_FOUNDATIONS] + (i * 3);
        ty = cursor_area_ty[AREA_FOUNDATIONS];

		draw_empty_card(tx, ty);

		// draw suit icon for each foundations
		index = (tx + 1) + ((ty + 1) << 5);
		screen_2[index] = (0x58 + i) | WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);
	}
}

void draw_title_screen()
{
    ws_screen_put_tiles(screen_2, gfx_title_screen_map, 0, 0, 28, 18);
}

// draw menu into an offscreen page which will be swapped out for screen_2
void draw_menu()
{
    uint16_t i;

	for (i = 0; i < WS_DISPLAY_WIDTH_TILES * WS_DISPLAY_HEIGHT_TILES; i++)
	{
        screen_2_page_2[i] = menu_tilemap[i] | WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);
    }
}

// load "You Win" graphics into sprites
void set_up_you_win_sprites()
{
    uint8_t i;

    // disable screen_2 to hide cards
    outportw(WS_DISPLAY_CTRL_PORT, WS_DISPLAY_CTRL_SCR1_ENABLE | WS_DISPLAY_CTRL_SPR_ENABLE);
    outportb(WS_SPR_FIRST_PORT, 0);
    outportb(WS_SPR_COUNT_PORT, 32);

    // 8x4 tiles image
    for (i = 0; i < 32; i++)
    {
        sprites[i].attr = (YOU_WIN_TILES + i) | WS_SPRITE_ATTR_PALETTE(CARDS_PALETTE) | WS_SPRITE_ATTR_PRIORITY;
        sprites[i].x = (10 + (i % 8)) << 3;
        sprites[i].y = (7 + (i / 8)) << 3;
    }
}

void reset_drawn_cursor()
{
	drawn_cursor_x = (cursor_area_tx[cursor_area] + (cursor_x * 3)) << 3;
	drawn_cursor_y = (cursor_area_ty[cursor_area] + cursor_y) << 3;
}

static uint16_t interpolate_value(uint16_t value, uint16_t target)
{
    if (value == target)
        return target;
    else if (value < target)
        return (value * 3 + target + 3) >> 2;
    else
        return (value * 3 + target) >> 2;
}

void draw_cursor()
{
    uint8_t i;

    // update drawn cursor position
    uint16_t old_drawn_cursor_x = drawn_cursor_x;
    uint16_t old_drawn_cursor_y = drawn_cursor_y;
    reset_drawn_cursor();
    drawn_cursor_x = interpolate_value(old_drawn_cursor_x, drawn_cursor_x);
    drawn_cursor_y = interpolate_value(old_drawn_cursor_y, drawn_cursor_y);

    // number of sprites to render
    outportb(WS_SPR_FIRST_PORT, 0);
    outportb(WS_SPR_COUNT_PORT, 2 + card_in_hand_tiles_count);

    // cursor position
    sprites[0].x = drawn_cursor_x + 20;
    sprites[0].y = drawn_cursor_y + 8 - camera_y;

    sprites[1].x = sprites[0].x;
    sprites[1].y = sprites[0].y + 8;

    // set up sprites for card which is being moved
    for (i = 0; i < card_in_hand_tiles_count; i++)
    {
        sprites[i + 2] = card_in_hand_tiles[i];
        sprites[i + 2].x = (drawn_cursor_x + ((i % 3) << 3)) + 4;
        sprites[i + 2].y = (drawn_cursor_y + ((i / 3) << 3)) + 6 - camera_y;
    }
}

// copy card tiles for the card at the given location
// into an array of sprites which will be used to move the card
// around with the cursor
void copy_card_tiles_to_sprites(uint8_t x, uint8_t y)
{
	uint8_t i;
	uint8_t dest_offset = 0;
	uint16_t source_offset = x + (y << 5);
	card_in_hand_tiles_count = 12;

	for (i = 0; i < 4; i++)
	{
		card_in_hand_tiles[dest_offset].attr = screen_2[source_offset] | WS_SPRITE_ATTR_PRIORITY;
		dest_offset++;
		source_offset++;

		card_in_hand_tiles[dest_offset].attr = screen_2[source_offset] | WS_SPRITE_ATTR_PRIORITY;
		dest_offset++;
		source_offset++;

		card_in_hand_tiles[dest_offset].attr = screen_2[source_offset] | WS_SPRITE_ATTR_PRIORITY;
		dest_offset++;
		source_offset++;

		source_offset += WS_SCREEN_WIDTH_TILES - 3;
	}
}

// remove tiles for the card at x, y
void clear_card_tiles(uint8_t x, uint8_t y)
{
	uint8_t i;
	uint16_t offset = x + (y << 5);

	// body of card
	for (i = 0; i < 4; i++)
	{
		screen_2[offset] = WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);
		screen_2[offset + 1] = WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);
		screen_2[offset + 2] = WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);
		offset += WS_SCREEN_WIDTH_TILES;
	}
}

// draw tiles for the given card at x, y
void draw_card_tiles(uint8_t card, uint8_t x, uint8_t y, uint8_t full_card)
{
	uint8_t i = 0;
	uint8_t value = (card & 0xf);
	uint8_t suit = (card >> 4);
	uint16_t card_body = 0x10;

	uint16_t offset = x + (y * WS_SCREEN_WIDTH_TILES);

	// top row
	screen_2[offset] = (0x54) | WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);
    screen_2[offset + 1] = (0x50 + suit) | WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);
	screen_2[offset + 2] = (0x60 + value) | WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);

	offset += WS_SCREEN_WIDTH_TILES;

    // whether to draw the full card or just the top row
	if (full_card == 1)
	{
		// ace
		if (value == 0)
		{
			card_body = 0x30 + (suit << 3);
		}

		// face cards
		else if (value >= 10)
		{
			card_body = 0x18 + ((value - 10) << 3);
		}

		// body of card
		for (i = 0; i < 2; i++)
		{
			screen_2[offset] = card_body | WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);
			card_body++;

			screen_2[offset + 1] = card_body | WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);
			card_body++;

			screen_2[offset + 2] = card_body | WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);
			card_body++;

			offset += WS_SCREEN_WIDTH_TILES;
		}

		// bottom row
		screen_2[offset] = (0x70 + value) | WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE) | WS_SCREEN_ATTR_FLIP_H | WS_SCREEN_ATTR_FLIP_V;
		screen_2[offset + 1] = (0x50 + suit) | WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE) | WS_SCREEN_ATTR_FLIP_H | WS_SCREEN_ATTR_FLIP_V;
		screen_2[offset + 2] = (0x57) | WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);
	}

}

// draw "empty" dotted line card for freecells and foundations
void draw_empty_card(uint8_t x, uint8_t y)
{
	uint8_t i = 0;
	uint8_t tile = 0x80;
	uint16_t offset = x + (y * WS_SCREEN_WIDTH_TILES);

	for (i = 0; i < 4; i++)
	{
		screen_2[offset] = tile | WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);
		tile++;

		screen_2[offset + 1] = tile | WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);
		tile++;

		screen_2[offset + 2] = tile | WS_SCREEN_ATTR_PALETTE(CARDS_PALETTE);
		tile++;

		offset += WS_SCREEN_WIDTH_TILES;
	}
}
