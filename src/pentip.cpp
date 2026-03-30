#include <climits>
#include <cstdlib>
#include <QColor>
#include <QRect>
#include <QImage>
#include "buffer.h"
#include "pentip.h"

PenTip::PenTip(QObject *parent) : Pen(parent),
    paintColor_(1),
    eraseColor_(0),
    width_(1),
    height_(1),
    shape_(Circle)
{
}

void PenTip::setPaintColor(unsigned paintColor) { paintColor_ = paintColor; }
void PenTip::setEraseColor(unsigned eraseColor) { eraseColor_ = eraseColor; }

int PenTip::size() const { return width_; }
int PenTip::width() const { return width_; }
int PenTip::height() const { return height_; }
PenTip::Shape PenTip::shape() const { return shape_; }

void PenTip::setSize(int w, int h)
{
    width_ = qMax(1, w);
    height_ = qMax(1, h);
}

void PenTip::setShape(Shape shape) { shape_ = shape; }

bool PenTip::inTip(int dx, int dy, int hw, int hh) const
{
    Q_UNUSED(hh);
    switch (shape_) {
    case Square: return true;
    default:     return dx * dx + dy * dy <= hw * hw + hw / 2;
    }
}

void PenTip::applyColor(const QPoint &point, Buffer *buffer, unsigned color) const
{
    if (width_ == 1 && height_ == 1) {
        if (buffer->image().rect().contains(point))
            buffer->image().setPixel(point, color);
        return;
    }
    int hw = width_ / 2, hh = height_ / 2;
    QRect imageRect = buffer->image().rect();
    for (int dy = -hh; dy <= hh; dy++) {
        for (int dx = -hw; dx <= hw; dx++) {
            if (inTip(dx, dy, hw, hh)) {
                QPoint p(point.x() + dx, point.y() + dy);
                if (imageRect.contains(p))
                    buffer->image().setPixel(p, color);
            }
        }
    }
}

void PenTip::applySmear(const QPoint &point, Buffer *buffer, unsigned fallbackColor) const
{
    QPoint dir = buffer->smearDirection();
    QRect imageRect = buffer->image().rect();
    auto doPixel = [&](const QPoint &p) {
        if (!imageRect.contains(p)) return;
        QPoint src = p - dir;
        if (dir.isNull() || !imageRect.contains(src))
            buffer->image().setPixel(p, fallbackColor);
        else
            buffer->image().setPixel(p, static_cast<uint>(buffer->image().pixelIndex(src)));
    };
    if (width_ == 1 && height_ == 1) { doPixel(point); return; }
    int hw = width_ / 2, hh = height_ / 2;
    for (int dy = -hh; dy <= hh; dy++)
        for (int dx = -hw; dx <= hw; dx++)
            if (inTip(dx, dy, hw, hh))
                doPixel(QPoint(point.x()+dx, point.y()+dy));
}

void PenTip::applySmooth(const QPoint &point, Buffer *buffer) const
{
    QRect imageRect = buffer->image().rect();
    const QVector<QRgb> palette = buffer->image().colorTable();
    auto findNearest = [&](QRgb color) -> int {
        int bestIdx = 0, bestDist = INT_MAX;
        for (int i = 0; i < palette.size(); i++) {
            int dr = qRed(color) - qRed(palette[i]);
            int dg = qGreen(color) - qGreen(palette[i]);
            int db = qBlue(color) - qBlue(palette[i]);
            int dist = dr*dr + dg*dg + db*db;
            if (dist < bestDist) { bestDist = dist; bestIdx = i; }
        }
        return bestIdx;
    };
    auto doPixel = [&](const QPoint &p) {
        if (!imageRect.contains(p)) return;
        int rSum = 0, gSum = 0, bSum = 0, count = 0;
        const int nx[] = {0, 0, -1, 1};
        const int ny[] = {-1, 1, 0, 0};
        for (int i = 0; i < 4; i++) {
            QPoint n(p.x() + nx[i], p.y() + ny[i]);
            if (imageRect.contains(n)) {
                QRgb c = buffer->image().color(buffer->image().pixelIndex(n));
                rSum += qRed(c); gSum += qGreen(c); bSum += qBlue(c); count++;
            }
        }
        if (count > 0) {
            QRgb avg = qRgb(rSum/count, gSum/count, bSum/count);
            buffer->image().setPixel(p, static_cast<uint>(findNearest(avg)));
        }
    };
    if (width_ == 1 && height_ == 1) { doPixel(point); return; }
    int hw = width_ / 2, hh = height_ / 2;
    for (int dy = -hh; dy <= hh; dy++)
        for (int dx = -hw; dx <= hw; dx++)
            if (inTip(dx, dy, hw, hh))
                doPixel(QPoint(point.x()+dx, point.y()+dy));
}

void PenTip::applyRange(const QPoint &point, Buffer *buffer, bool isErase) const
{
    QVector<int> grad = buffer->gradientColors();
    if (grad.size() < 2) return;
    QRect imageRect = buffer->image().rect();
    auto doPixel = [&](const QPoint &p) {
        if (!imageRect.contains(p)) return;
        int curIdx = buffer->image().pixelIndex(p);
        int pos = grad.indexOf(curIdx);
        if (pos < 0) return;
        if (!isErase && pos < grad.size() - 1)
            buffer->image().setPixel(p, static_cast<uint>(grad[pos + 1]));
        else if (isErase && pos > 0)
            buffer->image().setPixel(p, static_cast<uint>(grad[pos - 1]));
    };
    if (width_ == 1 && height_ == 1) { doPixel(point); return; }
    int hw = width_ / 2, hh = height_ / 2;
    for (int dy = -hh; dy <= hh; dy++)
        for (int dx = -hw; dx <= hw; dx++)
            if (inTip(dx, dy, hw, hh))
                doPixel(QPoint(point.x()+dx, point.y()+dy));
}

void PenTip::applyAverageSmear(const QPoint &point, Buffer *buffer) const
{
    QPoint dir = buffer->smearDirection();
    QVector<int> grad = buffer->gradientColors();
    QRect imageRect = buffer->image().rect();
    const QVector<QRgb> palette = buffer->image().colorTable();
    auto findNearest = [&](QRgb color) -> int {
        if (grad.isEmpty()) return 0;
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
    auto doPixel = [&](const QPoint &p) {
        if (!imageRect.contains(p)) return;
        QPoint src = p - dir;
        QRgb srcColor = (imageRect.contains(src) && !dir.isNull())
                        ? buffer->image().color(buffer->image().pixelIndex(src))
                        : buffer->image().color(buffer->image().pixelIndex(p));
        QRgb dstColor = buffer->image().color(buffer->image().pixelIndex(p));
        QRgb avg = qRgb((qRed(srcColor)   + qRed(dstColor))   / 2,
                        (qGreen(srcColor) + qGreen(dstColor)) / 2,
                        (qBlue(srcColor)  + qBlue(dstColor))  / 2);
        buffer->image().setPixel(p, static_cast<uint>(findNearest(avg)));
    };
    if (width_ == 1 && height_ == 1) { doPixel(point); return; }
    int hw = width_ / 2, hh = height_ / 2;
    for (int dy = -hh; dy <= hh; dy++)
        for (int dx = -hw; dx <= hw; dx++)
            if (inTip(dx, dy, hw, hh))
                doPixel(QPoint(point.x()+dx, point.y()+dy));
}

void PenTip::applyCycleRandom(const QPoint &point, Buffer *buffer, bool isErase, bool isRandom) const
{
    int color;
    if (isRandom) {
        QVector<int> grad = buffer->gradientColors();
        color = grad.isEmpty() ? static_cast<int>(isErase ? buffer->eraseColor() : buffer->paintColor())
                               : grad[rand() % grad.size()];
    } else {
        color = buffer->nextCycleColor(isErase);
    }
    applyColor(point, buffer, static_cast<unsigned>(color));
}

static const int kBayer4x4[4][4] = {
    { 0,  8,  2, 10},
    {12,  4, 14,  6},
    { 3, 11,  1,  9},
    {15,  7, 13,  5}
};

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

void PenTip::applyColorEffect(const QPoint &point, Buffer *buffer, unsigned baseColor, Buffer::PaintMode mode) const
{
    QRect imageRect = buffer->image().rect();
    const QVector<QRgb> palette = buffer->image().colorTable();
    QRgb paintRgb = (baseColor < static_cast<unsigned>(palette.size())) ? palette[static_cast<int>(baseColor)] : 0;
    QColor paintHSV = QColor(paintRgb).toHsv();
    const int kStep = qMax(1, buffer->drawModeAmount() * 255 / 100);

    auto doPixel = [&](const QPoint &p) {
        if (!imageRect.contains(p)) return;
        QRgb canvasRgb = buffer->image().color(buffer->image().pixelIndex(p));
        QColor canvasHSV = QColor(canvasRgb).toHsv();
        QRgb target = canvasRgb;

        switch (mode) {
        case Buffer::Tint:
            target = QColor::fromHsv(paintHSV.hsvHue(), paintHSV.hsvSaturation(),
                                     canvasHSV.value()).rgb();
            break;
        case Buffer::Colorize:
            if (canvasHSV.hsvSaturation() < 32) return;
            target = QColor::fromHsv(paintHSV.hsvHue(), paintHSV.hsvSaturation(),
                                     canvasHSV.value()).rgb();
            break;
        case Buffer::Brighten:
            target = QColor::fromHsv(canvasHSV.hsvHue(), canvasHSV.hsvSaturation(),
                                     qMin(255, canvasHSV.value() + kStep)).rgb();
            break;
        case Buffer::Darken:
            target = QColor::fromHsv(canvasHSV.hsvHue(), canvasHSV.hsvSaturation(),
                                     qMax(0, canvasHSV.value() - kStep)).rgb();
            break;
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
    };

    if (width_ == 1 && height_ == 1) { doPixel(point); return; }
    int hw = width_ / 2, hh = height_ / 2;
    for (int dy = -hh; dy <= hh; dy++)
        for (int dx = -hw; dx <= hw; dx++)
            if (inTip(dx, dy, hw, hh))
                doPixel(QPoint(point.x()+dx, point.y()+dy));
}

void PenTip::applyDither(const QPoint &point, Buffer *buffer, unsigned fgColor, unsigned bgColor, bool useBg) const
{
    int amount = buffer->drawModeAmount(); // 0-100
    QRect imageRect = buffer->image().rect();
    auto doPixel = [&](const QPoint &p) {
        if (!imageRect.contains(p)) return;
        int threshold = kBayer4x4[p.y() % 4][p.x() % 4] * 100 / 15;
        if (amount > threshold)
            buffer->image().setPixel(p, fgColor);
        else if (useBg)
            buffer->image().setPixel(p, bgColor);
    };
    if (width_ == 1 && height_ == 1) { doPixel(point); return; }
    int hw = width_ / 2, hh = height_ / 2;
    for (int dy = -hh; dy <= hh; dy++)
        for (int dx = -hw; dx <= hw; dx++)
            if (inTip(dx, dy, hw, hh))
                doPixel(QPoint(point.x()+dx, point.y()+dy));
}

void PenTip::applyTransparent(const QPoint &point, Buffer *buffer, unsigned paintColor) const
{
    QRect imageRect = buffer->image().rect();
    const QVector<QRgb> palette = buffer->image().colorTable();
    QRgb paintRgb = (paintColor < static_cast<unsigned>(palette.size())) ? palette[static_cast<int>(paintColor)] : 0;
    int opacity = buffer->drawModeAmount(); // 0-100
    auto doPixel = [&](const QPoint &p) {
        if (!imageRect.contains(p)) return;
        QRgb canvasRgb = buffer->image().color(buffer->image().pixelIndex(p));
        QRgb blended = qRgb(
            (qRed(paintRgb)   * opacity + qRed(canvasRgb)   * (100 - opacity)) / 100,
            (qGreen(paintRgb) * opacity + qGreen(canvasRgb) * (100 - opacity)) / 100,
            (qBlue(paintRgb)  * opacity + qBlue(canvasRgb)  * (100 - opacity)) / 100);
        buffer->image().setPixel(p, static_cast<uint>(findNearestPalette(palette, blended)));
    };
    if (width_ == 1 && height_ == 1) { doPixel(point); return; }
    int hw = width_ / 2, hh = height_ / 2;
    for (int dy = -hh; dy <= hh; dy++)
        for (int dx = -hw; dx <= hw; dx++)
            if (inTip(dx, dy, hw, hh))
                doPixel(QPoint(point.x()+dx, point.y()+dy));
}

void PenTip::applyBrushMode(const QPoint &point, Buffer *buffer) const
{
    const QImage &stamp = buffer->brushStamp();
    if (stamp.isNull()) {
        applyColor(point, buffer, paintColor_);
        return;
    }
    const int bw = stamp.width(), bh = stamp.height();
    const int transparent = buffer->brushTransparentIndex();
    QRect imageRect = buffer->image().rect();
    auto doPixel = [&](const QPoint &p) {
        if (!imageRect.contains(p)) return;
        int bx = ((p.x() % bw) + bw) % bw;
        int by = ((p.y() % bh) + bh) % bh;
        int ci = stamp.pixelIndex(bx, by);
        if (ci == transparent) return;
        buffer->image().setPixel(p, static_cast<uint>(ci));
    };
    if (width_ == 1 && height_ == 1) { doPixel(point); return; }
    int hw = width_ / 2, hh = height_ / 2;
    for (int dy = -hh; dy <= hh; dy++)
        for (int dx = -hw; dx <= hw; dx++)
            if (inTip(dx, dy, hw, hh))
                doPixel(QPoint(point.x() + dx, point.y() + dy));
}

void PenTip::applyPrimary(const QPoint &point, Buffer *buffer, bool isErase) const
{
    unsigned paintC = isErase ? eraseColor_ : paintColor_;
    unsigned eraseC = isErase ? paintColor_ : eraseColor_;
    switch (buffer->paintMode()) {
    case Buffer::Smear:        applySmear(point, buffer, paintC); break;
    case Buffer::Smooth:       applySmooth(point, buffer); break;
    case Buffer::Range:        applyRange(point, buffer, isErase); break;
    case Buffer::AverageSmear: applyAverageSmear(point, buffer); break;
    case Buffer::Cycle:        applyCycleRandom(point, buffer, isErase, false); break;
    case Buffer::Random:       applyCycleRandom(point, buffer, isErase, true); break;
    case Buffer::Tint:
    case Buffer::Colorize:
    case Buffer::Brighten:
    case Buffer::Darken:
    case Buffer::Mix:
    case Buffer::Negative:     applyColorEffect(point, buffer, paintC, buffer->paintMode()); break;
    case Buffer::Dither1:      applyDither(point, buffer, paintC, eraseC, false); break;
    case Buffer::Dither2:      applyDither(point, buffer, paintC, eraseC, true); break;
    case Buffer::Transparent:  applyTransparent(point, buffer, paintC); break;
    case Buffer::BrushMode:    applyBrushMode(point, buffer); break;
    default:                   applyColor(point, buffer, paintC); break;
    }
}

QRect PenTip::paint(const QPoint &point, Buffer *buffer) const
{
    applyPrimary(point, buffer, false);
    QRect changed = rect(point);
    int cx = buffer->mirrorCenterX(), cy = buffer->mirrorCenterY();
    if (buffer->mirrorX()) {
        QPoint mx(2 * cx - point.x(), point.y());
        applyPrimary(mx, buffer, false);
        changed = changed.united(rect(mx));
    }
    if (buffer->mirrorY()) {
        QPoint my(point.x(), 2 * cy - point.y());
        applyPrimary(my, buffer, false);
        changed = changed.united(rect(my));
    }
    if (buffer->mirrorX() && buffer->mirrorY()) {
        QPoint mxy(2 * cx - point.x(), 2 * cy - point.y());
        applyPrimary(mxy, buffer, false);
        changed = changed.united(rect(mxy));
    }
    return changed.intersected(buffer->image().rect());
}

QRect PenTip::erase(const QPoint &point, Buffer *buffer) const
{
    applyPrimary(point, buffer, true);
    QRect changed = rect(point);
    int cx = buffer->mirrorCenterX(), cy = buffer->mirrorCenterY();
    if (buffer->mirrorX()) {
        QPoint mx(2 * cx - point.x(), point.y());
        applyPrimary(mx, buffer, true);
        changed = changed.united(rect(mx));
    }
    if (buffer->mirrorY()) {
        QPoint my(point.x(), 2 * cy - point.y());
        applyPrimary(my, buffer, true);
        changed = changed.united(rect(my));
    }
    if (buffer->mirrorX() && buffer->mirrorY()) {
        QPoint mxy(2 * cx - point.x(), 2 * cy - point.y());
        applyPrimary(mxy, buffer, true);
        changed = changed.united(rect(mxy));
    }
    return changed.intersected(buffer->image().rect());
}

QRect PenTip::rect(const QPoint &point) const
{
    if (width_ == 1 && height_ == 1)
        return QRect(point, point);
    int hw = width_ / 2, hh = height_ / 2;
    return QRect(point.x() - hw, point.y() - hh, width_, height_);
}
