// Wondercell
// Joe Kennedy - 2023

#pragma once

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
extern const uint8_t __far cursor_area_tx[];
extern const uint8_t __far cursor_area_ty[];

uint8_t cursor_area;
uint8_t cursor_x;
uint8_t cursor_y;

uint8_t card_in_hand;
uint8_t card_in_hand_area;
uint8_t card_in_hand_x;
uint8_t card_in_hand_y;

ws_sprite_t card_in_hand_tiles[32];
uint8_t card_in_hand_tiles_count;

uint8_t cards[52];
uint8_t card_used[52];

uint8_t freecells[FREECELLS][1];
uint8_t freecell_counts[FREECELLS];

uint8_t foundations[FOUNDATIONS][16];
uint8_t foundation_counts[FOUNDATIONS];

uint8_t deck[52];
uint8_t deck_count;

uint8_t cascades[CASCADES][32];
uint8_t cascade_counts[CASCADES];

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