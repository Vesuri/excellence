#ifndef GRADIENTRENDERER_H
#define GRADIENTRENDERER_H

#include <QImage>
#include <QPoint>
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
}

#endif // GRADIENTRENDERER_H
