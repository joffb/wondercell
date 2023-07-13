// Wondercell
// Joe Kennedy - 2023

#include <wonderful.h>
#include <ws.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __WONDERFUL_WWITCH__
#include <sys/bios.h>
#else
// see wfconfig.toml
#define WAVE_RAM ((uint8_t __wf_iram*) 0xEC0)
#endif
#include "card.h"
#include "draw.h"
#include "vgm.h"
#include "entertainer_cvgm_bin.h"

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

void disable_interrupts()
{
#ifndef __WONDERFUL_WWITCH__
	// disable cpu interrupts
	cpu_irq_disable();

	// disable wonderswan hardware interrupts
	ws_hwint_disable_all();
#endif
}

void enable_interrupts()
{
#ifndef __WONDERFUL_WWITCH__
	// acknowledge interrupt
	outportb(IO_HWINT_ACK, 0xFF);

	// set interrupt handler which only acknowledges the vblank interrupt
	ws_hwint_set_default_handler_vblank();

	// enable wonderswan vblank interrupt
	ws_hwint_enable(HWINT_VBLANK);

	// enable cpu interrupts
	cpu_irq_enable();
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

	outportb(IO_SCR_BASE, SCR1_BASE(SCREEN_1) | SCR2_BASE(SCREEN_2));

	// default cursor to first cascade
	cursor_area = AREA_CASCADES;
	cursor_x = 0;

	// setup cursor sprites
	SPRITES[0].tile = CURSOR_TILES;
	SPRITES[0].palette = 0;
	SPRITES[0].priority = 1;

	SPRITES[1].tile = CURSOR_TILES + 1;
	SPRITES[1].palette = 0;
	SPRITES[1].priority = 1;

	show_game_screen();

	// do dealing out the cards animation to start with
	deal_x = deal_y = 0;
	game_state = GAME_DEALING;

	// no cards in hand
	card_in_hand = NO_CARD;
	card_in_hand_tiles_count = 0;
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
	outportb(IO_SND_WAVE_BASE, SND_WAVE_BASE(WAVE_RAM));
#endif

	// initial game state
	game_state = GAME_TITLE;

	// show title screen
	outportb(IO_SCR_BASE, SCR1_BASE(SCREEN_1_PAGE_2) | SCR2_BASE(SCREEN_2));
	show_title_screen();

	// reenable interrupts
	enable_interrupts();

	// main loop
	while (1)
	{
		// halt cpu
		// the program will sit here until the vblank interrupt
		// is triggered and unhalts it
#ifdef __WONDERFUL_WWITCH__
		sys_wait(1);
#else
		cpu_halt();
#endif

		// play music
		if (music_ticks == VGMSWAN_PLAYBACK_FINISHED)
		{
			// initialize music
			vgmswan_init(&music_state, entertainer_cvgm);
			music_ticks = 0;
		}

		if (music_ticks > 1)
			music_ticks--;
		else
			music_ticks = vgmswan_play(&music_state);

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
			outportb(IO_SPR_COUNT, 0);

			// update checkerboard scrolling
			if (tics % 4 == 0)
			{
				checker_scroll_x++;
				checker_scroll_y++;
				outportb(IO_SCR1_SCRL_X, checker_scroll_x);
				outportb(IO_SCR1_SCRL_Y, checker_scroll_y);				
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
				outportb(IO_SCR1_SCRL_X, 0);
				outportb(IO_SCR1_SCRL_Y, 0);

				// draw menu into an offscreen page for SCREEN_2
				draw_menu();

				// set up new game
				new_game();
				
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
				new_game();
				enable_interrupts();
			}
		}

		// dealing cards at start of game
		else if (game_state == GAME_DEALING)
		{
			// hide sprites
			outportb(IO_SPR_COUNT, 0);

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
				outportb(IO_SCR1_SCRL_X, checker_scroll_x);
				outportb(IO_SCR1_SCRL_Y, checker_scroll_y);				
			}

			tics++;

			// cursor up
			if (keypad_pushed & KEY_X1)
			{
				menu_cursor = (menu_cursor - 1);

				if (menu_cursor == 255)
				{
					menu_cursor = 2;
				}
			}
			// cursor down
			else if (keypad_pushed & KEY_X3)
			{
				menu_cursor = (menu_cursor + 1) % 3;
			}

			if (keypad_pushed & KEY_A)
			{
				// reset screen 1 scroll
				outportb(IO_SCR1_SCRL_X, 0);
				outportb(IO_SCR1_SCRL_Y, 0);

				// Back
				if (menu_cursor == 2)
				{
					// change SCREEN_2 base address back to the card screen map
					outportb(IO_SCR_BASE, SCR1_BASE(SCREEN_1) | SCR2_BASE(SCREEN_2));

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
			else if ((keypad_pushed & KEY_START) || (keypad_pushed & KEY_B))
			{
				// reset screen 1 scroll
				outportb(IO_SCR1_SCRL_X, 0);
				outportb(IO_SCR1_SCRL_Y, 0);
				
				// change SCREEN_2 base address to the card screen map
				outportb(IO_SCR_BASE, SCR1_BASE(SCREEN_1) | SCR2_BASE(SCREEN_2));

				game_state = GAME_INGAME;
			}

			// cursor position
			SPRITES[0].x = 152;
			SPRITES[0].y = 50 + (menu_cursor << 4);

			SPRITES[1].x = SPRITES[0].x;
			SPRITES[1].y = SPRITES[0].y + 8;
		}

		// ingame
		else if (game_state == GAME_INGAME)
		{

			// pick up or put down a card
			if (keypad_pushed & KEY_A)
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

						game_state = GAME_WON;
						tics = 0;
					}
				}
			}

			// return card to its source
			else if (keypad_pushed & KEY_B && card_in_hand != NO_CARD)
			{
				return_card();
			}

			// up/down
			if (keypad_pushed & KEY_X1 || keypad_pushed & KEY_X3)
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
			if (keypad_pushed & KEY_X4 || keypad_pushed & KEY_X2)
			{
				// left
				if (keypad_pushed & KEY_X4)
				{
					cursor_x = cursor_x - 1;
				}
				// right
				else if (keypad_pushed & KEY_X2)
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
			if (keypad_pushed & KEY_START)
			{
				// change SCREEN_2 base address to the menu screen map
				outportb(IO_SPR_COUNT, 2);
				outportb(IO_SCR_BASE, SCR1_BASE(SCREEN_1_PAGE_2) | SCR2_BASE(SCREEN_2_PAGE_2));
				
				// reset screen 1 scroll
				outportb(IO_SCR1_SCRL_X, 0);
				outportb(IO_SCR1_SCRL_Y, 0);

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

				outportb(IO_SCR1_SCRL_Y, camera_y);
				outportb(IO_SCR2_SCRL_Y, camera_y);

				draw_cursor();
			}
		}

		keypad_last = keypad;
	}

}
