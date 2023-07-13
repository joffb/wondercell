/**
 * WonderSwan audio playback library
 *
 * Copyright (c) 2022 Adrian "asie" Siekierka
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

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wonderful.h>
#include <ws.h>
#include "vgm.h"

#ifdef __WONDERFUL_WWITCH__
#include <sys/bios.h>
#endif

#define FLAG_USES_BANK 1

void vgmswan_init(vgmswan_state_t *state, const void __far* pointer) {
    state->ptr = pointer;
    state->start_offset = FP_OFF(pointer);
    state->flags = 0;

#ifdef __WONDERFUL_WWITCH__
    sound_init();
    sound_set_output(SND_OUT_HEADPHONES_ENABLE | SND_OUT_SPEAKER_ENABLE | SND_OUT_VOLUME_12_5);
#else
    outportb(IO_SND_OUT_CTRL, SND_OUT_HEADPHONES_ENABLE | SND_OUT_SPEAKER_ENABLE | SND_OUT_VOLUME_12_5);
#endif
}

uint16_t vgmswan_play(vgmswan_state_t *state) {
    const uint8_t __far* ptr = state->ptr;

#ifndef __WONDERFUL_WWITCH__
    uint16_t addrPrefix = (inportb(IO_SND_WAVE_BASE) << 6);
#endif
    uint16_t result = 0;
    bool restorePtr = true;

    while (result == 0) {
        // play routine
        uint8_t cmd = *(ptr++);
        switch (cmd & 0xE0) {
        case 0x00:
        case 0x20: { // memory write
            uint8_t len = *(ptr++);
#ifdef __WONDERFUL_WWITCH__
            // TODO: this won't support unusual write lengths (but Furnace doesn't emit such)
            sound_set_wave(cmd >> 4, ptr);
#else
            uint16_t addr = cmd | addrPrefix;
            memcpy((uint8_t __wf_iram*) addr, ptr, len);
#endif
            ptr += len;
        } break;
        case 0x40: { // port write (byte)
            uint8_t v = *(ptr++);
            outportb(cmd ^ 0xC0, v);
        } break;
        case 0x60: { // port write (word)
            uint16_t v = *((uint16_t __far*) ptr); ptr += 2;
            outportw(cmd ^ 0xE0, v);
        } break;
        case 0xE0: { // special
            switch (cmd) {
            case 0xEF: {
                uint16_t new_pos = *((uint16_t __far*) ptr); ptr += 2;
                state->ptr = ptr;
		ptr = MK_FP(FP_SEG(ptr), state->start_offset + new_pos);
                restorePtr = false;
            } break;
            case 0xF0:
            case 0xF1:
            case 0xF2:
            case 0xF3:
            case 0xF4:
            case 0xF5:
            case 0xF6: {
                result = cmd - 0xEF;
            } break;
            case 0xF8: {
                result = *(ptr++);
            } break;
            case 0xF9: {
                result = *((uint16_t __far*) ptr); ptr += 2;
            } break;
            case 0xFA: {
                uint16_t new_pos = *((uint16_t __far*) ptr); ptr += 2;
		ptr = MK_FP(FP_SEG(ptr), state->start_offset + new_pos);
            } break;
            case 0xFB: {
                uint8_t ctrl = *(ptr++);
                outportb(IO_SDMA_CTRL, 0);
                if (ctrl & 0x80) {
                    // play sample
                    uint32_t source = *((uint16_t __far*) ptr) + state->start_offset + (FP_SEG(ptr) << 4);
                    outportw(IO_SDMA_SOURCE_L, source); ptr += 2;
                    outportb(IO_SDMA_SOURCE_H, source >> 16);
                    outportw(IO_SDMA_COUNTER_L, *((uint16_t __far*) ptr)); ptr += 2;
                    outportb(IO_SDMA_COUNTER_H, 0);
                    outportb(IO_SDMA_CTRL, ctrl);
                }
            } break;
            case 0xFC:
            case 0xFD:
            case 0xFE:
            case 0xFF: {
                uint8_t __far* mem_ptr = MK_FP(FP_SEG(ptr), state->start_offset + *((uint16_t __far*) ptr)); ptr += 2;
#ifdef __WONDERFUL_WWITCH__
                sound_set_wave(cmd & 0x03, mem_ptr);
#else
                uint16_t addr = ((cmd - 0xFC) << 4) | addrPrefix;
                memcpy((uint8_t __wf_iram*) addr, mem_ptr, 16);
#endif
            } break;
            }
        }
        }
    }

    if (restorePtr) {
        state->ptr = ptr;
    }
    return result;
}
