#include <QImage>
#include "undobuffer.h"
#include "tool.h"
#include "buffer.h"

static QRgb defaultPalette[] = {
    0xff959595,
    0xff000000,
    0xffffffff,
    0xff3b67a2,
    0xff7b7b7b,
    0xffafafaf,
    0xffaa907c,
    0xffffa997
};

Buffer::Buffer(int width, int height, int colors, QObject *parent) : QObject(parent),
    pen_(0)
{
    initialize(width, height, colors);
}

Buffer::Buffer(const QString &path, QObject *parent) : QObject(parent),
    image_(path),
    pen_(0)
{
    if (image_.isNull() || image_.format() != QImage::Format_Indexed8) {
        initialize();
    }
}

void Buffer::initialize(int width, int height, int colors)
{
    image_ = QImage(width, height, QImage::Format_Indexed8);
    QVector<QRgb> colorTable;
    for (int i = 0; i < colors; i++) {
        colorTable.append(defaultPalette[i]);
    }
    image_.setColorTable(colorTable);
    image_.fill(0);
}

QImage &Buffer::image()
{
    return image_;
}

void Buffer::clear()
{
    undoBuffers.append(new UndoBuffer(QPoint(), image_.copy()));

    image_.fill(0);

    emit modified(image_.rect());
}

void Buffer::press(const QPoint &point, const Qt::MouseButton &button)
{
    tool_->setMode(button == Qt::LeftButton ? Tool::Paint : Tool::Erase);

    preModificationImage = image_.copy();

    modifiedArea = tool_->press(point);

    emit modified(modifiedArea);
}

void Buffer::move(const QPoint &point)
{
    QRect modification = tool_->move(point);

    modifiedArea = modifiedArea.united(modification);

    emit modified(modification);
}

void Buffer::release(const QPoint &point)
{
    QRect modification = tool_->release(point);

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
    pen_ = pen;
}

Pen *Buffer::pen() const
{
    return pen_;
}

void Buffer::setTool(Tool *tool)
{
    if (tool_ != tool) {
        tool_ = tool;

        emit toolChanged(tool);
    }
}

Tool *Buffer::tool() const
{
    return tool_;
}
