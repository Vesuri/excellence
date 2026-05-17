#include "gradientrenderer.h"
#include <QList>
#include <algorithm>
#include <climits>
#include <cmath>
#include "algorithms.h"

namespace GradientRenderer {

int colorIndex(float t, int pixelX, int pixelY,
               const GradientRange *range, const QImage &image)
{
    if (!range || image.colorCount() == 0) return 0;
    const auto &markers = range->markers();
    if (markers.isEmpty()) return 0;
    if (markers.size() == 1) return markers.first().colorIndex;

    const bool hardEdges = range->hardEdges();
    const bool isRandom  = range->random();
    const int  ditherAmt = range->ditherAmount();

    float span    = float(markers.last().slot - markers.first().slot);
    float slotPos = markers.first().slot + t * span;

    if (isRandom && ditherAmt > 0)
        slotPos = applyRandomDither(slotPos, span, ditherAmt, pixelX, pixelY,
                                    float(markers.first().slot), float(markers.last().slot));

    if (slotPos <= markers.first().slot)
        return markers.first().colorIndex;
    if (slotPos >= markers.last().slot)
        return markers.last().colorIndex;

    for (int i = 0; i < markers.size() - 1; i++) {
        if (slotPos >= markers[i].slot && slotPos < markers[i + 1].slot) {
            if (markers[i].abrupt || hardEdges)
                return markers[i].colorIndex;

            const int endColor = markers[i + 1].effectiveIncomingColor();
            float seg_t = (slotPos - markers[i].slot) / float(markers[i + 1].slot - markers[i].slot);

            if (isRandom)
                return seg_t < 0.5f ? markers[i].colorIndex : endColor;

            // clang-format off
            static const int bayer16[16][16] = {
                {  0,128, 32,160,  8,136, 40,168,  2,130, 34,162, 10,138, 42,170},
                {192, 64,224, 96,200, 72,232,104,194, 66,226, 98,202, 74,234,106},
                { 48,176, 16,144, 56,184, 24,152, 50,178, 18,146, 58,186, 26,154},
                {240,112,208, 80,248,120,216, 88,242,114,210, 82,250,122,218, 90},
                { 12,140, 44,172,  4,132, 36,164, 14,142, 46,174,  6,134, 38,166},
                {204, 76,236,108,196, 68,228,100,206, 78,238,110,198, 70,230,102},
                { 60,188, 28,156, 52,180, 20,148, 62,190, 30,158, 54,182, 22,150},
                {252,124,220, 92,244,116,212, 84,254,126,222, 94,246,118,214, 86},
                {  3,131, 35,163, 11,139, 43,171,  1,129, 33,161,  9,137, 41,169},
                {195, 67,227, 99,203, 75,235,107,193, 65,225, 97,201, 73,233,105},
                { 51,179, 19,147, 59,187, 27,155, 49,177, 17,145, 57,185, 25,153},
                {243,115,211, 83,251,123,219, 91,241,113,209, 81,249,121,217, 89},
                { 15,143, 47,175,  7,135, 39,167, 13,141, 45,173,  5,133, 37,165},
                {207, 79,239,111,199, 71,231,103,205, 77,237,109,197, 69,229,101},
                { 63,191, 31,159, 55,183, 23,151, 61,189, 29,157, 53,181, 21,149},
                {255,127,223, 95,247,119,215, 87,253,125,221, 93,245,117,213, 85}
            };
            // clang-format on

            float threshold = (bayer16[pixelY % 16][pixelX % 16] + 0.5f) / 256.0f;
            return seg_t > threshold ? endColor : markers[i].colorIndex;
        }
    }
    return markers.last().colorIndex;
}

float highlightTPolygon(int px, int py, const QPoint &center, const QList<QPoint> &polygon)
{
    float dx = float(px - center.x());
    float dy = float(py - center.y());
    if (dx * dx + dy * dy < 0.25f) return 0.0f;

    // Ray: center + t*(dx,dy). Pixel P is at t=1.
    // Find maximum t where the ray intersects a polygon edge.
    // gradient_t = 1/maxT (0 at center, 1 at boundary in each direction).
    float maxT = 1.0f;
    const int n = polygon.size();
    for (int i = 0; i < n; i++) {
        const QPoint &v1 = polygon[i];
        const QPoint &v2 = polygon[(i + 1) % n];
        float ex = float(v2.x() - v1.x());
        float ey = float(v2.y() - v1.y());
        // Cramer's rule for: center + t*(dx,dy) = v1 + s*(ex,ey)
        // det = ex*dy - ey*dx
        float det = ex * dy - ey * dx;
        if (fabsf(det) < 1e-6f) continue;
        float cvx = float(v1.x() - center.x());
        float cvy = float(v1.y() - center.y());
        float t = (-cvx * ey + ex * cvy) / det;
        float s = (dx * cvy - dy * cvx) / det;
        if (t > 0.0f && s >= 0.0f && s <= 1.0f)
            maxT = qMax(maxT, t);
    }
    return qBound(0.0f, 1.0f / maxT, 1.0f);
}

float conformRadius(const QRect &rect, const QPoint &from)
{
    float maxDist = 1.0f;
    const QPoint corners[] = { rect.topLeft(), rect.topRight(), rect.bottomLeft(), rect.bottomRight() };
    for (const QPoint &c : corners) {
        float ddx = float(c.x() - from.x());
        float ddy = float(c.y() - from.y());
        maxDist = qMax(maxDist, sqrtf(ddx * ddx + ddy * ddy));
    }
    return maxDist;
}

float computeT(int px, int py,
               GradientFillMode mode, const QPoint &from, const QPoint &to,
               const QRect &conformRect)
{
    const bool conform = conformRect.isValid();
    switch (mode) {
    case FillHorizontal: {
        int x0 = conform ? conformRect.left()  : qMin(from.x(), to.x());
        int x1 = conform ? conformRect.right() : qMax(from.x(), to.x());
        return float(px - x0) / qMax(x1 - x0, 1);
    }
    case FillVertical: {
        int y0 = conform ? conformRect.top()    : qMin(from.y(), to.y());
        int y1 = conform ? conformRect.bottom() : qMax(from.y(), to.y());
        return float(py - y0) / qMax(y1 - y0, 1);
    }
    case FillLinear: {
        float dx = float(to.x() - from.x());
        float dy = float(to.y() - from.y());
        float lenSq = dx * dx + dy * dy;
        if (lenSq < 1.0f) return 0.0f;
        float rx = float(px - from.x());
        float ry = float(py - from.y());
        float t_raw = (rx * dx + ry * dy) / lenSq;
        if (!conform)
            return qBound(0.0f, t_raw, 1.0f);
        const QPoint corners[] = { conformRect.topLeft(), conformRect.topRight(),
                                   conformRect.bottomLeft(), conformRect.bottomRight() };
        float t_min = 1e9f, t_max = -1e9f;
        for (const QPoint &c : corners) {
            float t = (float(c.x() - from.x()) * dx + float(c.y() - from.y()) * dy) / lenSq;
            t_min = qMin(t_min, t);
            t_max = qMax(t_max, t);
        }
        float span = t_max - t_min;
        if (span < 1e-6f) return 0.0f;
        return qBound(0.0f, (t_raw - t_min) / span, 1.0f);
    }
    case FillRadial: {
        float radius = conform ? conformRadius(conformRect, from)
                               : sqrtf(float(to.x() - from.x()) * float(to.x() - from.x())
                                     + float(to.y() - from.y()) * float(to.y() - from.y()));
        if (radius < 1.0f) return 0.0f;
        float rx = float(px - from.x());
        float ry = float(py - from.y());
        return qBound(0.0f, sqrtf(rx * rx + ry * ry) / radius, 1.0f);
    }
    case FillSpherical: {
        float radius = conform ? conformRadius(conformRect, from)
                               : sqrtf(float(to.x() - from.x()) * float(to.x() - from.x())
                                     + float(to.y() - from.y()) * float(to.y() - from.y()));
        if (radius < 1.0f) return 0.0f;
        float rx = float(px - from.x());
        float ry = float(py - from.y());
        float raw = qBound(0.0f, sqrtf(rx * rx + ry * ry) / radius, 1.0f);
        // Hemisphere geometry: slow near center, rapidly accelerating toward edges,
        // matching the curvature of a sphere's surface falling away from the viewer.
        return sphericalT(raw);
    }
    case FillHighlight: {
        // Shape tools compute highlight t via highlightTPolygon/ellipse; this is a
        // linear-radial fallback for contexts where shape data is not available.
        float radius = conform ? conformRadius(conformRect, from)
                               : sqrtf(float(to.x() - from.x()) * float(to.x() - from.x())
                                     + float(to.y() - from.y()) * float(to.y() - from.y()));
        if (radius < 1.0f) return 0.0f;
        float rx = float(px - from.x());
        float ry = float(py - from.y());
        return qBound(0.0f, sqrtf(rx * rx + ry * ry) / radius, 1.0f);
    }
    default:
        return 0.0f;
    }
}

QRect polygonFillScanline(QImage &image, const QList<QPoint> &polygon,
                          int fillColor, bool useGradient, const GradientRange *range,
                          GradientFillMode fillMode,
                          const QPoint &gradFrom, const QPoint &gradTo,
                          const QRect &conformRect)
{
    if (polygon.size() < 3)
        return QRect();

    const QRect imageRect = image.rect();
    const int n = polygon.size();
    const bool conform = conformRect.isValid();
    const bool hConform = conform && fillMode == FillHorizontal;
    const bool vConform = conform && fillMode == FillVertical;

    int minY = imageRect.bottom(), maxY = imageRect.top();
    for (const QPoint &v : polygon) {
        minY = qMin(minY, v.y());
        maxY = qMax(maxY, v.y());
    }
    minY = qMax(minY, imageRect.top());
    maxY = qMin(maxY, imageRect.bottom());

    auto scanlineXS = [&](int y) {
        QList<int> xs;
        for (int i = 0; i < n; i++) {
            const QPoint &p1 = polygon[i];
            const QPoint &p2 = polygon[(i + 1) % n];
            if ((p1.y() <= y && p2.y() > y) || (p2.y() <= y && p1.y() > y))
                xs.append(p1.x() + (y - p1.y()) * (p2.x() - p1.x()) / (p2.y() - p1.y()));
        }
        std::sort(xs.begin(), xs.end());
        return xs;
    };

    // For V conform: pre-scan to collect per-column y ranges.
    QVector<int> colY0, colY1;
    if (vConform) {
        colY0.fill(INT_MAX, imageRect.width());
        colY1.fill(INT_MIN, imageRect.width());
        for (int y = minY; y <= maxY; y++) {
            const QList<int> xs = scanlineXS(y);
            for (int i = 0; i + 1 < xs.size(); i += 2) {
                int x1 = qMax(xs[i], imageRect.left());
                int x2 = qMin(xs[i + 1], imageRect.right());
                for (int x = x1; x <= x2; x++) {
                    colY0[x - imageRect.left()] = qMin(colY0[x - imageRect.left()], y);
                    colY1[x - imageRect.left()] = qMax(colY1[x - imageRect.left()], y);
                }
            }
        }
    }

    QRect changedRect;
    for (int y = minY; y <= maxY; y++) {
        const QList<int> xs = scanlineXS(y);
        for (int i = 0; i + 1 < xs.size(); i += 2) {
            int x1 = qMax(xs[i], imageRect.left());
            int x2 = qMin(xs[i + 1], imageRect.right());
            for (int x = x1; x <= x2; x++) {
                if (useGradient) {
                    float t;
                    // Highlight, and Radial/Spherical with conform: normalize per-direction
                    // to actual polygon boundary using ray-polygon intersection.
                    if (fillMode == FillHighlight
                            || (conform && gradientFillIsRadial(fillMode))) {
                        t = highlightTPolygon(x, y, gradFrom, polygon);
                        if (fillMode == FillSpherical)
                            t = sphericalT(t);
                    } else {
                        QRect pixConform = conformRect;
                        if (hConform)
                            pixConform = QRect(x1, y, x2 - x1 + 1, 1);
                        else if (vConform) {
                            int xi = x - imageRect.left();
                            if (xi >= 0 && xi < colY0.size() && colY0[xi] <= colY1[xi])
                                pixConform = QRect(x, colY0[xi], 1, colY1[xi] - colY0[xi] + 1);
                        }
                        t = computeT(x, y, fillMode, gradFrom, gradTo, pixConform);
                    }
                    int ci = colorIndex(t, x, y, range, image);
                    image.setPixel(x, y, static_cast<uint>(ci));
                } else {
                    image.setPixel(x, y, static_cast<uint>(fillColor));
                }
            }
            if (x1 <= x2)
                changedRect = changedRect.united(QRect(x1, y, x2 - x1 + 1, 1));
        }
    }
    return changedRect;
}

QRect applyPolygonGradient(QImage &image, const QList<QPoint> &polygon,
                           int fillColor, const GradientRange *range,
                           GradientFillMode mode, const QPoint &gradFrom,
                           const QPoint &gradTo, bool conform)
{
    QRect polyBbox;
    for (const QPoint &p : polygon) polyBbox = polyBbox.united(QRect(p, p));
    QRect conformRect = conform ? polyBbox : QRect();
    QRect r = polygonFillScanline(image, polygon, fillColor, true, range,
                                  mode, gradFrom, gradTo, conformRect);

    // Scanline fill may miss boundary pixels due to integer edge-intersection rounding.
    // Re-apply gradient to every polygon edge. For H/V conform, per-row/-column
    // normalization can't be replicated cheaply here, so use QRect() (absolute canvas
    // coordinates) to avoid the wrong full-bbox t-value that polyBbox would give.
    if (polygon.size() >= 3) {
        const bool hvMode = mode == FillHorizontal || mode == FillVertical;
        const bool shapeConform = mode == FillHighlight || (conform && gradientFillIsRadial(mode));
        QRect edgeConformRect = (conform && !hvMode && !shapeConform) ? polyBbox : QRect();
        auto applyGrad = [&](const QPoint &p) {
            if (!image.rect().contains(p)) return;
            float t;
            if (shapeConform) {
                t = highlightTPolygon(p.x(), p.y(), gradFrom, polygon);
                if (mode == FillSpherical)
                    t = 1.0f - sqrtf(1.0f - t * t);
            } else {
                t = computeT(p.x(), p.y(), mode, gradFrom, gradTo, edgeConformRect);
            }
            image.setPixel(p, static_cast<uint>(colorIndex(t, p.x(), p.y(), range, image)));
        };
        for (int i = 0; i < polygon.size(); i++)
            Algorithms::line(polygon[i], polygon[(i + 1) % polygon.size()], applyGrad);
    }
    return r.united(polyBbox.intersected(image.rect()));
}

} // namespace GradientRenderer
