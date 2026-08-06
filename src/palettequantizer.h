#ifndef PALETTEQUANTIZER_H
#define PALETTEQUANTIZER_H

#include <QImage>
#include "dithermode.h"

class PaletteQuantizer
{
public:
    static QImage quantize(const QImage &source, int num_colors, DitherMode mode = DitherMode::None);
};

#endif // PALETTEQUANTIZER_H
