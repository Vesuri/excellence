#ifndef PALETTEQUANTIZER_H
#define PALETTEQUANTIZER_H

class Buffer;

class PaletteQuantizer
{
public:
    static Buffer *quantize(Buffer *buffer, int colorCount);
};

#endif // PALETTEQUANTIZER_H
