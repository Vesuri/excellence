#ifndef GRADIENTMARKERBOX_H
#define GRADIENTMARKERBOX_H

#include <QWidget>
#include "gradientrange.h"

class Buffer;

class GradientMarkerBox : public QWidget
{
    Q_OBJECT

public:
    explicit GradientMarkerBox(QWidget *parent = nullptr);

    void setRange(GradientRange *range);
    void setBuffer(Buffer *buffer);

    QSize sizeHint() const override;

signals:
    void rangeChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    static const int kSlotCount = 128;
    static const int kMarkerRowHeight = 20;
    static const int kPreviewHeight = 10;

    int slotAt(int x) const;
    int slotX(int slot) const;
    QColor colorForIndex(int colorIndex) const;
    struct DitherPair { int idx1; int idx2; float blend; };
    DitherPair ditherPair(const QColor &ideal) const;
    QColor interpolatedColor(float slotPos, int pixelX) const;

    GradientRange *range_ = nullptr;
    Buffer *buffer_ = nullptr;
};

#endif // GRADIENTMARKERBOX_H
