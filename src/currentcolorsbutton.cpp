#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include "colorutils.h"
#include "currentcolorsbutton.h"

static const int kSwatchW = 28;
static const int kGap     = 1;

CurrentColorsButton::CurrentColorsButton(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);
}

QSize CurrentColorsButton::sizeHint() const
{
    return QSize(kSwatchW * 2 + kGap, 14);
}

void CurrentColorsButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    auto drawSwatch = [&](const QRect &r, const QColor &color, int index) {
        painter.fillRect(r, color);
        painter.setPen(QColor(0, 0, 0, 160));
        painter.drawRect(r.adjusted(0, 0, -1, -1));
        painter.setPen(contrastColor(color));
        QFont f = painter.font();
        f.setPixelSize(9);
        painter.setFont(f);
        painter.drawText(r, Qt::AlignCenter, QString::number(index));
    };

    drawSwatch(QRect(0,              0, kSwatchW, height()), paintColor_, paintIndex_);
    drawSwatch(QRect(kSwatchW + kGap, 0, kSwatchW, height()), eraseColor_, eraseIndex_);
}

void CurrentColorsButton::mousePressEvent(QMouseEvent *e)
{
    if (e->position().x() <= kSwatchW)
        emit foregroundClicked();
    else
        emit backgroundClicked();
}

void CurrentColorsButton::setPaintColor(unsigned colorIndex, const QColor &color)
{
    if (paintColor_ == color && paintIndex_ == static_cast<int>(colorIndex))
        return;
    paintColor_ = color;
    paintIndex_ = static_cast<int>(colorIndex);
    update();
}

void CurrentColorsButton::setEraseColor(unsigned colorIndex, const QColor &color)
{
    if (eraseColor_ == color && eraseIndex_ == static_cast<int>(colorIndex))
        return;
    eraseColor_ = color;
    eraseIndex_ = static_cast<int>(colorIndex);
    update();
}
