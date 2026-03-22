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
    int transparentIndex() const;
    void setTransparentIndex(int index);
    void remap(const QVector<QRgb> &palette);
    void replaceColor(int fromIndex, int toIndex);
    void detectBackground();

    void flipHorizontal();
    void flipVertical();
    void rotate90CW();
    void rotate90CCW();
    void scale(int width, int height);
    void doubleSize();
    void halveSize();
    void shearX(double factor);
    void shearY(double factor);
    void bendX(double amount);
    void bendY(double amount);
    void outline(int colorIndex);
    void trim();
    void storeOriginal();
    void restoreOriginal();
    bool hasOriginal() const;

private:
    QImage reindex(const QImage &src) const;

    QImage image_;
    QImage originalImage_;
    int transparentIndex_;
    QPoint handleOffset_;
};

#endif // BRUSH_H
