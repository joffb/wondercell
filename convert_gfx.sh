# title screen
wf-superfamiconv -R -M wsc -B 4 -T 16 -i gfx/title_screen.png -t cbin/title_screen_tiles.bin -m cbin/title_screen_map.bin -p cbin/title_screen_palette.bin
wf-lzsa -r -f 2 cbin/title_screen_tiles.bin cbin/title_screen_tiles_lzsa2.bin

# card tiles
wf-superfamiconv -R -D -F -M wsc -B 4 -i gfx/cards.png -t cbin/cards_tiles.bin -p cbin/cards_palette.bin

# text
wf-superfamiconv -R -D -F -M wsc -B 4 -i gfx/text.png -t cbin/text_tiles.bin
wf-lzsa -r -f 2 cbin/text_tiles.bin cbin/text_tiles_lzsa2.bin

# you win
wf-superfamiconv -R -D -F -M wsc -B 4 -i gfx/you_win.png -t cbin/you_win_tiles.bin
wf-lzsa -r -f 2 cbin/you_win_tiles.bin cbin/you_win_tiles_lzsa2.bin

# baize
wf-superfamiconv -R -D -F -M wsc -B 4 -i gfx/baize.png -t cbin/baize_tiles.bin -p cbin/baize_palette.bin
