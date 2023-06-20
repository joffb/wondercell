// Wondercell
// Joe Kennedy - 2023

#include "rand.h"

uint8_t rand()
{
    rnd_val ^= rnd_val << 7;
    rnd_val ^= rnd_val >> 9;
    rnd_val ^= rnd_val << 8;

    return rnd_val;
}