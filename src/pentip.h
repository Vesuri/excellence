#ifndef PENTIP_H
#define PENTIP_H

#include "pen.h"
#include "buffer.h"

class PenTip : public Pen
{
    Q_OBJECT
public:
    enum Shape { Circle, Square };

    explicit PenTip(QObject *parent = nullptr);

    QRect paint(const QPoint &point, Buffer *buffer) const override;
    QRect erase(const QPoint &point, Buffer *buffer) const override;
    QRect paintAsColor(const QPoint &point, Buffer *buffer) const override;
    QRect rect(const QPoint &point) const override;

    int size() const;
    int width() const;
    int height() const;
    Shape shape() const;

signals:
    void sizeChanged(int w, int h);

public slots:
    void setPaintColor(unsigned paintColor);
    void setEraseColor(unsigned eraseColor);
    void setSize(int w, int h);
    void setShape(Shape shape);

private:
    bool inTip(int dx, int dy, int hw, int hh) const;
    void applyTipAt(const QPoint &point, Buffer *buffer,
                    Buffer::PaintMode mode, bool isErase,
                    unsigned paintColor, unsigned eraseColor) const;
    void applyColor(const QPoint &point, Buffer *buffer, unsigned color) const;
    void applyBrushMode(const QPoint &point, Buffer *buffer) const;

    unsigned paintColor_;
    unsigned eraseColor_;
    int width_;
    int height_;
    Shape shape_;
};

#endif // PENTIP_H
