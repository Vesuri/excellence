#include "colormap.h"

ColorMap::ColorMap() : Chunk("CMAP", QByteArray(6, 0))
{
}

ColorMap::ColorMap(const Chunk &chunk) : Chunk(chunk)
{
}

unsigned ColorMap::count() const
{
    return size() / 3;
}

QRgb ColorMap::at(unsigned index) const
{
    int offset = static_cast<int>(index * 3);
    return 0xff000000u | static_cast<unsigned>(ubyte(offset) << 16) | static_cast<unsigned>(ubyte(offset + 1) << 8) | ubyte(offset + 2);
}

QVector<QRgb> ColorMap::toVector() const
{
    QVector<QRgb> vector;
    for (unsigned i = 0; i < count(); i++) {
        vector.append(at(i));
    }
    return vector;
}
