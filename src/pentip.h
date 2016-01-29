#ifndef PENTIP_H
#define PENTIP_H

#include "pen.h"

class PenTip : public Pen
{
    Q_OBJECT
public:
    explicit PenTip(QObject *parent = 0);

    void setPaintColor(unsigned paintColor);
    void setEraseColor(unsigned eraseColor);

    virtual QRect paint(const QPoint &point, QImage &image);
    virtual QRect erase(const QPoint &point, QImage &image);

private:
    unsigned paintColor_;
    unsigned eraseColor_;
};

#endif // PENTIP_H
