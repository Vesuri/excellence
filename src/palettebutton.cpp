#include <QPaintEvent>
#include <QPainter>
#include "palettebutton.h"

PaletteButton::PaletteButton(QWidget *parent) : QAbstractButton(parent),
    paintButtonDown(false),
    eraseButtonDown(false)
{
}

void PaletteButton::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    painter.fillRect(event->rect(), color);
    painter.end();
}

void PaletteButton::mousePressEvent(QMouseEvent *event)
{
    QAbstractButton::mousePressEvent(event);

    if (event->button() == Qt::LeftButton) {
        paintButtonDown = true;
    } else if (event->button() == Qt::RightButton) {
        eraseButtonDown = true;
    }
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
    this->color = color;
}
