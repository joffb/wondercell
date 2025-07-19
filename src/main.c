// Wondercell
// Joe Kennedy - 2023

#include <wonderful.h>
#include <ws.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __WONDERFUL_WWITCH__
#include <sys/bios.h>
#endif

#include "card.h"
#include "draw.h"
#include "main.h"
#include "vgm.h"
#include "entertainer_cvgm_bin.h"
#include "title_screen_cvgm_bin.h"
#include "you_win_cvgm_bin.h"

#define IRAM_IMPLEMENTATION
#include "iram.h"

extern void vblank_int_handler(void);

enum game_states {
  GAME_DEALING = 0,
  GAME_INGAME,
  GAME_MENU,
  GAME_TITLE,
  GAME_WON
};

uint8_t tics;

uint16_t rnd_val;

uint16_t keypad;
uint16_t keypad_pushed;
uint16_t keypad_last;

uint8_t game_state;
uint16_t game_seed;

uint8_t menu_cursor;

uint8_t deal_x, deal_y;
uint8_t checker_scroll_x, checker_scroll_y;

vgmswan_state_t music_state;
uint16_t music_ticks;

const uint8_t __far * current_cvgm;

void disable_interrupts()
{
#ifndef __WONDERFUL_WWITCH__
	// disable cpu interrupts
	ia16_disable_irq();

	// disable wonderswan hardware interrupts
	ws_int_disable_all();
#endif
}

void enable_interrupts()
{
#ifndef __WONDERFUL_WWITCH__
	// acknowledge interrupt
	outportb(WS_INT_ACK_PORT, 0xFF);

	// set interrupt handler which only acknowledges the vblank interrupt
	ws_int_set_default_handler_vblank();

	// enable wonderswan vblank interrupt
	ws_int_enable(WS_INT_ENABLE_VBLANK);

	// enable cpu interrupts
	ia16_enable_irq();
#endif
}

void new_game()
{
	// keep the random seed which this game uses around
	// for the restart game function
	game_seed = rnd_val;
	srand(rnd_val);

	// clear card tiles and redraw backgrounds
	clear_card_layer();
	draw_baize();
	draw_empty_freecells();
	draw_empty_foundations();

	// clear cascade/freecell/foundation arrays
	initialise_cascades();
	initialise_freecells();
	initialise_foundations();

	// initialise deck of cards and shuffle it
	initialise_cards_array();
	initialise_deck();
	shuffle_deck();

	outportb(WS_SCR_BASE_PORT, WS_SCR_BASE_ADDR1(screen_1) | WS_SCR_BASE_ADDR2(screen_2));

	// default cursor to first cascade
	cursor_area = AREA_CASCADES;
	cursor_x = 0;
	reset_drawn_cursor();

	// setup cursor sprites
	sprites[0].attr = (CURSOR_TILES) | WS_SPRITE_ATTR_PRIORITY | WS_SPRITE_ATTR_PALETTE(CARDS_PALETTE);
	sprites[1].attr = (CURSOR_TILES + 1) | WS_SPRITE_ATTR_PRIORITY | WS_SPRITE_ATTR_PALETTE(CARDS_PALETTE);

	show_game_screen();

	// do dealing out the cards animation to start with
	deal_x = deal_y = 0;
	game_state = GAME_DEALING;

	// no cards in hand
	card_in_hand = NO_CARD;
	card_in_hand_tiles_count = 0;
}


void wait_for_vblank()
{
#ifdef __WONDERFUL_WWITCH__
	sys_wait(1);
#else
	// halt cpu
	// the program will sit here until the vblank interrupt
	// is triggered and unhalts it
	ia16_halt();
#endif

	// play music
	if (music_ticks == VGMSWAN_PLAYBACK_FINISHED)
	{
		// initialize music
		vgmswan_init(&music_state, current_cvgm);
		music_ticks = 0;
	}

	if (music_ticks > 1)
		music_ticks--;
	else
		music_ticks = vgmswan_play(&music_state);
}

void main()
{
	// disable interrupts for now
	disable_interrupts();

	// initial random seed
	rnd_val = 0;

	// current and last keypad status
	keypad = 0;
	keypad_last = 0;
	
	// checkerboard scrolling
	tics = 0;
	checker_scroll_x = checker_scroll_y = 0;
	camera_y = 0;

	// setup video
	init_video();
	copy_palettes();

	// copy graphics for title screen
	// and copy the tilemap
	copy_title_screen_gfx();
	copy_checkerboard_gfx();
	draw_title_screen();
	draw_checkerboard();

	// setup music driver
	music_ticks = VGMSWAN_PLAYBACK_FINISHED;
#ifndef __WONDERFUL_WWITCH__
	outportb(WS_SOUND_WAVE_BASE_PORT, WS_SOUND_WAVE_BASE_ADDR(&wave_ram));
#endif

	// initial game state
	game_state = GAME_TITLE;

	// show title screen
	outportb(WS_SCR_BASE_PORT, WS_SCR_BASE_ADDR1(screen_1_page_2) | WS_SCR_BASE_ADDR2(screen_2));
	show_title_screen();

	// initial background music
	current_cvgm = title_screen_cvgm;

	// reenable interrupts
	enable_interrupts();

	// main loop
	while (1)
	{
		wait_for_vblank();

		// get keypad state and from the last keypad state
		// determine if a key has been pressed this frame 
		// which wasn't pressed last frame
#ifdef __WONDERFUL_WWITCH__
		keypad_pushed = key_hit_check();
		keypad = key_press_check();
#else
		keypad = ws_keypad_scan();
		keypad_pushed = ((keypad ^ keypad_last) & keypad);
#endif

		// increment the random number seed every frame
		rnd_val++;

		// title screen
		if (game_state == GAME_TITLE)
		{
			// hide sprites
			outportb(WS_SPR_COUNT_PORT, 0);

			// update checkerboard scrolling
			if (tics % 4 == 0)
			{
				checker_scroll_x++;
				checker_scroll_y++;
				outportb(WS_SCR1_SCRL_X_PORT, checker_scroll_x);
				outportb(WS_SCR1_SCRL_Y_PORT, checker_scroll_y);				
			}

			tics++;

			// wait for a key to be pressed to start the game
			if (keypad_pushed)
			{
				disable_interrupts();
				hide_screen();

				// copy game graphics
				copy_card_tile_gfx();
				copy_text_gfx();
				copy_you_win_gfx();
				copy_baize_gfx();

				// reset screen 1 scroll
				outportb(WS_SCR1_SCRL_X_PORT, 0);
				outportb(WS_SCR1_SCRL_Y_PORT, 0);

				// draw menu into an offscreen page for screen_2
				draw_menu();

				// set up new game
				new_game();

				// game music
				current_cvgm = entertainer_cvgm;
				music_ticks = VGMSWAN_PLAYBACK_FINISHED;
				
				enable_interrupts();
			}
		}

		// game won screen
		else if (game_state == GAME_WON)
		{
			// need to wait 1 second before you can close the game won screen
			if (tics < 75)
			{
				tics++;
			}

			// wait for a key to be pressed to start a new game
			if (keypad_pushed && tics == 75)
			{
				disable_interrupts();
				current_cvgm = entertainer_cvgm;
				music_ticks = VGMSWAN_PLAYBACK_FINISHED;
				new_game();

				enable_interrupts();
			}
		}

		// dealing cards at start of game
		else if (game_state == GAME_DEALING)
		{
			// hide sprites
			outportb(WS_SPR_COUNT_PORT, 0);

			// still have cards to deal
			if (deck_count > 0)
			{
				move_top_of_deck_to_cascade(deal_x);		

				draw_card_tiles(
					cascades[deal_x][deal_y], 
					(deal_x * 3) + 2, 
					deal_y + 5,
					1
				);

				// move through each cascade in turn
				deal_x = (deal_x + 1) % 8;

				// move to next row after going through all cascades
				if (deal_x == 0)
				{
					deal_y++;
				}
			}

			// all cards dealt
			else
			{
				cursor_y = cascade_counts[cursor_x] - 1;
				game_state = GAME_INGAME;
			}
		}

		// ingame menu
		else if (game_state == GAME_MENU)
		{
			// update checkerboard scrolling
			if (tics % 4 == 0)
			{
				checker_scroll_x++;
				checker_scroll_y++;
				outportb(WS_SCR1_SCRL_X_PORT, checker_scroll_x);
				outportb(WS_SCR1_SCRL_Y_PORT, checker_scroll_y);				
			}

			tics++;

			// cursor up
			if (keypad_pushed & WS_KEY_X1)
			{
				menu_cursor = (menu_cursor - 1);

				if (menu_cursor == 255)
				{
					menu_cursor = 2;
				}
			}
			// cursor down
			else if (keypad_pushed & WS_KEY_X3)
			{
				menu_cursor = (menu_cursor + 1) % 3;
			}

			if (keypad_pushed & WS_KEY_A)
			{
				// reset screen 1 scroll
				outportb(WS_SCR1_SCRL_X_PORT, 0);
				outportb(WS_SCR1_SCRL_Y_PORT, 0);

				// Back
				if (menu_cursor == 2)
				{
					// change screen_2 base address back to the card screen map
					outportb(WS_SCR_BASE_PORT, WS_SCR_BASE_ADDR1(screen_1) | WS_SCR_BASE_ADDR2(screen_2));

					game_state = GAME_INGAME;
				}

				// new game
				else if (menu_cursor == 1)
				{
					disable_interrupts();
					new_game();
					enable_interrupts();
				}

				// retry game
				else if (menu_cursor == 0)
				{
					rnd_val = game_seed;
					
					disable_interrupts();
					new_game();
					enable_interrupts();
				}
			}
			else if ((keypad_pushed & WS_KEY_START) || (keypad_pushed & WS_KEY_B))
			{
				// reset screen 1 scroll
				outportb(WS_SCR1_SCRL_X_PORT, 0);
				outportb(WS_SCR1_SCRL_Y_PORT, 0);
				
				// change screen_2 base address to the card screen map
				outportb(WS_SCR_BASE_PORT, WS_SCR_BASE_ADDR1(screen_1) | WS_SCR_BASE_ADDR2(screen_2));

				game_state = GAME_INGAME;
			}

			// cursor position
			sprites[0].x = 152;
			sprites[0].y = 50 + (menu_cursor << 4);

			sprites[1].x = sprites[0].x;
			sprites[1].y = sprites[0].y + 8;
		}

		// ingame
		else if (game_state == GAME_INGAME)
		{
			// pick up or put down a card
			if (keypad_pushed & WS_KEY_A)
			{				
				// no card currently
				if (card_in_hand == NO_CARD)
				{
					take_card();
				}
				else
				{
					place_card();

					// check if we've won
					if (check_if_game_won())
					{
						set_up_you_win_sprites();

						// change to You Win music
						current_cvgm = you_win_cvgm;
						music_ticks = VGMSWAN_PLAYBACK_FINISHED;

						game_state = GAME_WON;
						tics = 0;
					}
				}
			}

			// return card to its source
			else if (keypad_pushed & WS_KEY_B && card_in_hand != NO_CARD)
			{
				return_card();
			}

			// up/down
			if (keypad_pushed & WS_KEY_X1 || keypad_pushed & WS_KEY_X3)
			{
				// moving up from the cascades
				if (cursor_area == AREA_CASCADES)
				{
					if (cursor_x < 4)
					{
						cursor_area = AREA_FREECELLS;
					}
					else
					{
						cursor_area = AREA_FOUNDATIONS;
						cursor_x = cursor_x - 4;
					}

					cursor_y = 0;
				}
				// moving back down to the cascades
				else
				{
					if (cursor_area == AREA_FOUNDATIONS)
					{
						cursor_x = cursor_x + 4;
					}

					cursor_area = AREA_CASCADES;
					cursor_y = cascade_counts[cursor_x] > 0
								? cascade_counts[cursor_x] - 1
								: 0;
				}
			}

			// move cursor
			if (keypad_pushed & WS_KEY_X4 || keypad_pushed & WS_KEY_X2)
			{
				// left
				if (keypad_pushed & WS_KEY_X4)
				{
					cursor_x = cursor_x - 1;
				}
				// right
				else if (keypad_pushed & WS_KEY_X2)
				{
					cursor_x = cursor_x + 1;
				}

				// moving left and right within the cascades
				if (cursor_area == AREA_CASCADES)
				{
					cursor_x = cursor_x % CASCADES;
					cursor_y = (cascade_counts[cursor_x] > 0)
								? (cascade_counts[cursor_x] - 1) 
								: 0;
				}

				// moving left and right within the foundations
				else if (cursor_area == AREA_FOUNDATIONS)
				{
					// move into the freecells
					if (cursor_x == 4)
					{
						cursor_x = 0;
						cursor_area = 0;
					}
					else if (cursor_x == 255)
					{
						cursor_x = 3;
						cursor_area = 0;
					}
				}

				// moving left and right within the freecells
				else if (cursor_area == AREA_FREECELLS)
				{
					// move into the foundations
					if (cursor_x == 4)
					{
						cursor_area = AREA_FOUNDATIONS;
						cursor_x = 0;
						cursor_y = 0;
					}
					else if (cursor_x == 255)
					{
						cursor_area = AREA_FOUNDATIONS;
						cursor_x = 3;
						cursor_y = 0;
					}
				}
			}

			// start button opens the menu
			if (keypad_pushed & WS_KEY_START)
			{
				// change screen_2 base address to the menu screen map
				outportb(WS_SPR_COUNT_PORT, 2);
				outportb(WS_SCR_BASE_PORT, WS_SCR_BASE_ADDR1(screen_1_page_2) | WS_SCR_BASE_ADDR2(screen_2_page_2));
				
				// reset screen 1 scroll
				outportb(WS_SCR1_SCRL_X_PORT, 0);
				outportb(WS_SCR1_SCRL_Y_PORT, 0);

				checker_scroll_x = checker_scroll_y = 0;

				menu_cursor = 0;
				game_state = GAME_MENU;
			}

			// update cursor and camera position if the state is still ingame
			if (game_state == GAME_INGAME)
			{
				camera_y = (cursor_y < 9) 
							? 0 
							: (cursor_y - 9) * 8;

				outportb(WS_SCR1_SCRL_Y_PORT, camera_y);
				outportb(WS_SCR2_SCRL_Y_PORT, camera_y);

				draw_cursor();
			}
		}

		keypad_last = keypad;
	}

}
