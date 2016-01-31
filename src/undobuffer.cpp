#include <QImage>
#include <cstdio>
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

void UndoBuffer::apply(QImage &image) const
{
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            image.setPixel(pos_.x() + x, pos_.y() + y, image_.pixelIndex(x, y));
        }
    }
}
