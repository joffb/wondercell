// Wondercell
// Joe Kennedy - 2023

#include <stdint.h>
#include <ws.h>
#include <wonderful.h>
#include <wsx/lzsa.h>

#include "draw.h"
#include "card.h"

#include "cards_tiles_bin.h"
#include "cards_palette_bin.h"

#include "baize_tiles_bin.h"
#include "baize_palette_bin.h"

#include "text_tiles_lzsa2_bin.h"

#include "you_win_tiles_lzsa2_bin.h"

#include "title_screen_map_bin.h"
#include "title_screen_tiles_lzsa2_bin.h"

#include "menu_tilemap_bin.h"

void init_video()
{
	hide_screen();

	// 4bpp planar gfx, set black color
	ws_mode_set(WS_MODE_COLOR_4BPP);
	MEM_COLOR_PALETTE(0)[0] = 0x000;

	// set base addresses for screens 1 and 2
	outportb(IO_SCR_BASE, SCR1_BASE(SCREEN_1) | SCR2_BASE(SCREEN_2));

	// reset scroll registers to 0
	outportb(IO_SCR1_SCRL_X, 0);
	outportb(IO_SCR1_SCRL_Y, 0);
	outportb(IO_SCR2_SCRL_X, 0);
	outportb(IO_SCR2_SCRL_Y, 0);

	// set sprite base address
	outportb(IO_SPR_BASE, SPR_BASE(SPRITES));
	
	// don't render any sprites for now
	outportb(IO_SPR_COUNT, 0);
}

void hide_screen()
{
	// disable all video output for now
	outportw(IO_DISPLAY_CTRL, 0);
}

void show_title_screen()
{
	// enable just SCREEN_1 at title screen
	outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE | DISPLAY_SCR2_ENABLE);
}

void show_game_screen()
{
	// enable all tile layers and sprites
	outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE | DISPLAY_SCR2_ENABLE | DISPLAY_SPR_ENABLE);
}

void copy_checkerboard_gfx()
{
    ws_dma_copy_words(MEM_TILE_4BPP(0), cards_tiles, 0x10 * TILE_4BPP_LENGTH);
}

void copy_title_screen_gfx()
{
    wsx_lzsa2_decompress(MEM_TILE_4BPP(16), title_screen_tiles_lzsa2);
}

void copy_text_gfx()
{
    wsx_lzsa2_decompress(MEM_TILE_4BPP(160), text_tiles_lzsa2);
}

void copy_card_tile_gfx()
{
    ws_dma_copy_words(MEM_TILE_4BPP(0), cards_tiles, 160 * TILE_4BPP_LENGTH);
}

void copy_you_win_gfx()
{
    wsx_lzsa2_decompress(MEM_TILE_4BPP(YOU_WIN_TILES), you_win_tiles_lzsa2);
}

void copy_baize_gfx()
{
    ws_dma_copy_words(MEM_TILE_4BPP(BAIZE_TILES), baize_tiles, 9 * TILE_4BPP_LENGTH);
}


// copy palettes to vram
void copy_palettes()
{	
    ws_dma_copy_words(MEM_COLOR_PALETTE(0) + 1, cards_palette + 2, 30);
    ws_dma_copy_words(MEM_COLOR_PALETTE(1), baize_palette, 32);
    ws_dma_copy_words(MEM_COLOR_PALETTE(4), cards_palette, 32);
    ws_dma_copy_words(MEM_COLOR_PALETTE(8), cards_palette, 32);
}

void clear_card_layer()
{
    ws_screen_fill_tiles(SCREEN_2, 0, 0, 0, SCR_WIDTH, SCR_HEIGHT);
}

// draw the checkerboard background onto screen 1 page 2
void draw_checkerboard()
{
    uint16_t index;

	for (index = 0; index < SCR_WIDTH * SCR_HEIGHT; index++)
	{
		SCREEN_1_PAGE_2[index].tile = CHECKERBOARD_TILES + (index % 2) + (((index / 32) % 2) * 2);
        SCREEN_1_PAGE_2[index].palette = 0;
        SCREEN_1_PAGE_2[index].bank = 0;
        SCREEN_1_PAGE_2[index].flip_h = 0;
        SCREEN_1_PAGE_2[index].flip_v = 0;
	}
}

// draw the green baize background onto screen 1
void draw_baize()
{
    uint16_t index;

	for (index = 0; index < SCR_WIDTH * SCR_HEIGHT; index++)
	{
		SCREEN_1[index].tile = BAIZE_TILES + (index % 3) + (((index / 32) % 3) * 3);
        SCREEN_1[index].palette = 1;
        SCREEN_1[index].bank = 0;
        SCREEN_1[index].flip_h = 0;
        SCREEN_1[index].flip_v = 0;
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
		SCREEN_2[index].tile = 0x58 + i;
	}
}

void draw_title_screen()
{
    ws_screen_put_tiles(SCREEN_2, title_screen_map, 0, 0, 28, 18);
}

// draw menu into an offscreen page which will be swapped out for SCREEN_2
void draw_menu()
{
    uint16_t i;

	for (i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
	{
        SCREEN_2_PAGE_2[i].tile = menu_tilemap[i];
    }
}

// load "You Win" graphics into sprites
void set_up_you_win_sprites()
{
    uint8_t i;

    // disable SCREEN_2 to hide cards
    outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE | DISPLAY_SPR_ENABLE);
    outportb(IO_SPR_FIRST, 0);
    outportb(IO_SPR_COUNT, 32);

    // 8x4 tiles image
    for (i = 0; i < 32; i++)
    {
        SPRITES[i].tile = YOU_WIN_TILES + i;
        SPRITES[i].x = (10 + (i % 8)) << 3;
        SPRITES[i].y = (7 + (i / 8)) << 3;
        SPRITES[i].priority = 1;
        SPRITES[i].flip_v = 0;
        SPRITES[i].flip_h = 0;
    }
}

void draw_cursor()
{
    uint8_t i;

    // number of sprites to render
    outportb(IO_SPR_FIRST, 0);
    outportb(IO_SPR_COUNT, 2 + card_in_hand_tiles_count);

    // cursor position
    SPRITES[0].x = (cursor_area_tx[cursor_area] + (cursor_x * 3)) << 3;
    SPRITES[0].x += 20;
    SPRITES[0].y = (cursor_area_ty[cursor_area] + cursor_y) << 3;
    SPRITES[0].y += 8;
    SPRITES[0].y -= camera_y;

    SPRITES[1].x = SPRITES[0].x;
    SPRITES[1].y = SPRITES[0].y + 8;

    uint8_t tx = cursor_area_tx[cursor_area] + (cursor_x * 3);
    uint8_t ty = cursor_area_ty[cursor_area] + (cursor_y);

    // set up sprites for card which is being moved
    for (i = 0; i < card_in_hand_tiles_count; i++)
    {
        SPRITES[i + 2] = card_in_hand_tiles[i];
        SPRITES[i + 2].x = ((tx + (i % 3)) << 3) + 4;
        SPRITES[i + 2].y = ((ty + (i / 3)) << 3) + 6 - camera_y;
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
		card_in_hand_tiles[dest_offset].tile = SCREEN_2[source_offset].tile;
        card_in_hand_tiles[dest_offset].flip_h = SCREEN_2[source_offset].flip_h;
        card_in_hand_tiles[dest_offset].flip_v = SCREEN_2[source_offset].flip_v;
        card_in_hand_tiles[dest_offset].priority = 1;
		dest_offset++;
		source_offset++;

		card_in_hand_tiles[dest_offset].tile = SCREEN_2[source_offset].tile;
        card_in_hand_tiles[dest_offset].flip_h = SCREEN_2[source_offset].flip_h;
        card_in_hand_tiles[dest_offset].flip_v = SCREEN_2[source_offset].flip_v;
        card_in_hand_tiles[dest_offset].priority = 1;
		dest_offset++;
		source_offset++;

		card_in_hand_tiles[dest_offset].tile = SCREEN_2[source_offset].tile;
        card_in_hand_tiles[dest_offset].flip_h = SCREEN_2[source_offset].flip_h;
        card_in_hand_tiles[dest_offset].flip_v = SCREEN_2[source_offset].flip_v;
        card_in_hand_tiles[dest_offset].priority = 1;
		dest_offset++;
		source_offset++;

		source_offset += SCR_WIDTH - 3;
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
		SCREEN_2[offset].tile = 0;
		SCREEN_2[offset + 1].tile = 0;
		SCREEN_2[offset + 2].tile = 0;
		offset += SCR_WIDTH;
	}
}

// draw tiles for the given card at x, y
void draw_card_tiles(uint8_t card, uint8_t x, uint8_t y, uint8_t full_card)
{
	uint8_t i = 0;
	uint8_t value = (card & 0xf);
	uint8_t suit = (card >> 4);
	uint16_t card_body = 0x10;

	uint16_t offset = x + (y * SCR_WIDTH);

	// top row
	SCREEN_2[offset].tile = 0x54;
    SCREEN_2[offset].flip_h = 0;
    SCREEN_2[offset].flip_v = 0;
    SCREEN_2[offset].palette = 4;

    SCREEN_2[offset + 1].tile = 0x50 + suit;
    SCREEN_2[offset + 1].flip_h = 0;
    SCREEN_2[offset + 1].flip_v = 0;
    SCREEN_2[offset + 1].palette = 4;

	SCREEN_2[offset + 2].tile = 0x60 + value;
    SCREEN_2[offset + 2].flip_h = 0;
    SCREEN_2[offset + 2].flip_v = 0;
    SCREEN_2[offset + 2].palette = 4;

	offset += SCR_WIDTH;

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
			SCREEN_2[offset].tile = card_body;
            SCREEN_2[offset].flip_h = 0;
            SCREEN_2[offset].flip_v = 0;
            SCREEN_2[offset].palette = 4;
			card_body++;

			SCREEN_2[offset + 1].tile = card_body;
            SCREEN_2[offset + 1].flip_h = 0;
            SCREEN_2[offset + 1].flip_v = 0;
            SCREEN_2[offset + 1].palette = 4;
			card_body++;

			SCREEN_2[offset + 2].tile = card_body;
            SCREEN_2[offset + 2].flip_h = 0;
            SCREEN_2[offset + 2].flip_v = 0;
            SCREEN_2[offset + 2].palette = 4;
			card_body++;

			offset += SCR_WIDTH;
		}

		// bottom row
		SCREEN_2[offset].tile = 0x70 + value;
        SCREEN_2[offset].flip_h = 1;
        SCREEN_2[offset].flip_v = 1;
        SCREEN_2[offset].palette = 4;

		SCREEN_2[offset + 1].tile = 0x50 + suit;
        SCREEN_2[offset + 1].flip_h = 1;
        SCREEN_2[offset + 1].flip_v = 1;
        SCREEN_2[offset + 1].palette = 4;

		SCREEN_2[offset + 2].tile = 0x57;
        SCREEN_2[offset + 2].flip_h = 0;
        SCREEN_2[offset + 2].flip_v = 0;
        SCREEN_2[offset + 2].palette = 4;
	}

}

// draw "empty" dotted line card for freecells and foundations
void draw_empty_card(uint8_t x, uint8_t y)
{
	uint8_t i = 0;
	uint8_t tile = 0x80;
	uint16_t offset = x + (y * SCR_WIDTH);

	for (i = 0; i < 4; i++)
	{
		SCREEN_2[offset].tile = tile;
        SCREEN_2[offset].flip_h = 0;
        SCREEN_2[offset].flip_v = 0;
		tile++;

		SCREEN_2[offset + 1].tile = tile;
        SCREEN_2[offset + 1].flip_h = 0;
        SCREEN_2[offset + 1].flip_v = 0;
		tile++;

		SCREEN_2[offset + 2].tile = tile;
        SCREEN_2[offset + 2].flip_h = 0;
        SCREEN_2[offset + 2].flip_v = 0;
		tile++;

		offset += SCR_WIDTH;
	}
}
