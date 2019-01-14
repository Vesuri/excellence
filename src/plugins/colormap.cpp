#include <QByteArray>
#include <QImage>

#include "colormap.h"

ColorMap::ColorMap(const QImage &image) : Chunk("CMAP", QByteArray(image.colorCount() * 3, 0))
{
    QVector<QRgb> colorTable = image.colorTable();
    for (unsigned i = 0; i < static_cast<unsigned>(image.colorCount()); i++) {
        setUbyte(i * 3, static_cast<unsigned char>(colorTable.at(static_cast<int>(i)) >> 16));
        setUbyte(i * 3 + 1, static_cast<unsigned char>(colorTable.at(static_cast<int>(i)) >> 8));
        setUbyte(i * 3 + 2, static_cast<unsigned char>(colorTable.at(static_cast<int>(i))));
    }
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
    return 0xff000000u | static_cast<unsigned>(ubyte(index * 3) << 16) | static_cast<unsigned>(ubyte(index * 3 + 1) << 8) | ubyte(index * 3 + 2);
}

QVector<QRgb> ColorMap::toVector() const
{
    QVector<QRgb> vector;
    for (unsigned i = 0; i < count(); i++) {
        vector.append(at(i));
    }
    return vector;
}
