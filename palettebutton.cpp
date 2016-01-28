#include <QPaintEvent>
#include <QPainter>
#include "palettebutton.h"

PaletteButton::PaletteButton(QWidget *parent) : QAbstractButton(parent)
{

}

void PaletteButton::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    painter.fillRect(event->rect(), color);
    painter.end();
}

int PaletteButton::paletteIndex() const
{
    return paletteIndex_;
}

void PaletteButton::setPaletteIndex(int paletteIndex)
{
    paletteIndex_ = paletteIndex;
}

void PaletteButton::setColor(const QColor &color)
{
    this->color = color;
}
