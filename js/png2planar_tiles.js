// png2planar tiles
// Joe Kennedy 2023

// output in a 4bpp planar format 

let libpng = require("node-libpng");
let fs = require('fs');

let args = process.argv.slice(2)

// load image file
let png = libpng.readPngFileSync(args[0]);

console.log(png);

let prefix = args[1] ? args[1] : "tiles";

let outbyte = 0;

let bpp = 4;
let bitmasks = [0x1, 0x2, 0x4, 0x8];

// write header file
let h_out = fs.openSync("include\\" + prefix + ".h", "w");

fs.writeSync(h_out, "#include <stdint.h>\n");
fs.writeSync(h_out, "#pragma once\n\n");
fs.writeSync(h_out, "extern const uint8_t __far " + prefix + "[];\n");
fs.writeSync(h_out, "extern const uint16_t __far " + prefix + "_palette[];\n");

fs.closeSync(h_out);

// write header file
let out = fs.openSync("src\\" +  prefix + ".c", "w");

fs.writeSync(out, "#include \"" + prefix + ".h\"\n");
fs.writeSync(out, "const uint8_t __far " + prefix + "[] = { \n");

// tiles y
for (var ty = 0; ty < png.height / 8; ty++)
{
	// tiles x
	for (var tx = 0; tx < png.width / 8; tx++)
	{
		// row within tile
		for (var row = 0; row < 8; row++)
		{			
			// bit within tile
			for (var b = 0; b < bpp; b++)
			{
				outbyte = 0;

				for (var pixel = 0; pixel < 8; pixel++)
				{
					outbyte |= ((png.data[pixel + (tx * 8) + ((row + (ty * 8)) * png.width)] & bitmasks[b]) >> b) << (7 - pixel);
				}

				fs.writeSync(out, outbyte + ", ");
			}
		}
	}

	fs.writeSync(out, "\n");
}

	// each row of the map in tiles
	for (var j = 0; j < png.height / 8; j++)
	{

		for (var b = 0; b < bpp; b++)
		{
			outbyte = 0;

			// each pixel row of each tile
			for (var t = 0; t < 8; t++)
			{

				// each pixel
				for (var i = 0; i < 8; i++)
				{
					outbyte |= ((png.data[i + (((j * 8) + t) * png.width)] & bitmasks[b]) >> b) << (7 - (i % 8));

					if (i % 8 == 7)
					{
						fs.writeSync(out, outbyte + ", ");
						outbyte = 0;
					}
				}
			}
			
			fs.writeSync(out, "\n");
		}
	}

fs.writeSync(out, "};\n\n");

// palette entries
fs.writeSync(out, "const uint16_t __far " + prefix + "_palette[] = { \n");

png.palette.forEach(function (val) {
	var colour_12 = ((val[0] >> 4) << 8) | 
					((val[1] >> 4) << 4) | 
					((val[2] >> 4) << 0);

	console.log(colour_12);


	fs.writeSync(out, Math.floor(colour_12) + ", ");
});

fs.writeSync(out, "};\n");

fs.closeSync(out);