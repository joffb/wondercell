#ifndef __IRAM_H__
#define __IRAM_H__

#include <wonderful.h>
#include <ws.h>

#ifdef IRAM_IMPLEMENTATION
#define IRAM_EXTERN
#else
#define IRAM_EXTERN extern
#endif

#ifdef __WONDERFUL_WWITCH__
#define screen_1 ((uint16_t __wf_iram*) 0x1000)
#define screen_1_page_2 ((uint16_t __wf_iram*) 0x1800)
#define screen_2 ((uint16_t __wf_iram*) 0x3000)
#define screen_2_page_2 ((uint16_t __wf_iram*) 0x3800)
#define sprites ((ws_sprite_t __wf_iram*) 0x2e00)
#else
__attribute__((section(".iramx_2bpp_2000")))
IRAM_EXTERN uint8_t tile_2bpp[WS_DISPLAY_TILE_SIZE * 256];
__attribute__((section(".iramx_4bpp_4000")))
IRAM_EXTERN uint8_t tile_4bpp[WS_DISPLAY_TILE_SIZE_4BPP * 256];
__attribute__((section(".iramx_wave")))
IRAM_EXTERN ws_sound_wavetable_t wave_ram;
__attribute__((section(".iramx_screen.1")))
IRAM_EXTERN uint16_t screen_1[32*32];
__attribute__((section(".iramx_screen.2")))
IRAM_EXTERN uint16_t screen_1_page_2[32*32];
__attribute__((section(".iramx_screen.3")))
IRAM_EXTERN uint16_t screen_2[32*32];
__attribute__((section(".iramx_screen.4")))
IRAM_EXTERN uint16_t screen_2_page_2[32*32];
__attribute__((section(".iramx_sprite")))
IRAM_EXTERN ws_sprite_t sprites[128];
#endif

#undef IRAM_EXTERN

#endif /* __IRAM_H__ */
