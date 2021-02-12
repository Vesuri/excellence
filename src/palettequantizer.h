#ifndef PALETTEQUANTIZER_H
#define PALETTEQUANTIZER_H

#include <QImage>

class PaletteQuantizer
{
public:
    static QImage quantize(const QImage &source, int num_colors);
};

#endif // PALETTEQUANTIZER_H
