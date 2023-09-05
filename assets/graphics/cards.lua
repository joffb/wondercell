local process = require("wf.api.v1.process")
local lzsa = require("wf.api.v1.process.tools.lzsa")
local superfamiconv = require("wf.api.v1.process.tools.superfamiconv")

process.emit_symbol("gfx_cards_mono", superfamiconv.convert_tileset(
	"cards_mono.png",
	superfamiconv.config()
		:mode("ws"):bpp(2)
		:tile_direct()
))
process.emit_symbol("gfx_cards", superfamiconv.convert_tileset(
	"cards.png",
	superfamiconv.config()
		:mode("wsc"):bpp(4)
		:tile_direct()
))
