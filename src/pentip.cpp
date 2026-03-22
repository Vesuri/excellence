#include <climits>
#include <QRect>
#include <QImage>
#include "buffer.h"
#include "pentip.h"

PenTip::PenTip(QObject *parent) : Pen(parent),
    paintColor_(1),
    eraseColor_(0),
    size_(1)
{
}

void PenTip::setPaintColor(unsigned paintColor) { paintColor_ = paintColor; }
void PenTip::setEraseColor(unsigned eraseColor) { eraseColor_ = eraseColor; }

int PenTip::size() const { return size_; }

void PenTip::setSize(int size)
{
    // Clamp to supported sizes: 1, 3, 5, 7
    if (size <= 1)      size_ = 1;
    else if (size <= 3) size_ = 3;
    else if (size <= 5) size_ = 5;
    else                size_ = 7;
}

void PenTip::applyColor(const QPoint &point, Buffer *buffer, unsigned color) const
{
    if (size_ == 1) {
        if (buffer->image().rect().contains(point))
            buffer->image().setPixel(point, color);
        return;
    }
    int r = size_ / 2;
    QRect imageRect = buffer->image().rect();
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx * dx + dy * dy <= r * r + r / 2) {
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
    if (size_ == 1) { doPixel(point); return; }
    int r = size_ / 2;
    for (int dy = -r; dy <= r; dy++)
        for (int dx = -r; dx <= r; dx++)
            if (dx*dx + dy*dy <= r*r + r/2)
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
    if (size_ == 1) { doPixel(point); return; }
    int r = size_ / 2;
    for (int dy = -r; dy <= r; dy++)
        for (int dx = -r; dx <= r; dx++)
            if (dx*dx + dy*dy <= r*r + r/2)
                doPixel(QPoint(point.x()+dx, point.y()+dy));
}

QRect PenTip::paint(const QPoint &point, Buffer *buffer) const
{
    switch (buffer->paintMode()) {
    case Buffer::Smear:  applySmear(point, buffer, paintColor_); break;
    case Buffer::Smooth: applySmooth(point, buffer); break;
    default:             applyColor(point, buffer, paintColor_); break;
    }
    return rect(point).intersected(buffer->image().rect());
}

QRect PenTip::erase(const QPoint &point, Buffer *buffer) const
{
    switch (buffer->paintMode()) {
    case Buffer::Smear:  applySmear(point, buffer, eraseColor_); break;
    case Buffer::Smooth: applySmooth(point, buffer); break;
    default:             applyColor(point, buffer, eraseColor_); break;
    }
    return rect(point).intersected(buffer->image().rect());
}

QRect PenTip::rect(const QPoint &point) const
{
    if (size_ == 1)
        return QRect(point, point);
    int r = size_ / 2;
    return QRect(point.x() - r, point.y() - r, size_, size_);
}
