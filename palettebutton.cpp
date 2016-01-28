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

void PaletteButton::setColor(const QColor &color)
{
    this->color = color;
}
