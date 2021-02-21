#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include "currentcolorsbutton.h"

CurrentColorsButton::CurrentColorsButton(QWidget *parent) : QAbstractButton(parent)
{

}

void CurrentColorsButton::paintEvent(QPaintEvent *event)
{
    QRect rect = event->rect().adjusted(0, 0, 1, 1);

    QPainter painter;
    painter.begin(this);

    QPainterPath path;
    path.moveTo(rect.topLeft());
    path.lineTo(rect.topRight());
    path.lineTo(rect.right(), (rect.top() + rect.bottom()) / 2);
    path.lineTo(rect.left(), (rect.top() + rect.bottom()) / 2);
    painter.fillPath(path, paintColor);

    path = QPainterPath();
    path.moveTo(rect.left(), (rect.top() + rect.bottom()) / 2);
    path.lineTo(rect.right(), (rect.top() + rect.bottom()) / 2);
    path.lineTo(rect.bottomRight());
    path.lineTo(rect.bottomLeft());
    painter.fillPath(path, eraseColor);

    painter.end();
}

void CurrentColorsButton::setPaintColor(unsigned, const QColor &color)
{
    this->paintColor = color;

    update();
}

void CurrentColorsButton::setEraseColor(unsigned, const QColor &color)
{
    this->eraseColor = color;

    update();
}
