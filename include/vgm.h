/**
 * WonderSwan audio playback library
 *
 * Copyright (c) 2022, 2023 Adrian "asie" Siekierka
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#pragma once
#include <stdint.h>

typedef struct {
    const uint8_t __far* ptr;
    uint16_t start_offset;
    uint8_t flags;
} vgmswan_state_t;

#define VGMSWAN_PLAYBACK_FINISHED 0xFFFF

void vgmswan_init(vgmswan_state_t *state, const void __far* ptr);
// return: amount of HBLANK lines to wait
uint16_t vgmswan_play(vgmswan_state_t *state);
