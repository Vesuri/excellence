#ifndef PALETTEQUANTIZER_H
#define PALETTEQUANTIZER_H

#include <QImage>
#include "dithermode.h"
#include "palettesortmode.h"

class PaletteQuantizer
{
public:
    // outOf restricts generated palette entries to an evenly spaced RGB grid
    // with outOf total addressable colors (e.g. 4096 = 12-bit, 16 levels per
    // channel). Entries are guaranteed distinct as long as num_colors does
    // not exceed outOf; num_colors is clamped to outOf otherwise.
    static QImage quantize(const QImage &source, int num_colors, DitherMode mode = DitherMode::None, int outOf = 16777216, PaletteSortMode sortMode = PaletteSortMode::None);
};

#endif // PALETTEQUANTIZER_H
