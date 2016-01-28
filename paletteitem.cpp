#include <QDebug>
#include <QPainter>
#include "paletteitem.h"

PaletteItem::PaletteItem(QGraphicsItem *parent) : QGraphicsItem(parent),
    palette(0)
{
}

void PaletteItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    static int itemsPerRow = 2;

    if (palette != 0 && palette->count() > 0) {
        float itemWidth = size.width() / itemsPerRow;
        float itemHeight = size.height() / ((palette->count() - 1) / itemsPerRow + 1);

        for (int i = 0, x = 0, y = 0; i < palette->count(); i++) {
            painter->fillRect(QRectF(x * itemWidth, y * itemHeight, itemWidth, itemHeight), QColor(palette->at(i)));
            x++;
            if (x >= itemsPerRow) {
                x -= itemsPerRow;
                y++;
            }
        }
    }
}

QRectF PaletteItem::boundingRect() const
{
    return QRectF(QPointF(), size);
}

void PaletteItem::setPalette(QVector<QRgb> *palette)
{
    this->palette = palette;

    update();
}

void PaletteItem::setSize(const QSizeF &size)
{
    this->size = size;

    update();
}
