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
    pen_(nullptr)
{
    initialize(width, height, colors);
}

Buffer::Buffer(const QString &path, QObject *parent) : QObject(parent),
    path_(path),
    image_(path),
    pen_(nullptr)
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

void Buffer::setPath(const QString &path)
{
    path_ = path;

    emit pathChanged(path);
}

QString Buffer::path() const
{
    return path_;
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

void Buffer::press(const QPoint &point, const Qt::MouseButton &button, const Qt::KeyboardModifiers &modifiers)
{
    tool_->setMouseButton(button);

    if (tool_->type() == Tool::Modify) {
        preModificationImage = image_.copy();

        modifiedArea = tool_->press(point, modifiers);

        emit modified(modifiedArea);
    } else {
        QRect zoomedArea = tool_->press(point, modifiers);

        emit zoomed(zoomedArea);
    }
}

void Buffer::move(const QPoint &point)
{
    QRect area = tool_->move(point);

    if (tool_->type() == Tool::Modify) {
        modifiedArea = modifiedArea.united(area);

        emit modified(area);
    } else {
        emit zoomed(area);
    }
}

void Buffer::release(const QPoint &point)
{
    QRect area = tool_->release(point);

    if (tool_->type() == Tool::Modify) {
        modifiedArea = modifiedArea.united(area);

        emit modified(area);

        undoBuffers.append(new UndoBuffer(modifiedArea.topLeft(), preModificationImage.copy(modifiedArea)));

        modifiedArea = QRect();
        preModificationImage = QImage();
    } else {
        emit zoomed(area);
    }
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
