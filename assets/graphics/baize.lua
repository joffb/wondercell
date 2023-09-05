local process = require("wf.api.v1.process")
local superfamiconv = require("wf.api.v1.process.tools.superfamiconv")

process.emit_symbol("gfx_baize_mono", superfamiconv.convert_tileset(
	"baize_mono.png",
	superfamiconv.config()
		:mode("ws"):bpp(2)
		:tile_direct()
))
process.emit_symbol("gfx_baize", superfamiconv.convert_tileset(
	"baize.png",
	superfamiconv.config()
		:mode("wsc"):bpp(4)
		:tile_direct()
))
