#ifndef BRUSH_H
#define BRUSH_H

#include <QImage>
#include "pen.h"

class Brush : public Pen
{
    Q_OBJECT
public:
    enum TransformQuality { LowQuality, MediumQuality, HighQuality };

    explicit Brush(const QImage &image, int transparentIndex = -1, QObject *parent = nullptr);

    QRect paint(const QPoint &point, Buffer *buffer) const override;
    QRect erase(const QPoint &point, Buffer *buffer) const override;
    QRect paintAsColor(const QPoint &point, Buffer *buffer) const override;
    QRect rect(const QPoint &point) const override;
    const QImage &image() const;
    QPoint handleOffset() const;
    void setHandleOffset(const QPoint &offset);
    QPoint alignmentOffset() const;
    void setAlignmentOffset(const QPoint &offset);
    void setCaptureOrigin(const QPoint &origin);
    bool hasCaptureOrigin() const;
    QPoint captureOrigin() const;
    bool hasLastStampPoint() const;
    QPoint lastStampPoint() const;
    QRect paintPreview(const QPoint &point, Buffer *buffer) const;
    int transparentIndex() const;
    void setTransparentIndex(int index);
    void remap(const QVector<QRgb> &palette);
    void replaceColor(int fromIndex, int toIndex);
    void detectBackground();

    void flipHorizontal();
    void flipVertical();
    void rotate90CW();
    void rotate90CCW();
    void rotateByDegrees(double degrees);
    void scale(int width, int height);
    void doubleSize();
    void doubleWidth();
    void doubleHeight();
    void halveSize();
    void halveWidth();
    void halveHeight();
    void shearX(double factor);
    void shearY(double factor);
    void bendX(double amount);
    void bendY(double amount);
    void outline(int colorIndex);
    void trim();
    void tileCut();
    void storeOriginal();
    void restoreOriginal();
    bool hasOriginal() const;
    static void setTransformQuality(TransformQuality quality);
    static TransformQuality transformQuality();

signals:
    void imageChanged();
    void alignmentChanged();

private:
    QImage reindex(const QImage &src) const;

    QImage image_;
    QImage originalImage_;
    int transparentIndex_;
    QPoint handleOffset_;
    static TransformQuality transformQuality_;
    QPoint alignmentOffset_;
    QPoint captureOrigin_;
    bool hasCaptureOrigin_ = false;
    mutable QPoint lastStampPoint_;
    mutable bool hasLastStampPoint_ = false;
};

#endif // BRUSH_H
