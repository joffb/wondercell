local process = require("wf.api.v1.process")
local lzsa = require("wf.api.v1.process.tools.lzsa")
local superfamiconv = require("wf.api.v1.process.tools.superfamiconv")

local tileset_mono = superfamiconv.convert_tileset(
	"text_mono.png",
	superfamiconv.config()
		:mode("ws"):bpp(2)
		:tile_direct()
)
local tileset_color = superfamiconv.convert_tileset(
	"text.png",
	superfamiconv.config()
		:mode("wsc"):bpp(4)
		:tile_direct()
)

tileset_mono.tiles = lzsa.compress2(tileset_mono.tiles)
tileset_color.tiles = lzsa.compress2(tileset_color.tiles)

process.emit_symbol("gfx_text_mono", tileset_mono)
process.emit_symbol("gfx_text", tileset_color)
