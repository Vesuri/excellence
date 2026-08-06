#ifndef DITHER_H
#define DITHER_H

#include <QImage>
#include <QRgb>
#include <QVector>
#include "dithermode.h"

// Maps a 32-bit RGB source image onto an indexed image using the given
// palette, applying the requested dither mode. The returned image has the
// palette set as its color table.
QImage ditherToPalette(const QImage &source, const QVector<QRgb> &palette, DitherMode mode);

#endif // DITHER_H
