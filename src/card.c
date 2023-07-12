// Wondercell
// Joe Kennedy - 2023

#include <stdint.h>
#include <stdlib.h>
#include <ws.h>
#include <wonderful.h>
#include "card.h"
#include "draw.h"

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

const uint8_t __wf_rom cursor_area_tx[] = { 1, 15, 2};
const uint8_t __wf_rom cursor_area_ty[] = { 0, 0,  5};

const char* __wf_rom value_names[13] = {
    "A", "2", "3", "4", "5", "6", "7",
    "8", "9", "10", "J", "Q", "K"
};

const char* __wf_rom suit_names[4] = {
    "Hearts", "Clubs", "Diamonds", "Spades"
};


// clear cascade arrays
void initialise_cascades()
{
    uint8_t i, j;
   
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 32; j++)
        {
            cascades[i][j] = 0xff;
        }
       
        cascade_counts[i] = 0;
    }
}

uint8_t move_top_of_deck_to_cascade(uint8_t cascade)
{
    // sanity check number of cards left in deck
    if (deck_count <= 0)
    {
        return 0;
    }
   
    // sanity check cascade index
    if (cascade < 0 || cascade >= CASCADES)
    {
        return 0;
    }
   
    // get card from top of deck
    uint8_t card = deck[deck_count - 1];
   
    // "remove" it
    deck_count--;
   
    // update the top card of the cascade and keep count
    cascades[cascade][cascade_counts[cascade]] = card;
    cascade_counts[cascade]++;
   
    return 1;
}

// initialise deck from cards array
// and reset deck count
void initialise_deck()
{
    uint8_t i = 0;
   
    // initialise deck
    for (i = 0; i < 52; i++)
    {
        deck[i] = cards[i];
    }
   
    deck_count = 52;
}

void initialise_freecells()
{
    uint8_t i;

	// clear freecells and foundations
	for (i = 0; i < 4; i++)
	{
		freecells[i][0] = NO_CARD;
	}
}

void initialise_foundations()
{
    uint8_t i, j;

	// clear freecells and foundations
	for (i = 0; i < 4; i++)
	{
        foundation_counts[i] = 0;

		for (j = 0; j < 16; j++)
		{
			foundations[i][j] = NO_CARD;
		}
	}
}


void shuffle_deck()
{
    uint8_t i;
    uint8_t tmp;
    uint16_t rnd;
   
    // fisher–yates shuffle
    for (i = 0; i < deck_count; i++)
    {
        // get random location to swap with
        rnd = i + (rand() % (deck_count - i));
       
        // swap card at index i
        // with the card at index rnd
        tmp = deck[rnd];
        deck[rnd] = deck[i];
        deck[i] = tmp;
    }
}

void initialise_cards_array()
{
    uint8_t i = 0;
   
    // four suits
    for (uint8_t suit = 0; suit < 4; suit++)
    {
        // cards with values 0-12 for [A, 2, 3 ... J, Q, K]
        for (uint8_t value = 0; value < 13; value++)
        {
            // upper nibble has the card's suit
            // lower nibble has the card's value
            cards[i] = value + (suit << 4);
            i++;
        }  
    }
}

// check if you can move card1 onto card2
uint8_t can_move_card_onto_card(uint8_t card1, uint8_t card2)
{   
    // new card's value should be 1 less than the cascade card's
    if (((card1 & 0xf) + 1) != (card2 & 0xf))
    {
        return 0;
    }
   
    // lowest bit of the upper nibble says if the card is
    // black or red, so if that bit is different for both cards
    // then when xor'ed that bit should be 1
    if (((card1 ^ card2) & 0x10) != 0x10)
    {
        return 0;
    }
   
    // no problems found
    return 1;
}

// game is essentially won if there are no cards "stuck" in the cascades
// e.g. a black 2 under a red 9
uint8_t check_if_game_won()
{
    uint8_t i, j;

    for (i = 0; i < CASCADES; i++)
    {
        for (j = 1; j < cascade_counts[i]; j++)
        {
            // use can_move_card_onto_card to determine whether the card at j
            // is sequential and a different colour to the card at j - 1
            // if it's not, we haven't won yet
            if (can_move_card_onto_card(cascades[i][j], cascades[i][j - 1]) == 0)
            {
                return 0;
            }
        }
    }

    return 1;
}

void take_card()
{
    uint8_t card;

    // take card from cascades
    if (cursor_area == AREA_CASCADES && cascade_counts[cursor_x] > 0)
    {
        card = cascades[cursor_x][cursor_y];

        copy_card_tiles_to_sprites(cursor_area_tx[cursor_area] + (cursor_x * 3), cursor_area_ty[cursor_area] + cursor_y);
        clear_card_tiles(cursor_area_tx[cursor_area] + (cursor_x * 3), cursor_area_ty[cursor_area] + cursor_y);
        cascade_counts[cursor_x]--;

        // check if this is an ace, if it is move it to the foundations
        if ((card & 0xf) == 0)
        {
            foundations[card >> 4][0] = card;
            foundation_counts[card >> 4]++;

            card_in_hand_tiles_count = 0;

            draw_card_tiles(
                card,
                cursor_area_tx[AREA_FOUNDATIONS] + ((card >> 4) * 3), 
                cursor_area_ty[AREA_FOUNDATIONS],
                1
            );
        }
        else
        {
            card_in_hand = cascades[cursor_x][cursor_y];
            card_in_hand_area = AREA_CASCADES;
            card_in_hand_x = cursor_x;
            card_in_hand_y = cursor_y;
        }

        if (cursor_y > 0)
        {
            cursor_y--;
        }

        // redraw bottom card of cascade
        if (cascade_counts[cursor_x] > 0)
        {
            card = cascades[cursor_x][cursor_y];

            draw_card_tiles(
                card,
                cursor_area_tx[cursor_area] + (cursor_x * 3), 
                cursor_area_ty[cursor_area] + cursor_y,
                1
            );
        }
    }

    // take card from freecell
    else if (cursor_area == AREA_FREECELLS && freecells[cursor_x][0] != NO_CARD)
    {
        copy_card_tiles_to_sprites(cursor_area_tx[cursor_area] + (cursor_x * 3), cursor_area_ty[cursor_area] + cursor_y);
        draw_empty_card(cursor_area_tx[AREA_FREECELLS] + (cursor_x * 3), cursor_area_ty[AREA_FREECELLS]);

        card_in_hand = freecells[cursor_x][0];
        card_in_hand_area = AREA_FREECELLS;
        card_in_hand_x = cursor_x;
        card_in_hand_y = cursor_y;

        freecells[cursor_x][0] = NO_CARD;
    }
}

void place_card()
{
    uint8_t i;

    // putting the card down on a cascade
    if (cursor_area == AREA_CASCADES)
    {
        // check if the move is possible
        if (cascade_counts[cursor_x] == 0 || can_move_card_onto_card(card_in_hand, cascades[cursor_x][cursor_y]))
        {
            if (cascade_counts[cursor_x] > 0)
            {
                cursor_y++;
            }

            draw_card_tiles(
                card_in_hand,
                cursor_area_tx[cursor_area] + (cursor_x * 3), 
                cursor_area_ty[cursor_area] + cursor_y,
                1
            );

            cascades[cursor_x][cursor_y] = card_in_hand;
            cascade_counts[cursor_x]++;
            card_in_hand = NO_CARD;
            card_in_hand_tiles_count = 0;				
        }
    }

    // putting the card down on an empty freecell
    else if (cursor_area == AREA_FREECELLS && freecells[cursor_x][0] == NO_CARD)
    {
        draw_card_tiles(
            card_in_hand,
            cursor_area_tx[cursor_area] + (cursor_x * 3), 
            cursor_area_ty[cursor_area] + cursor_y,
            1
        );

        freecells[cursor_x][0] = card_in_hand;
        card_in_hand = NO_CARD;
        card_in_hand_tiles_count = 0;
    }

    // putting the card down on a foundation
    else if (cursor_area == AREA_FOUNDATIONS)
    {
        i = foundation_counts[cursor_x];

        if (
            i > 0 && 
            (card_in_hand - 1) == foundations[cursor_x][i - 1]
        )
        {
            draw_card_tiles(
                card_in_hand,
                cursor_area_tx[cursor_area] + (cursor_x * 3), 
                cursor_area_ty[cursor_area] + cursor_y,
                1
            );

            foundations[cursor_x][i] = card_in_hand;
            foundation_counts[cursor_x]++;

            card_in_hand = NO_CARD;
            card_in_hand_tiles_count = 0;
        }
    }
}

void return_card()
{
    // returning cards to cascades
    if (card_in_hand_area == AREA_CASCADES)
    {
        cascades[card_in_hand_x][card_in_hand_y] = card_in_hand;
        cascade_counts[card_in_hand_x]++;

        // move cursor up
        if (cursor_area == AREA_CASCADES && cursor_x == card_in_hand_x)
        {
            cursor_y = cascade_counts[cursor_x] - 1;
        }
    }

    // returning card to freecell
    else if (card_in_hand_area == AREA_FREECELLS)
    {
        freecells[card_in_hand_x][0] = card_in_hand;
    }

    draw_card_tiles(
        card_in_hand,
        cursor_area_tx[card_in_hand_area] + (card_in_hand_x * 3), 
        cursor_area_ty[card_in_hand_area] + card_in_hand_y,
        1
    );

    card_in_hand = NO_CARD;
    card_in_hand_tiles_count = 0;
}
