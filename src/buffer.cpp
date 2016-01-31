#include <QImage>
#include "undobuffer.h"
#include "tool.h"
#include "buffer.h"

Buffer::Buffer(int width, int height, int, QObject *parent) : QObject(parent),
    image_(width, height, QImage::Format_Indexed8),
    pen_(0)
{
    palette_.append(0xff959595);
    palette_.append(0xff000000);
    palette_.append(0xffffffff);
    palette_.append(0xff3b67a2);
    palette_.append(0xff7b7b7b);
    palette_.append(0xffafafaf);
    palette_.append(0xffaa907c);
    palette_.append(0xffffa997);
    image_.setColorTable(palette_);
    image_.fill(0);
}

QImage &Buffer::image()
{
    return image_;
}

QRgb Buffer::color(unsigned int index) const
{
    return palette_.at(index);
}

unsigned Buffer::colorCount() const
{
    return palette_.count();
}

void Buffer::clear()
{
    undoBuffers.append(new UndoBuffer(QPoint(), image_.copy()));

    image_.fill(0);

    emit modified(image_.rect());
}

void Buffer::press(const QPoint &point, const Qt::MouseButton &button)
{
    tool->setMode(button == Qt::LeftButton ? Tool::Paint : Tool::Erase);

    preModificationImage = image_.copy();

    modifiedArea = tool->press(point);

    emit modified(modifiedArea);
}

void Buffer::move(const QPoint &point)
{
    QRect modification = tool->move(point);

    modifiedArea = modifiedArea.united(modification);

    emit modified(modification);
}

void Buffer::release(const QPoint &point)
{
    QRect modification = tool->release(point);

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
        undoBuffer->apply(this);
        emit modified(undoBuffer->rect());
        delete undoBuffer;
    }
}

void Buffer::setPen(Pen *pen)
{
    this->pen_ = pen;
}

Pen *Buffer::pen() const
{
    return pen_;
}

void Buffer::setTool(Tool *tool)
{
    this->tool = tool;
}
