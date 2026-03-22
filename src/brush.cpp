#include <QRect>
#include <QImage>
#include <climits>
#include "buffer.h"
#include "brush.h"

Brush::Brush(const QImage &image, int transparentIndex, QObject *parent) : Pen(parent),
    image_(image),
    transparentIndex_(transparentIndex),
    handleOffset_(image.width() / 2, image.height() / 2)
{
}

QPoint Brush::handleOffset() const
{
    return handleOffset_;
}

void Brush::setHandleOffset(const QPoint &offset)
{
    handleOffset_ = offset;
}

QRect Brush::paint(const QPoint &point, Buffer *buffer) const
{
    QPoint origin = point - handleOffset_;
    QRect imageRect = buffer->image().rect();
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            int idx = image_.pixelIndex(x, y);
            if (idx == transparentIndex_)
                continue;
            QPoint p(origin.x() + x, origin.y() + y);
            if (imageRect.contains(p))
                buffer->image().setPixel(p, static_cast<uint>(idx));
        }
    }

    return image_.rect().translated(origin).intersected(imageRect);
}

QRect Brush::erase(const QPoint &point, Buffer *buffer) const
{
    QPoint origin = point - handleOffset_;
    QRect imageRect = buffer->image().rect();
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            if (image_.pixelIndex(x, y) == transparentIndex_)
                continue;
            QPoint p(origin.x() + x, origin.y() + y);
            if (imageRect.contains(p))
                buffer->image().setPixel(p, 0);
        }
    }

    return image_.rect().translated(origin).intersected(imageRect);
}

QRect Brush::rect(const QPoint &point) const
{
    return image_.rect().translated(point - handleOffset_);
}

const QImage &Brush::image() const
{
    return image_;
}

int Brush::transparentIndex() const
{
    return transparentIndex_;
}

void Brush::setTransparentIndex(int index)
{
    transparentIndex_ = index;
}

void Brush::remap(const QVector<QRgb> &palette)
{
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            QRgb color = image_.color(image_.pixelIndex(x, y));
            int bestIdx = 0, bestDist = INT_MAX;
            for (int i = 0; i < palette.size(); i++) {
                int dr = qRed(color)   - qRed(palette[i]);
                int dg = qGreen(color) - qGreen(palette[i]);
                int db = qBlue(color)  - qBlue(palette[i]);
                int dist = dr*dr + dg*dg + db*db;
                if (dist < bestDist) { bestDist = dist; bestIdx = i; }
            }
            image_.setPixel(x, y, static_cast<uint>(bestIdx));
        }
    }
}

void Brush::replaceColor(int fromIndex, int toIndex)
{
    for (int y = 0; y < image_.height(); y++)
        for (int x = 0; x < image_.width(); x++)
            if (image_.pixelIndex(x, y) == fromIndex)
                image_.setPixel(x, y, static_cast<uint>(toIndex));
}

void Brush::detectBackground()
{
    if (image_.width() < 1 || image_.height() < 1)
        return;
    int tl = image_.pixelIndex(0, 0);
    int tr = image_.pixelIndex(image_.width() - 1, 0);
    int bl = image_.pixelIndex(0, image_.height() - 1);
    int br = image_.pixelIndex(image_.width() - 1, image_.height() - 1);
    if (tl == tr && tl == bl && tl == br)
        transparentIndex_ = tl;
}
