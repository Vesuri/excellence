#include <QImage>
#include "undobuffer.h"
#include "tool.h"
#include "buffer.h"

static QRgb defaultPalette[] = {
    0xff000000,
    0xffffffff,
    0xffdddddd,
    0xffbbbbbb,
    0xff999999,
    0xff666666,
    0xff444444,
    0xff222222,
    0xff995500,
    0xffff7700,
    0xffffff00,
    0xff00ff00,
    0xff00ffff,
    0xff0000ff,
    0xffaa00ff,
    0xffff0000,
    0xff005544,
    0xff008899,
    0xff00bb99,
    0xff332200,
    0xff443300,
    0xff664411,
    0xff775522,
    0xff996633,
    0xffbb7755,
    0xffcc9977,
    0xffeeaa99,
    0xffffccaa,
    0xffffccbb,
    0xffcc8888,
    0xffee88cc,
    0xff770044,
    0xffff0000,
    0xffff2200,
    0xffff5500,
    0xffff7700,
    0xffffaa00,
    0xffffdd00,
    0xffffff00,
    0xffddff00,
    0xffaaff00,
    0xff88ff00,
    0xff55ff00,
    0xff22ff00,
    0xff00ff00,
    0xff00ff33,
    0xff00ff77,
    0xff00ffbb,
    0xff00ffff,
    0xff00ccff,
    0xff0088ff,
    0xff0044ff,
    0xff0000ff,
    0xff2200ff,
    0xff5500ff,
    0xff7700ff,
    0xffaa00ff,
    0xffdd00ff,
    0xffff00ff,
    0xffff00dd,
    0xffff00aa,
    0xffff0088,
    0xffff0055,
    0xffff0033,
    0xffffaaaa,
    0xffffbbaa,
    0xffffccaa,
    0xffffddaa,
    0xffffeeaa,
    0xffffffaa,
    0xffffffaa,
    0xffffffaa,
    0xffeeffaa,
    0xffddffaa,
    0xffccffaa,
    0xffbbffaa,
    0xffaaffaa,
    0xffaaffbb,
    0xffaaffcc,
    0xffaaffee,
    0xffaaffff,
    0xffaaeeff,
    0xffaaddff,
    0xffaaccff,
    0xffaaaaff,
    0xffbbaaff,
    0xffccaaff,
    0xffddaaff,
    0xffeeaaff,
    0xffffaaff,
    0xffffaaff,
    0xffffaaff,
    0xffffaaee,
    0xffffaadd,
    0xffffaacc,
    0xffffaabb,
    0xffff5555,
    0xffff6655,
    0xffff8855,
    0xffffaa55,
    0xffffcc55,
    0xffffee55,
    0xffffff55,
    0xffeeff55,
    0xffccff55,
    0xffaaff55,
    0xff88ff55,
    0xff66ff55,
    0xff55ff55,
    0xff55ff77,
    0xff55ff88,
    0xff55ffdd,
    0xff55ffff,
    0xff55ddff,
    0xff55aaff,
    0xff5588ff,
    0xff5555ff,
    0xff6655ff,
    0xff8855ff,
    0xffaa55ff,
    0xffcc55ff,
    0xffee55ff,
    0xffff55ff,
    0xffff55ee,
    0xffff55cc,
    0xffff55aa,
    0xffff5588,
    0xffff5577,
    0xffcc0000,
    0xffcc1100,
    0xffcc4400,
    0xffcc5500,
    0xffcc8800,
    0xffccbb00,
    0xffcccc00,
    0xffbbcc00,
    0xff88cc00,
    0xff66cc00,
    0xff44cc00,
    0xff11cc00,
    0xff00cc00,
    0xff00cc22,
    0xff00cc55,
    0xff00cc99,
    0xff00cccc,
    0xff00aacc,
    0xff0066cc,
    0xff0033cc,
    0xff0000cc,
    0xff1100cc,
    0xff4400cc,
    0xff5500cc,
    0xff8800cc,
    0xffbb00cc,
    0xffcc00cc,
    0xffcc00bb,
    0xffcc0088,
    0xffcc0066,
    0xffcc0044,
    0xffcc0022,
    0xff990000,
    0xff991100,
    0xff993300,
    0xff994400,
    0xff996600,
    0xff998800,
    0xff999900,
    0xff889900,
    0xff669900,
    0xff559900,
    0xff339900,
    0xff119900,
    0xff009900,
    0xff009911,
    0xff009944,
    0xff009977,
    0xff009999,
    0xff007799,
    0xff005599,
    0xff002299,
    0xff000099,
    0xff110099,
    0xff330099,
    0xff440099,
    0xff660099,
    0xff880099,
    0xff990099,
    0xff990088,
    0xff990066,
    0xff990055,
    0xff990033,
    0xff990011,
    0xff660000,
    0xff661100,
    0xff662200,
    0xff663300,
    0xff664400,
    0xff665500,
    0xff666600,
    0xff556600,
    0xff446600,
    0xff336600,
    0xff226600,
    0xff116600,
    0xff006600,
    0xff006611,
    0xff006622,
    0xff006644,
    0xff006666,
    0xff005566,
    0xff003366,
    0xff001166,
    0xff000066,
    0xff110066,
    0xff220066,
    0xff330066,
    0xff440066,
    0xff550066,
    0xff660066,
    0xff660055,
    0xff660044,
    0xff660033,
    0xff660022,
    0xff660011,
    0xff330000,
    0xff330000,
    0xff331100,
    0xff331100,
    0xff332200,
    0xff332200,
    0xff333300,
    0xff333300,
    0xff223300,
    0xff113300,
    0xff113300,
    0xff003300,
    0xff003300,
    0xff003300,
    0xff003311,
    0xff003322,
    0xff003333,
    0xff002233,
    0xff001133,
    0xff000033,
    0xff000033,
    0xff000033,
    0xff110033,
    0xff110033,
    0xff220033,
    0xff220033,
    0xff330033,
    0xff330022,
    0xff330022,
    0xff330011,
    0xff330011,
    0xff330000
};

// MagicWB
/*
    0xff959595,
    0xff000000,
    0xffffffff,
    0xff3b67a2,
    0xff7b7b7b,
    0xffafafaf,
    0xffaa907c,
    0xffffa997
*/

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

void Buffer::setImage(const QImage &image)
{
    image_ = image;

    emit modified(image_.rect());
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

void Buffer::copyColor(unsigned fromIndex, unsigned toIndex, bool remap)
{
    if (remap) {
        for (int y = 0; y < image_.height(); y++) {
            for (int x = 0; x < image_.width(); x++) {
                if (image_.pixelIndex(x, y) == static_cast<int>(toIndex)) {
                    image_.setPixel(x, y, fromIndex);
                }
            }
        }
    } else {
        QRgb color = image_.color(fromIndex);
        image_.setColor(toIndex, color);
        emit paletteModified();
    }
    emit modified(image_.rect());
}

void Buffer::swapColors(unsigned index1, unsigned index2, bool remap)
{
    if (remap) {
        for (int y = 0; y < image_.height(); y++) {
            for (int x = 0; x < image_.width(); x++) {
                if (image_.pixelIndex(x, y) == static_cast<int>(index1)) {
                    image_.setPixel(x, y, index2);
                } else if (image_.pixelIndex(x, y) == static_cast<int>(index2)) {
                    image_.setPixel(x, y, index1);
                }
            }
        }
    } else {
        QRgb color1 = image_.color(index1);
        QRgb color2 = image_.color(index2);
        image_.setColor(index1, color2);
        image_.setColor(index2, color1);
        emit paletteModified();
    }
    emit modified(image_.rect());
}
