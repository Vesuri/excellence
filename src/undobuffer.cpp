#include <QImage>
#include "buffer.h"
#include "undobuffer.h"

UndoBuffer::UndoBuffer(const QPoint &pos, const QImage &image, QObject *parent) : QObject(parent),
    pos_(pos),
    image_(image)
{
}

QPoint UndoBuffer::pos() const
{
    return pos_;
}

QImage UndoBuffer::image() const
{
    return image_;
}

QRect UndoBuffer::rect() const
{
    return QRect(pos_, image_.size());
}

void UndoBuffer::apply(Buffer *buffer) const
{
    const QRect imageRect = buffer->image().rect();
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            QPoint p(pos_.x() + x, pos_.y() + y);
            if (imageRect.contains(p))
                buffer->image().setPixel(p, static_cast<uint>(image_.pixelIndex(x, y)));
        }
    }
}
