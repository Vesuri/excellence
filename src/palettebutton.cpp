#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include "colorutils.h"
#include "palettebutton.h"

PaletteButton::PaletteButton(QWidget *parent) : QAbstractButton(parent),
    paintButtonDown(false),
    eraseButtonDown(false)
{
    setAcceptDrops(false);
    setMinimumHeight(16);
}

void PaletteButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    const QRect r = rect();
    painter.fillRect(r, color_);

    painter.setPen(contrastColor(color_));
    QFont f = painter.font();
    f.setPixelSize(9);
    painter.setFont(f);
    painter.drawText(r, Qt::AlignCenter, QString::number(paletteIndex_));

    if (isEraseColor_) {
        painter.setPen(QColor(0x33, 0x75, 0xe6));
        painter.drawRect(r.adjusted(0, 0, -1, -1));
    }
    if (isPaintColor_) {
        painter.setPen(Qt::white);
        painter.drawRect(r.adjusted(0, 0, -1, -1));
    }
}

void PaletteButton::mousePressEvent(QMouseEvent *event)
{
    QAbstractButton::mousePressEvent(event);

    if (event->button() == Qt::LeftButton) {
        paintButtonDown = true;
        dragStartPos_ = event->pos();
    } else if (event->button() == Qt::RightButton) {
        eraseButtonDown = true;
    }
}

void PaletteButton::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPos_).manhattanLength() < QApplication::startDragDistance())
        return;
    paintButtonDown = false;

    QDrag *drag = new QDrag(this);
    QMimeData *mime = new QMimeData;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << paletteIndex_;
    mime->setData("application/x-palette-index", data);

    QPixmap pm(12, 12);
    pm.fill(color_);
    drag->setPixmap(pm);
    drag->setMimeData(mime);
    drag->exec(Qt::CopyAction);
}

void PaletteButton::mouseReleaseEvent(QMouseEvent *event)
{
    QAbstractButton::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton) {
        if (paintButtonDown && contentsRect().contains(event->pos())) {
            emit paintColorSelected(paletteIndex_);
        }
        paintButtonDown = false;
    } else if (event->button() == Qt::RightButton) {
        if (eraseButtonDown && contentsRect().contains(event->pos())) {
            emit eraseColorSelected(paletteIndex_);
        }
        eraseButtonDown = false;
    }
}

unsigned PaletteButton::paletteIndex() const
{
    return paletteIndex_;
}

void PaletteButton::setPaletteIndex(unsigned paletteIndex)
{
    paletteIndex_ = paletteIndex;
}

void PaletteButton::setColor(const QColor &color)
{
    if (color_ == color) return;
    color_ = color;
    update();
}

void PaletteButton::setColorFlag(bool &member, bool value)
{
    if (member == value) return;
    member = value;
    update();
}

void PaletteButton::setIsPaintColor(bool isPaintColor)
{
    setColorFlag(isPaintColor_, isPaintColor);
}

void PaletteButton::setIsEraseColor(bool isEraseColor)
{
    setColorFlag(isEraseColor_, isEraseColor);
}
