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

    int size() const;

public slots:
    void setPaintColor(unsigned paintColor);
    void setEraseColor(unsigned eraseColor);
    void setSize(int size);

private:
    void applyColor(const QPoint &point, Buffer *buffer, unsigned color) const;
    void applySmear(const QPoint &point, Buffer *buffer, unsigned fallbackColor) const;
    void applySmooth(const QPoint &point, Buffer *buffer) const;

    unsigned paintColor_;
    unsigned eraseColor_;
    int size_;
};

#endif // PENTIP_H
