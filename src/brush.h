#ifndef BRUSH_H
#define BRUSH_H

#include <QImage>
#include "pen.h"

class Brush : public Pen
{
    Q_OBJECT
public:
    explicit Brush(const QImage &image, int transparentIndex = -1, QObject *parent = nullptr);

    QRect paint(const QPoint &point, Buffer *buffer) const override;
    QRect erase(const QPoint &point, Buffer *buffer) const override;
    QRect rect(const QPoint &point) const override;
    const QImage &image() const;
    QPoint handleOffset() const;
    void setHandleOffset(const QPoint &offset);

private:
    QImage image_;
    int transparentIndex_;
    QPoint handleOffset_;
};

#endif // BRUSH_H
