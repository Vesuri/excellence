#ifndef PENTIP_H
#define PENTIP_H

#include "pen.h"

class PenTip : public Pen
{
    Q_OBJECT
public:
    explicit PenTip(QObject *parent = 0);

    void setColor(int color);

    virtual QRect draw(const QPoint &point, QImage &image);

private:
    int color;
};

#endif // PENTIP_H
