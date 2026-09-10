#ifndef GRADIENTRENDERER_H
#define GRADIENTRENDERER_H

#include <QImage>
#include <QList>
#include <QPoint>
#include <QRect>
#include <cmath>
#include "gradientrange.h"

inline float sphericalT(float t) { return 1.0f - sqrtf(1.0f - t * t); }

namespace GradientRenderer {
    // Map t in [0, 1] to a dithered palette index.
    int colorIndex(float t, int pixelX, int pixelY,
                   const GradientRange *range, const QImage &image);

    float conformRadius(const QRect &rect, const QPoint &from);

    // Normalize distance to the polygon boundary along the pixel ray.
    float highlightTPolygon(int px, int py, const QPoint &center, const QList<QPoint> &polygon);

    // Apply a polygon gradient while respecting an optional stencil.
    QRect applyPolygonGradient(QImage &image, const QList<QPoint> &polygon,
                               int fillColor, const GradientRange *range,
                               GradientFillMode mode, const QPoint &gradFrom,
                               const QPoint &gradTo, bool conform, Buffer *buffer = nullptr);

    // Compute a pixel's normalized gradient position.
    float computeT(int px, int py,
                   GradientFillMode mode, const QPoint &from, const QPoint &to,
                   const QRect &conformRect = QRect());

    // Fill a polygon and return its changed area.
    QRect polygonFillScanline(QImage &image, const QList<QPoint> &polygon,
                              int fillColor, bool useGradient, const GradientRange *range,
                              GradientFillMode fillMode,
                              const QPoint &gradFrom, const QPoint &gradTo,
                              const QRect &conformRect = QRect(), Buffer *buffer = nullptr);
}

#endif // GRADIENTRENDERER_H
