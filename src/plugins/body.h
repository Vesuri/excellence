#ifndef BODY_H
#define BODY_H

#include <QImage>

#include "chunk.h"

class BitmapHeader;
class ColorMap;
class CommodoreAmiga;

class Body : public Chunk
{
public:
    Body(const QImage &image);
    Body(const Chunk &chunk);

    QImage toImage(const BitmapHeader &bitmapHeader, const ColorMap &colorMap, const CommodoreAmiga &commodoreAmiga) const;
};

#endif // BODY_H
