#ifndef PENTIP_H
#define PENTIP_H

#include "pen.h"

class Buffer;

class PenTip : public Pen
{
    Q_OBJECT
public:
    explicit PenTip(QObject *parent = 0);

    void setPaintColor(unsigned paintColor);
    void setEraseColor(unsigned eraseColor);

    virtual QRect paint(const QPoint &point, Buffer *buffer) const;
    virtual QRect erase(const QPoint &point, Buffer *buffer) const;
    virtual QRect rect(const QPoint &point) const;

private:
    unsigned paintColor_;
    unsigned eraseColor_;
};

#endif // PENTIP_H
