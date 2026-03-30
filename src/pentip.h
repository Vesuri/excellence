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
    QRect rect(const QPoint &point) const override;

    int size() const;
    int width() const;
    int height() const;
    Shape shape() const;

public slots:
    void setPaintColor(unsigned paintColor);
    void setEraseColor(unsigned eraseColor);
    void setSize(int w, int h);
    void setShape(Shape shape);

private:
    bool inTip(int dx, int dy, int hw, int hh) const;
    void applyPrimary(const QPoint &point, Buffer *buffer, bool isErase) const;
    void applyColor(const QPoint &point, Buffer *buffer, unsigned color) const;
    void applySmear(const QPoint &point, Buffer *buffer, unsigned fallbackColor) const;
    void applySmooth(const QPoint &point, Buffer *buffer) const;
    void applyRange(const QPoint &point, Buffer *buffer, bool isErase) const;
    void applyAverageSmear(const QPoint &point, Buffer *buffer) const;
    void applyCycleRandom(const QPoint &point, Buffer *buffer, bool isErase, bool isRandom) const;
    void applyColorEffect(const QPoint &point, Buffer *buffer, unsigned baseColor, Buffer::PaintMode mode) const;
    void applyDither(const QPoint &point, Buffer *buffer, unsigned paintColor, unsigned bgColor, bool useBg) const;
    void applyTransparent(const QPoint &point, Buffer *buffer, unsigned paintColor) const;
    void applyBrushMode(const QPoint &point, Buffer *buffer) const;

    unsigned paintColor_;
    unsigned eraseColor_;
    int width_;
    int height_;
    Shape shape_;
};

#endif // PENTIP_H
