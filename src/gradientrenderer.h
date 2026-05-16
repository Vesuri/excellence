#ifndef GRADIENTRENDERER_H
#define GRADIENTRENDERER_H

#include <QImage>
#include <QList>
#include <QPoint>
#include <QRect>
#include <cmath>
#include "gradientrange.h"

// Hemisphere curve: slow near center, rapid near the boundary.
inline float sphericalT(float t) { return 1.0f - sqrtf(1.0f - t * t); }

namespace GradientRenderer {
    // Returns the palette index of the closest color to `target` by squared RGB distance.
    int nearestColorIndex(QRgb target, const QImage &image);

    // Returns the palette color index for gradient position t (0=start, 1=end),
    // applying dithering based on pixel coordinates.
    int colorIndex(float t, int pixelX, int pixelY,
                   const GradientRange *range, const QImage &image);

    // Returns the maximum distance from `from` to any corner of `rect`.
    float conformRadius(const QRect &rect, const QPoint &from);

    // FillHighlight: t = dist(center, pixel) / dist(center, polygon boundary along that ray).
    // Returns t in [0, 1]: 0 at the center, 1 at the boundary in each pixel's direction.
    float highlightTPolygon(int px, int py, const QPoint &center, const QList<QPoint> &polygon);

    // Applies a gradient fill with explicit from/to endpoints to a polygon.
    // Computes the bounding-box conformRect from the polygon itself when conform is true.
    QRect applyPolygonGradient(QImage &image, const QList<QPoint> &polygon,
                               int fillColor, const GradientRange *range,
                               GradientFillMode mode, const QPoint &gradFrom,
                               const QPoint &gradTo, bool conform);

    // Computes the t value (0..1) for a pixel based on the fill mode.
    // For Horizontal/Vertical, from/to define the start and end of the gradient span.
    // conformRect: when valid, scales the gradient to the fill bounding rect instead of
    // using absolute canvas coordinates. Pass QRect() to disable.
    float computeT(int px, int py,
                   GradientFillMode mode, const QPoint &from, const QPoint &to,
                   const QRect &conformRect = QRect());

    // Scanline polygon fill with optional gradient. Returns the changed rect.
    // gradFrom/gradTo are the gradient endpoints (caller handles hvMode logic).
    // range may be null when useGradient is false.
    // conformRect: when valid, scales the gradient to the bounding rect (see computeT).
    QRect polygonFillScanline(QImage &image, const QList<QPoint> &polygon,
                              int fillColor, bool useGradient, const GradientRange *range,
                              GradientFillMode fillMode,
                              const QPoint &gradFrom, const QPoint &gradTo,
                              const QRect &conformRect = QRect());
}

#endif // GRADIENTRENDERER_H
