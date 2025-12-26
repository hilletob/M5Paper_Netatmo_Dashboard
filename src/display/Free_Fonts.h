// Header file for FreeFont library
// This file references the GNU FreeFont library included with Seeed_GFX
// All fonts support German umlauts (ä, ö, ü, Ä, Ö, Ü, ß) and other Latin-1 characters
//
// NOTE: The actual font files are already included by Seeed_GFX/TFT_eSPI.h
// We just need to declare external references to use them

#ifndef FREE_FONTS_H
#define FREE_FONTS_H

#include <Fonts/GFXFF/gfxfont.h>

// FreeSans family (sans-serif)
extern const GFXfont FreeSans9pt7b;
extern const GFXfont FreeSans12pt7b;
extern const GFXfont FreeSans18pt7b;
extern const GFXfont FreeSans24pt7b;

extern const GFXfont FreeSansBold9pt7b;
extern const GFXfont FreeSansBold12pt7b;
extern const GFXfont FreeSansBold18pt7b;
extern const GFXfont FreeSansBold24pt7b;

extern const GFXfont FreeSansOblique9pt7b;
extern const GFXfont FreeSansOblique12pt7b;
extern const GFXfont FreeSansOblique18pt7b;
extern const GFXfont FreeSansOblique24pt7b;

extern const GFXfont FreeSansBoldOblique9pt7b;
extern const GFXfont FreeSansBoldOblique12pt7b;
extern const GFXfont FreeSansBoldOblique18pt7b;
extern const GFXfont FreeSansBoldOblique24pt7b;

// FreeSerif family (serif)
extern const GFXfont FreeSerif9pt7b;
extern const GFXfont FreeSerif12pt7b;
extern const GFXfont FreeSerif18pt7b;
extern const GFXfont FreeSerif24pt7b;

extern const GFXfont FreeSerifBold9pt7b;
extern const GFXfont FreeSerifBold12pt7b;
extern const GFXfont FreeSerifBold18pt7b;
extern const GFXfont FreeSerifBold24pt7b;

extern const GFXfont FreeSerifItalic9pt7b;
extern const GFXfont FreeSerifItalic12pt7b;
extern const GFXfont FreeSerifItalic18pt7b;
extern const GFXfont FreeSerifItalic24pt7b;

extern const GFXfont FreeSerifBoldItalic9pt7b;
extern const GFXfont FreeSerifBoldItalic12pt7b;
extern const GFXfont FreeSerifBoldItalic18pt7b;
extern const GFXfont FreeSerifBoldItalic24pt7b;

// FreeMono family (monospace)
extern const GFXfont FreeMono9pt7b;
extern const GFXfont FreeMono12pt7b;
extern const GFXfont FreeMono18pt7b;
extern const GFXfont FreeMono24pt7b;

extern const GFXfont FreeMonoBold9pt7b;
extern const GFXfont FreeMonoBold12pt7b;
extern const GFXfont FreeMonoBold18pt7b;
extern const GFXfont FreeMonoBold24pt7b;

extern const GFXfont FreeMonoOblique9pt7b;
extern const GFXfont FreeMonoOblique12pt7b;
extern const GFXfont FreeMonoOblique18pt7b;
extern const GFXfont FreeMonoOblique24pt7b;

extern const GFXfont FreeMonoBoldOblique9pt7b;
extern const GFXfont FreeMonoBoldOblique12pt7b;
extern const GFXfont FreeMonoBoldOblique18pt7b;
extern const GFXfont FreeMonoBoldOblique24pt7b;

#endif // FREE_FONTS_H
