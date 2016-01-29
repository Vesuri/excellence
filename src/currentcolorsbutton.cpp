#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include "currentcolorsbutton.h"

CurrentColorsButton::CurrentColorsButton(QWidget *parent) : QAbstractButton(parent)
{

}

void CurrentColorsButton::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);

    QPainterPath path;
    path.moveTo(event->rect().topLeft());
    path.lineTo(event->rect().topRight());
    path.lineTo(event->rect().right(), (event->rect().top() + event->rect().bottom()) / 2);
    path.lineTo(event->rect().left(), (event->rect().top() + event->rect().bottom()) / 2);
    painter.fillPath(path, paintColor);

    path = QPainterPath();
    path.moveTo(event->rect().left(), (event->rect().top() + event->rect().bottom()) / 2);
    path.lineTo(event->rect().right(), (event->rect().top() + event->rect().bottom()) / 2);
    path.lineTo(event->rect().bottomRight());
    path.lineTo(event->rect().bottomLeft());
    painter.fillPath(path, eraseColor);

    painter.end();
}

void CurrentColorsButton::setPaintColor(const QColor &color)
{
    this->paintColor = color;

    update();
}

void CurrentColorsButton::setEraseColor(const QColor &color)
{
    this->eraseColor = color;

    update();
}
