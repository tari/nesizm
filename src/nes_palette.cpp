// palette and color support

#include "platform.h"
#include "debug.h"
#include "nes.h"

#include "settings.h"

typedef struct RgbColor
{
   uint8 r;
   uint8 g;
   uint8 b;
} RgbColor;

typedef struct HsvColor
{
   uint8 h;
   uint8 s;
   uint8 v;
} HsvColor;

RgbColor HsvToRgb(HsvColor hsv);
HsvColor RgbToHsv(RgbColor rgb);

// harcoded palettes defined at the bottom of the file
extern const uint8 fceuxPalette[64 * 3];
extern const uint8 classicPalette[64 * 3];
extern const uint8 smoothFBXPalette[64 * 3];

const uint8* GetCustomPalette() {
	static uint8 customPalette[64 * 3];
	static bool customPaletteLoaded = false;
	static bool hasCustomPalette = false;

	if (customPaletteLoaded == false) {
		customPaletteLoaded = true;

		unsigned short palName[32];
		Bfile_StrToName_ncpy(palName, "\\\\fls0\\Custom.pal", 32);

		// try to load file
		int file = Bfile_OpenFile_OS(palName, READ, 0);
		if (file < 0) {
			return nullptr;
		}

		// read palette
		if (Bfile_ReadFile_OS(file, customPalette, 64 * 3, 0) != 64 * 3) {
			return nullptr;
		}

		hasCustomPalette = true;
	}

	return hasCustomPalette ? customPalette : nullptr;
}

void nes_ppu::initPalette() {
	// build 16-bit 565 palette from our source 24 bit palettes based on options
	const uint8* srcPalette = fceuxPalette;
	if (nesSettings.GetSetting(ST_Palette) == 1)
		srcPalette = classicPalette;
	else if (nesSettings.GetSetting(ST_Palette) == 2)
		srcPalette = smoothFBXPalette;
	else if (nesSettings.GetSetting(ST_Palette) == 3) {
		// attempt to load a custom palette
		if (GetCustomPalette()) {
			srcPalette = GetCustomPalette();
		}
	}

	for (int c = 0; c < 64; c++ ,srcPalette += 3) {
		uint32 r = srcPalette[0];
		uint32 g = srcPalette[1];
		uint32 b = srcPalette[2];

		if (nesSettings.GetSetting(ST_Color) != 5 || nesSettings.GetSetting(ST_Brightness) != 5) {
			HsvColor hsv = RgbToHsv({ (uint8)r, (uint8)g, (uint8)b });

			// brightness 0 = 30%, 10 = 170%
			int brightnessFactor = (nesSettings.GetSetting(ST_Brightness) * 14) + 30;
			int vibrance = hsv.v * brightnessFactor / 100;
			if (vibrance > 255) vibrance = 255;
			hsv.v = vibrance;

			// color 0 = 0%, 10 = 200%
			int colorFactor = (nesSettings.GetSetting(ST_Color) * 20) + 0;
			int color = hsv.s * colorFactor / 100;
			if (color > 255) color = 255;
			hsv.s = color;

			// convert back to hsv
			{
				RgbColor rgb = HsvToRgb(hsv);
				r = rgb.r;
				g = rgb.g;
				b = rgb.b;
			}
		}

		rgbPalette[c] =
			((min(r + 4, 255) >> 3) << 11) |
			((min(g + 2, 255) >> 2) << 5) |
			((min(b + 4, 255) >> 3) << 0);
	}
}

// resolves base palette to our working palette based on selected indices for sprites and backgrounds, while
// also applying emphasis bits as requested
void nes_ppu::resolveWorkingPalette() {
	for (int i = 0; i < 0x20; i++) {
		if (i & 3) {
			workingPalette[i] = rgbPalette[palette[i]];
		} else {
			workingPalette[i] = rgbPalette[palette[0]];
		}
	}

	// emphasis bits
	uint32 emph = PPUMASK & (PPUMASK_EMPHRED | PPUMASK_EMPHGREEN | PPUMASK_EMPHBLUE);
	if (emph) {
		int shiftRed = 1;
		int shiftGreen = 1;
		int shiftBlue = 1;
		if (emph & PPUMASK_EMPHRED) {
			shiftRed -= 1;
			shiftGreen += 1;
			shiftBlue += 1;
		}
		if (emph & PPUMASK_EMPHGREEN) {
			shiftRed += 1;
			shiftGreen -= 1;
			shiftBlue += 1;
		}
		if (emph & PPUMASK_EMPHBLUE) {
			shiftRed += 1;
			shiftGreen += 1;
			shiftBlue -= 1;
		}

		const uint16 ShiftUpTable[32] =
		{
			0b00000, 0b00010, 0b00100, 0b00110, 0b01000, 0b01010, 0b01100, 0b01110,
			0b10000, 0b10010, 0b10100, 0b10110, 0b11000, 0b11010, 0b11100, 0b11110,
			0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111,
			0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111,
		};
		const uint16 ShiftNoneTable[32] =
		{
			0b00000, 0b00001, 0b00010, 0b00011, 0b00100, 0b00101, 0b00110, 0b00111,
			0b01000, 0b01001, 0b01010, 0b01011, 0b01100, 0b01101, 0b01110, 0b01111,
			0b10000, 0b10001, 0b10010, 0b10011, 0b10100, 0b10101, 0b10110, 0b10111,
			0b11000, 0b11001, 0b11010, 0b11011, 0b11100, 0b11101, 0b11110, 0b11111,
		};
		const uint16 ShiftDownTable[32] =
		{
			0b00000, 0b00001, 0b00010, 0b00010, 0b00011, 0b00100, 0b00101, 0b00101,
			0b00110, 0b00111, 0b01000, 0b01000, 0b01001, 0b01010, 0b01011, 0b01011,
			0b01100, 0b01101, 0b01110, 0b01110, 0b01111, 0b10000, 0b10001, 0b10001,
			0b10010, 0b10011, 0b10100, 0b10100, 0b10101, 0b10110, 0b10111, 0b10111,
		};
		const uint16 ShiftDown2Table[32] =
		{
			0b00000, 0b00001, 0b00001, 0b00001, 0b00010, 0b00010, 0b00011, 0b00011,
			0b00100, 0b00100, 0b00101, 0b00101, 0b00110, 0b00110, 0b00111, 0b00111,
			0b01000, 0b01000, 0b01001, 0b01001, 0b01010, 0b01010, 0b01011, 0b01011,
			0b01100, 0b01100, 0b01101, 0b01101, 0b01110, 0b01110, 0b01111, 0b01111,
		};
		const uint16* ShiftTables[5] = { ShiftUpTable, ShiftNoneTable, ShiftDownTable, ShiftDown2Table, ShiftDown2Table };

		for (int32 i = 0; i < 0x20; i++) {
			workingPalette[i] =
				(ShiftTables[shiftRed + 1][(workingPalette[i] & 0b1111100000000000) >> 11] << 11) |
				(ShiftTables[shiftGreen + 1][(workingPalette[i] & 0b0000011111000000) >> 6] << 6) |
				(workingPalette[i] & 0b0000000000100000) |
				ShiftTables[shiftBlue + 1][workingPalette[i] & 0b0000000000011111];
		}
	}

	dirtyPalette = false;
}

// HSV <-> RGB conversion without floats lifted from 
// https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both

RgbColor HsvToRgb(HsvColor hsv)
{
    RgbColor rgb;
    unsigned char region, remainder, p, q, t;

    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6; 

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.r = hsv.v; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = hsv.v; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = hsv.v; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v; rgb.g = p; rgb.b = q;
            break;
    }

    return rgb;
}

HsvColor RgbToHsv(RgbColor rgb)
{
    HsvColor hsv;
    unsigned char rgbMin, rgbMax;

    rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

    hsv.v = rgbMax;
    if (hsv.v == 0)
    {
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
    if (hsv.s == 0)
    {
        hsv.h = 0;
        return hsv;
    }

    if (rgbMax == rgb.r)
        hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.g)
        hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
    else
        hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

    return hsv;
}

// hardcoded palettes

const uint8 smoothFBXPalette[64 * 3] = {
	0x69, 0x6B, 0x63, 0x00, 0x17, 0x74, 0x1E, 0x00, 0x87, 0x34, 0x00, 0x73, 0x56, 0x00, 0x57, 0x5E,
	0x00, 0x13, 0x53, 0x1A, 0x00, 0x3B, 0x24, 0x00, 0x24, 0x30, 0x00, 0x06, 0x3A, 0x00, 0x00, 0x3F,
	0x00, 0x00, 0x3B, 0x1E, 0x00, 0x33, 0x4E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xB9, 0xBB, 0xB3, 0x14, 0x53, 0xB9, 0x4D, 0x2C, 0xDA, 0x67, 0x1E, 0xDE, 0x98, 0x18, 0x9C, 0x9D,
	0x23, 0x44, 0xA0, 0x3E, 0x00, 0x8D, 0x55, 0x00, 0x65, 0x6D, 0x00, 0x2C, 0x79, 0x00, 0x00, 0x81,
	0x00, 0x00, 0x7D, 0x42, 0x00, 0x78, 0x8A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0x69, 0xA8, 0xFF, 0x96, 0x91, 0xFF, 0xB2, 0x8A, 0xFA, 0xEA, 0x7D, 0xFA, 0xF3,
	0x7B, 0xC7, 0xF2, 0x8E, 0x59, 0xE6, 0xAD, 0x27, 0xD7, 0xC8, 0x05, 0x90, 0xDF, 0x07, 0x64, 0xE5,
	0x3C, 0x45, 0xE2, 0x7D, 0x48, 0xD5, 0xD9, 0x4E, 0x50, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xD2, 0xEA, 0xFF, 0xE2, 0xE2, 0xFF, 0xE9, 0xD8, 0xFF, 0xF5, 0xD2, 0xFF, 0xF8,
	0xD9, 0xEA, 0xFA, 0xDE, 0xB9, 0xF9, 0xE8, 0x9B, 0xF3, 0xF2, 0x8C, 0xD3, 0xFA, 0x91, 0xB8, 0xFC,
	0xA8, 0xAE, 0xFA, 0xCA, 0xCA, 0xF3, 0xF3, 0xBE, 0xC0, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const uint8 fceuxPalette[64 * 3] = {
	0x60, 0x60, 0x60, 0x00, 0x00, 0x78, 0x14, 0x00, 0x80, 0x2C, 0x00, 0x6E, 0x4A, 0x00, 0x4E, 0x6C,
	0x00, 0x18, 0x5A, 0x03, 0x02, 0x51, 0x18, 0x00, 0x34, 0x24, 0x00, 0x00, 0x34, 0x00, 0x00, 0x32,
	0x00, 0x00, 0x34, 0x20, 0x00, 0x2C, 0x78, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0xC4, 0xC4, 0xC4, 0x00, 0x58, 0xDE, 0x30, 0x1F, 0xFC, 0x7F, 0x14, 0xE0, 0xA8, 0x00, 0xB0, 0xC0,
	0x06, 0x5C, 0xC0, 0x2B, 0x0E, 0xA6, 0x40, 0x10, 0x6F, 0x61, 0x00, 0x30, 0x80, 0x00, 0x00, 0x7C,
	0x00, 0x00, 0x7C, 0x3C, 0x00, 0x6E, 0x84, 0x14, 0x14, 0x14, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0xF0, 0xF0, 0xF0, 0x4C, 0xAA, 0xFF, 0x6F, 0x73, 0xF5, 0xB0, 0x70, 0xFF, 0xDA, 0x5A, 0xFF, 0xF0,
	0x60, 0xC0, 0xF8, 0x83, 0x6D, 0xD0, 0x90, 0x30, 0xD4, 0xC0, 0x30, 0x66, 0xD0, 0x00, 0x26, 0xDD,
	0x1A, 0x2E, 0xC8, 0x66, 0x34, 0xC2, 0xBE, 0x54, 0x54, 0x54, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0xFF, 0xFF, 0xFF, 0xB6, 0xDA, 0xFF, 0xC8, 0xCA, 0xFF, 0xDA, 0xC2, 0xFF, 0xF0, 0xBE, 0xFF, 0xFC,
	0xBC, 0xEE, 0xFF, 0xD0, 0xB4, 0xFF, 0xDA, 0x90, 0xEC, 0xEC, 0x92, 0xDC, 0xF6, 0x9E, 0xB8, 0xFF,
	0xA2, 0xAE, 0xEA, 0xBE, 0x9E, 0xEF, 0xEF, 0xBE, 0xBE, 0xBE, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};

const uint8 classicPalette[64 * 3] = {
	0x61, 0x61, 0x61, 0x00, 0x00, 0x88, 0x1F, 0x0D, 0x99, 0x37, 0x13, 0x79, 0x56, 0x12, 0x60, 0x5D,
	0x00, 0x10, 0x52, 0x0E, 0x00, 0x3A, 0x23, 0x08, 0x21, 0x35, 0x0C, 0x0D, 0x41, 0x0E, 0x17, 0x44,
	0x17, 0x00, 0x3A, 0x1F, 0x00, 0x2F, 0x57, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xAA, 0xAA, 0xAA, 0x0D, 0x4D, 0xC4, 0x4B, 0x24, 0xDE, 0x69, 0x12, 0xCF, 0x90, 0x14, 0xAD, 0x9D,
	0x1C, 0x48, 0x92, 0x34, 0x04, 0x73, 0x50, 0x05, 0x5D, 0x69, 0x13, 0x16, 0x7A, 0x11, 0x13, 0x80,
	0x08, 0x12, 0x76, 0x49, 0x1C, 0x66, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFC, 0xFC, 0xFC, 0x63, 0x9A, 0xFC, 0x8A, 0x7E, 0xFC, 0xB0, 0x6A, 0xFC, 0xDD, 0x6D, 0xF2, 0xE7,
	0x71, 0xAB, 0xE3, 0x86, 0x58, 0xCC, 0x9E, 0x22, 0xA8, 0xB1, 0x00, 0x72, 0xC1, 0x00, 0x5A, 0xCD,
	0x4E, 0x34, 0xC2, 0x8E, 0x4F, 0xBE, 0xCE, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFC, 0xFC, 0xFC, 0xBE, 0xD4, 0xFC, 0xCA, 0xCA, 0xFC, 0xD9, 0xC4, 0xFC, 0xEC, 0xC1, 0xFC, 0xFA,
	0xC3, 0xE7, 0xF7, 0xCE, 0xC3, 0xE2, 0xCD, 0xA7, 0xDA, 0xDB, 0x9C, 0xC8, 0xE3, 0x9E, 0xBF, 0xE5,
	0xB8, 0xB2, 0xEB, 0xC8, 0xB7, 0xE5, 0xEB, 0xAC, 0xAC, 0xAC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};