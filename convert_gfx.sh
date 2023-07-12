# title screen
superfamiconv -R -M wsc -B 4 -T 16 -i gfx/title_screen.png -t cbin/title_screen_tiles.bin -m cbin/title_screen_map.bin -p cbin/title_screen_palette.bin

# card tiles
superfamiconv -R -D -F -M wsc -B 4 -i gfx/cards.png -t cbin/cards_tiles.bin -p cbin/cards_palette.bin

# text
superfamiconv -R -D -F -M wsc -B 4 -i gfx/text.png -t cbin/text_tiles.bin

# you win
superfamiconv -R -D -F -M wsc -B 4 -i gfx/you_win.png -t cbin/you_win_tiles.bin

# baize
superfamiconv -R -D -F -M wsc -B 4 -i gfx/baize.png -t cbin/baize_tiles.bin -p cbin/baize_palette.bin