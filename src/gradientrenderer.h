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

    // Computes the t value (0..1) for a pixel based on the fill mode and
    // the drag start/end points. For Horizontal/Vertical the from/to are ignored.
    float computeT(int px, int py, int imageW, int imageH,
                   GradientFillMode mode, const QPoint &from, const QPoint &to);
}

#endif // GRADIENTRENDERER_H
