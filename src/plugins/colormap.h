#ifndef COLORMAP_H
#define COLORMAP_H

#include <QVector>
#include <QRgb>

#include "chunk.h"

class QImage;

class ColorMap : public Chunk {
public:
    ColorMap(const QImage &image);
    ColorMap(const Chunk &chunk);

    unsigned count() const;
    QRgb at(unsigned index) const;
    QVector<QRgb> toVector() const;
};

#endif // COLORMAP_H
