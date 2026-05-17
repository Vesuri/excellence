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
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    static const int kSlotCount = kGradientSlotCount;
    static const int kMarkerRowHeight = 20;
    static const int kPreviewHeight = 10;

    int slotAt(int x) const;
    int slotX(int slot) const;
    QColor colorForIndex(int colorIndex) const;
    void saveSlotState(int slot);
    void shiftMarkers(int fromSlot, int toSlot, int delta);
    void shiftMarkersAt(int slot, int x);
    void shiftRightClickAt(int slot);

    GradientRange *range_ = nullptr;
    Buffer *buffer_ = nullptr;

    bool dragging_ = false;
    int dragStartSlot_ = 0;
    int dragBaseColor_ = 0;
    struct DragSlotState { bool hadMarker; int colorIndex; int incomingColorIndex; bool abrupt; };
    QMap<int, DragSlotState> dragSavedStates_;
};

#endif // GRADIENTMARKERBOX_H
