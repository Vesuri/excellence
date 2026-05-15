#include <climits>
#include <cstdlib>
#include <QRect>
#include <QVector>
#include "pen.h"
#include "tool.h"

Pen::Pen(QObject *parent) : QObject(parent)
{
}

// ── Shared per-pixel helpers ──────────────────────────────────────────────────

static int findNearestPalette(const QVector<QRgb> &palette, QRgb color)
{
    int bestIdx = 0, bestDist = INT_MAX;
    for (int i = 0; i < palette.size(); i++) {
        int dr = qRed(color)   - qRed(palette[i]);
        int dg = qGreen(color) - qGreen(palette[i]);
        int db = qBlue(color)  - qBlue(palette[i]);
        int dist = dr*dr + dg*dg + db*db;
        if (dist < bestDist) { bestDist = dist; bestIdx = i; }
    }
    return bestIdx;
}

static const int kBayer4x4[4][4] = {
    { 0,  8,  2, 10},
    {12,  4, 14,  6},
    { 3, 11,  1,  9},
    {15,  7, 13,  5}
};

static void smearPixel(const QPoint &p, Buffer *buffer, unsigned fallbackColor)
{
    QRect imageRect = buffer->image().rect();
    if (!imageRect.contains(p)) return;
    QPoint dir = buffer->smearDirection();
    QPoint src = p - dir;
    if (dir.isNull() || !imageRect.contains(src))
        buffer->image().setPixel(p, fallbackColor);
    else
        buffer->image().setPixel(p, static_cast<uint>(buffer->image().pixelIndex(src)));
}

static void smoothPixel(const QPoint &p, Buffer *buffer)
{
    const QImage &ref = buffer->referenceImage();
    QRect imageRect = ref.rect();
    if (!imageRect.contains(p)) return;
    const QVector<QRgb> palette = buffer->image().colorTable();
    int centerIdx = ref.pixelIndex(p.x(), p.y());
    QRgb centerColor = ref.color(centerIdx);
    const int nx[] = {0, 0, -1, 1};
    const int ny[] = {-1, 1, 0, 0};
    int rSum = 0, gSum = 0, bSum = 0, count = 0;
    for (int i = 0; i < 4; i++) {
        QPoint n(p.x() + nx[i], p.y() + ny[i]);
        if (!imageRect.contains(n)) continue;
        int nIdx = ref.pixelIndex(n.x(), n.y());
        if (nIdx == centerIdx) continue;  // same color, not a boundary
        QRgb nc = ref.color(nIdx);
        rSum += qRed(nc); gSum += qGreen(nc); bSum += qBlue(nc); count++;
    }
    if (count == 0) return;  // no color boundary, leave pixel unchanged
    // Find the palette color closest to the midpoint between this pixel and its
    // differently-colored neighbours. Only apply if a genuinely intermediate
    // palette entry exists (i.e. the result differs from the original colour).
    QRgb neighborAvg = qRgb(rSum / count, gSum / count, bSum / count);
    QRgb mid = qRgb((qRed(centerColor)   + qRed(neighborAvg))   / 2,
                    (qGreen(centerColor) + qGreen(neighborAvg)) / 2,
                    (qBlue(centerColor)  + qBlue(neighborAvg))  / 2);
    int nearest = findNearestPalette(palette, mid);
    if (nearest != centerIdx)
        buffer->image().setPixel(p, static_cast<uint>(nearest));
}

static void rangePixel(const QPoint &p, Buffer *buffer, bool isErase)
{
    QRect imageRect = buffer->image().rect();
    if (!imageRect.contains(p)) return;
    QVector<int> grad = buffer->gradientColors();
    if (grad.size() < 2) return;
    int curIdx = buffer->image().pixelIndex(p);
    int pos = grad.indexOf(curIdx);
    if (pos < 0) return;
    if (!isErase && pos < grad.size() - 1)
        buffer->image().setPixel(p, static_cast<uint>(grad[pos + 1]));
    else if (isErase && pos > 0)
        buffer->image().setPixel(p, static_cast<uint>(grad[pos - 1]));
}

static void averageSmearPixel(const QPoint &p, Buffer *buffer)
{
    QRect imageRect = buffer->image().rect();
    if (!imageRect.contains(p)) return;
    QPoint dir = buffer->smearDirection();
    QVector<int> grad = buffer->gradientColors();
    const QVector<QRgb> palette = buffer->image().colorTable();
    auto findNearest = [&](QRgb color) -> int {
        if (grad.isEmpty()) return findNearestPalette(palette, color);
        int bestIdx = grad[0], bestDist = INT_MAX;
        for (int gi : grad) {
            if (gi >= palette.size()) continue;
            int dr = qRed(color)   - qRed(palette[gi]);
            int dg = qGreen(color) - qGreen(palette[gi]);
            int db = qBlue(color)  - qBlue(palette[gi]);
            int dist = dr*dr + dg*dg + db*db;
            if (dist < bestDist) { bestDist = dist; bestIdx = gi; }
        }
        return bestIdx;
    };
    QPoint src = p - dir;
    QRgb srcColor = (imageRect.contains(src) && !dir.isNull())
                    ? buffer->image().color(buffer->image().pixelIndex(src))
                    : buffer->image().color(buffer->image().pixelIndex(p));
    QRgb dstColor = buffer->image().color(buffer->image().pixelIndex(p));
    QRgb avg = qRgb((qRed(srcColor)   + qRed(dstColor))   / 2,
                    (qGreen(srcColor) + qGreen(dstColor)) / 2,
                    (qBlue(srcColor)  + qBlue(dstColor))  / 2);
    buffer->image().setPixel(p, static_cast<uint>(findNearest(avg)));
}

static void colorEffectPixel(const QPoint &p, Buffer *buffer, unsigned baseColor, Buffer::PaintMode mode)
{
    QRect imageRect = buffer->image().rect();
    if (!imageRect.contains(p)) return;
    const QVector<QRgb> palette = buffer->image().colorTable();
    QRgb paintRgb = (baseColor < static_cast<unsigned>(palette.size())) ? palette[static_cast<int>(baseColor)] : 0;
    QRgb canvasRgb = buffer->image().color(buffer->image().pixelIndex(p));
    QRgb target = canvasRgb;
    // Brilliance-compatible luma: (R*77 + G*150 + B*29) >> 8
    auto brillLuma = [](QRgb c) { return (qRed(c)*77 + qGreen(c)*150 + qBlue(c)*29) >> 8; };
    // Scale each paint channel by brightness ratio, clamping at 255
    auto scaleRgb = [](QRgb paint, int num, int denom) -> QRgb {
        return qRgb(qMin(255, qRed(paint)   * num / denom),
                    qMin(255, qGreen(paint) * num / denom),
                    qMin(255, qBlue(paint)  * num / denom));
    };
    switch (mode) {
    case Buffer::Tint: {
        // Scale paint by canvas_luma / paint_luma (Brilliance Rec.601-like)
        int lc = brillLuma(canvasRgb);
        int lp = qMax(1, brillLuma(paintRgb));
        target = scaleRgb(paintRgb, lc, lp);
        break;
    }
    case Buffer::Colorize: {
        // No effect on achromatic (grey/black/white) canvas
        int cv = qMax(qRed(canvasRgb), qMax(qGreen(canvasRgb), qBlue(canvasRgb)));
        if (cv == 0) return;
        if (qAbs(qRed(canvasRgb) - qGreen(canvasRgb)) < 8 &&
            qAbs(qGreen(canvasRgb) - qBlue(canvasRgb)) < 8) return;
        // Scale paint by canvas_V / paint_V (HSV value = max channel)
        int pv = qMax(1, qMax(qRed(paintRgb), qMax(qGreen(paintRgb), qBlue(paintRgb))));
        target = scaleRgb(paintRgb, cv, pv);
        break;
    }
    case Buffer::Brighten:
    case Buffer::Darken: {
        // Shift perceptual luminance by the amount, then find nearest palette colour to the
        // resulting grey. This matches Brilliance: both #000 and #00f at 50% → #999.
        const QImage &ref = buffer->referenceImage();
        QRgb refRgb = ref.color(ref.pixelIndex(p));
        double Y = 0.2126 * qRed(refRgb) + 0.7152 * qGreen(refRgb) + 0.0722 * qBlue(refRgb);
        double step = buffer->drawModeAmount() * 255.0 / 100.0;
        int gray = qRound((mode == Buffer::Brighten) ? qMin(255.0, Y + step) : qMax(0.0, Y - step));
        target = qRgb(gray, gray, gray);
        break;
    }
    case Buffer::Mix:
        target = qRgb((qRed(canvasRgb)   + qRed(paintRgb))   / 2,
                      (qGreen(canvasRgb) + qGreen(paintRgb)) / 2,
                      (qBlue(canvasRgb)  + qBlue(paintRgb))  / 2);
        break;
    case Buffer::Negative:
        target = qRgb(255 - qRed(canvasRgb), 255 - qGreen(canvasRgb), 255 - qBlue(canvasRgb));
        break;
    default: return;
    }
    buffer->image().setPixel(p, static_cast<uint>(findNearestPalette(palette, target)));
}

static void ditherPixel(const QPoint &p, Buffer *buffer, unsigned fgColor, unsigned bgColor, bool useBg)
{
    QRect imageRect = buffer->image().rect();
    if (!imageRect.contains(p)) return;
    int amount = buffer->drawModeAmount();
    int threshold = kBayer4x4[p.y() % 4][p.x() % 4] * 100 / 15;
    if (amount > threshold)
        buffer->image().setPixel(p, fgColor);
    else if (useBg)
        buffer->image().setPixel(p, bgColor);
}

static void transparentPixel(const QPoint &p, Buffer *buffer, unsigned paintColor)
{
    QRect imageRect = buffer->image().rect();
    if (!imageRect.contains(p)) return;
    const QVector<QRgb> palette = buffer->image().colorTable();
    QRgb paintRgb = (paintColor < static_cast<unsigned>(palette.size())) ? palette[static_cast<int>(paintColor)] : 0;
    int opacity = buffer->drawModeAmount();
    QRgb canvasRgb = buffer->image().color(buffer->image().pixelIndex(p));
    QRgb blended = qRgb(
        (qRed(paintRgb)   * opacity + qRed(canvasRgb)   * (100 - opacity)) / 100,
        (qGreen(paintRgb) * opacity + qGreen(canvasRgb) * (100 - opacity)) / 100,
        (qBlue(paintRgb)  * opacity + qBlue(canvasRgb)  * (100 - opacity)) / 100);
    buffer->image().setPixel(p, static_cast<uint>(findNearestPalette(palette, blended)));
}

// ── Public methods ────────────────────────────────────────────────────────────

unsigned Pen::resolveDrawColor(Buffer *buffer, Buffer::PaintMode &mode,
                                bool &isErase, unsigned paintColor, unsigned eraseColor)
{
    Tool *tool = buffer->tool();
    bool hovering = tool && tool->mouseButton() == Qt::NoButton;

    if (mode == Buffer::Cycle) {
        mode = Buffer::Normal;
        if (hovering) return paintColor;
        unsigned color = static_cast<unsigned>(buffer->nextCycleColor(isErase));
        isErase = false;
        return color;
    }
    if (mode == Buffer::Random) {
        mode = Buffer::Normal;
        if (hovering) return paintColor;
        QVector<int> grad = buffer->gradientColors();
        unsigned base = isErase ? eraseColor : paintColor;
        isErase = false;
        return grad.isEmpty() ? base : static_cast<unsigned>(grad[rand() % grad.size()]);
    }
    return paintColor;
}

// ── Public dispatcher ─────────────────────────────────────────────────────────

void Pen::applyPixelMode(const QPoint &p, Buffer *buffer,
                         Buffer::PaintMode mode, bool isErase,
                         unsigned paintColor, unsigned eraseColor)
{
    unsigned paintC = isErase ? eraseColor : paintColor;
    unsigned eraseC = isErase ? paintColor : eraseColor;
    switch (mode) {
    case Buffer::Normal:
    case Buffer::Replace:
        if (buffer->image().rect().contains(p))
            buffer->image().setPixel(p, paintC);
        break;
    case Buffer::Smear:        smearPixel(p, buffer, paintC); break;
    case Buffer::Smooth:       smoothPixel(p, buffer); break;
    case Buffer::Range:        rangePixel(p, buffer, isErase); break;
    case Buffer::AverageSmear: averageSmearPixel(p, buffer); break;
    case Buffer::Tint:
    case Buffer::Colorize:
    case Buffer::Brighten:
    case Buffer::Darken:
    case Buffer::Mix:
    case Buffer::Negative:
        if (isErase) {
            if (buffer->image().rect().contains(p))
                buffer->image().setPixel(p, paintC);
        } else {
            colorEffectPixel(p, buffer, paintC, mode);
        }
        break;
    case Buffer::Dither1:      ditherPixel(p, buffer, paintC, eraseC, false); break;
    case Buffer::Dither2:      ditherPixel(p, buffer, paintC, eraseC, true); break;
    case Buffer::Transparent:  transparentPixel(p, buffer, paintC); break;
    default: break; // BrushMode, Cycle, Random: callers handle these before calling applyPixelMode
    }
}
