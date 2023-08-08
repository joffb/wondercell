local process = require("wf.api.v1.process")
local lzsa = require("wf.api.v1.process.tools.lzsa")
local superfamiconv = require("wf.api.v1.process.tools.superfamiconv")

local tilemap_mono = superfamiconv.convert_tilemap(
	"title_screen_mono.png",
	superfamiconv.config()
		:mode("ws"):bpp(2)
		:tile_base(16):palette_base(12):no_remap()
)
local tilemap_color = superfamiconv.convert_tilemap(
	"title_screen.png",
	superfamiconv.config()
		:mode("wsc"):bpp(4)
		:tile_base(16):palette_base(12):no_remap()
)

tilemap_mono.tiles = lzsa.compress2(tilemap_mono.tiles)
tilemap_color.tiles = lzsa.compress2(tilemap_color.tiles)

process.emit_symbol("gfx_title_screen_mono", tilemap_mono)
process.emit_symbol("gfx_title_screen", tilemap_color)
