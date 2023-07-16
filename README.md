# wondercell
FreeCell for Wonderswan Colour using Wonderful Toolchain

![screenshot of wondercell](https://github.com/joffb/wondercell/blob/main/screenshots/screenshot.png?raw=true)

Built using Asie's [Wonderful Toolchain](https://github.com/WonderfulToolchain/wonderful-i8086)

Graphics drawn in Aseprite

Tilemaps for menu and title screen in [Tilemap Studio](https://github.com/Rangi42/tilemap-studio)

WonderWitch support, LZSA graphics compression and VGM playback added by Asie!

Controls:
+ X dpad to move the cursor
+ A to pick up or place down a card
+ B to return a card you've picked up to where it came from
+ Start to open the menu

With Wonderful Toolchain and the Wonderswan target installed you can build it by running
```
./convert_gfx.sh
make
```

And for WonderWitch:
```
make -f Makefile.wwitch
```

Still to do:
+ Moving multiple cards at a time
+ Fades/transitions between screens
+ Sound!