#include <QImage>
#include <QDebug>
#include "undobuffer.h"
#include "tool.h"
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
    image_->fill(0);
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
    undoBuffers.append(new UndoBuffer(QPoint(), image_->copy()));

    image_->fill(0);

    emit modified(image_->rect());
}

void Buffer::press(const QPoint &point, Tool *tool)
{
    preModificationImage = image_->copy();

    modifiedArea = tool->press(point, *image_);

    emit modified(modifiedArea);
}

void Buffer::move(const QPoint &point, Tool *tool)
{
    QRect modification = tool->move(point, *image_);

    modifiedArea = modifiedArea.united(modification);

    emit modified(modification);
}

void Buffer::release(const QPoint &point, Tool *tool)
{
    QRect modification = tool->release(point, *image_);

    modifiedArea = modifiedArea.united(modification);

    emit modified(modification);

    undoBuffers.append(new UndoBuffer(modifiedArea.topLeft(), preModificationImage.copy(modifiedArea)));

    modifiedArea = QRect();
    preModificationImage = QImage();
}

void Buffer::undo()
{
    if (!undoBuffers.isEmpty()) {
        UndoBuffer *undoBuffer = undoBuffers.takeLast();
        undoBuffer->apply(*image_);
        emit modified(undoBuffer->rect());
        delete undoBuffer;
    }
}
