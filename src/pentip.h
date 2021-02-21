#ifndef PENTIP_H
#define PENTIP_H

#include "pen.h"

class Buffer;

class PenTip : public Pen
{
    Q_OBJECT
public:
    explicit PenTip(QObject *parent = nullptr);

    QRect paint(const QPoint &point, Buffer *buffer) const override;
    QRect erase(const QPoint &point, Buffer *buffer) const override;
    QRect rect(const QPoint &point) const override;

public slots:
    void setPaintColor(unsigned paintColor);
    void setEraseColor(unsigned eraseColor);

private:
    unsigned paintColor_;
    unsigned eraseColor_;
};

#endif // PENTIP_H
