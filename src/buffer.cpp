#include <QImage>
#include "buffer.h"

Buffer::Buffer(int width, int height, int colors, QObject *parent) : QObject(parent),
    image_(new QImage(width, height, QImage::Format_Indexed8)),
    palette_(new QVector<QRgb>)
{
    palette_->append(0xff959595);
    palette_->append(0xff000000);
    palette_->append(0xffffffff);
    palette_->append(0xff3b67a2);
    palette_->append(0xff7b7b7b);
    palette_->append(0xffafafaf);
    palette_->append(0xffaa907c);
    palette_->append(0xffffa997);
    image_->setColorTable(*palette_);

    clear();
}

QImage *Buffer::image() const
{
    return image_;
}

QVector<QRgb> *Buffer::palette() const
{
    return palette_;
}

void Buffer::clear()
{
    image_->fill(0);
}
