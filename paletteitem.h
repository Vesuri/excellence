#ifndef PALETTEITEM_H
#define PALETTEITEM_H

#include <QGraphicsItem>

template <class T> class QVector;

class PaletteItem : public QGraphicsItem
{
public:
    explicit PaletteItem(QGraphicsItem * parent = 0);

    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
    virtual QRectF boundingRect() const;
    void setPalette(QVector<QRgb> *palette);
    void setSize(const QSizeF &size);

private:
    QVector<QRgb> *palette;
    QSizeF size;
};

#endif // PALETTEITEM_H
