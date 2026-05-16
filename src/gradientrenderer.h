#ifndef GRADIENTRENDERER_H
#define GRADIENTRENDERER_H

#include <QImage>
#include <QList>
#include <QPoint>
#include <QRect>
#include "gradientrange.h"

namespace GradientRenderer {
    // Returns the palette color index for gradient position t (0=start, 1=end),
    // applying dithering based on pixel coordinates.
    int colorIndex(float t, int pixelX, int pixelY,
                   const GradientRange *range, const QImage &image);

    // Computes the t value (0..1) for a pixel based on the fill mode.
    // For Horizontal/Vertical, from/to define the start and end of the gradient span.
    float computeT(int px, int py,
                   GradientFillMode mode, const QPoint &from, const QPoint &to);

    // Scanline polygon fill with optional gradient. Returns the changed rect.
    // gradFrom/gradTo are the gradient endpoints (caller handles hvMode logic).
    // range may be null when useGradient is false.
    QRect polygonFillScanline(QImage &image, const QList<QPoint> &polygon,
                              int fillColor, bool useGradient, const GradientRange *range,
                              GradientFillMode fillMode,
                              const QPoint &gradFrom, const QPoint &gradTo);
}

#endif // GRADIENTRENDERER_H
