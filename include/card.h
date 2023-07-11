// Wondercell
// Joe Kennedy - 2023

#pragma once
#include <wonderful.h>

#define FREECELLS 4
#define FOUNDATIONS 4
#define CASCADES 8

#define NO_CARD 0xff

enum cursor_areas {
  AREA_FREECELLS = 0,
  AREA_FOUNDATIONS,
  AREA_CASCADES,
}; 

// x and y pixel locations of cursor areas
extern const uint8_t __wf_rom cursor_area_tx[];
extern const uint8_t __wf_rom cursor_area_ty[];

extern uint8_t cursor_area;
extern uint8_t cursor_x;
extern uint8_t cursor_y;

extern uint8_t card_in_hand;
extern uint8_t card_in_hand_area;
extern uint8_t card_in_hand_x;
extern uint8_t card_in_hand_y;

extern ws_sprite_t card_in_hand_tiles[32];
extern uint8_t card_in_hand_tiles_count;

extern uint8_t cards[52];
extern uint8_t card_used[52];

extern uint8_t freecells[FREECELLS][1];
extern uint8_t freecell_counts[FREECELLS];

extern uint8_t foundations[FOUNDATIONS][16];
extern uint8_t foundation_counts[FOUNDATIONS];

extern uint8_t deck[52];
extern uint8_t deck_count;

extern uint8_t cascades[CASCADES][32];
extern uint8_t cascade_counts[CASCADES];

void initialise_cards_array();
void initialise_cascades();
void initialise_freecells();
void initialise_foundations();
void initialise_deck();
void shuffle_deck();

uint8_t move_top_of_deck_to_cascade(uint8_t cascade);
uint8_t can_move_card_onto_card(uint8_t card, uint8_t cascade);
uint8_t check_if_game_won();

void take_card();
void place_card();
void return_card();
