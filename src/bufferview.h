#ifndef BUFFERVIEW_H
#define BUFFERVIEW_H

#include <QWidget>

namespace Ui {
class BufferView;
}

class CanvasScene;
class QGraphicsPixmapItem;
class Buffer;
class QPixmap;
class Tool;

class BufferView : public QWidget
{
    Q_OBJECT

public:
    explicit BufferView(QWidget *parent = nullptr);
    ~BufferView();

    void setBuffer(Buffer *buffer);
    void setZoomLevel(int level);
    void centerOn(const QPoint &point);
    QSize idealSize(int zoomLevel) const;
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void magnifyAtPointRequested(int zoomLevel, QPoint point);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void setPixmap(const QRect &area);
    void setZoom(const QRect &area);
    void updateWindowTitle(const QPoint &mouseCoordinates = QPoint(-1, -1));
    void onToolChanged();

private:
    void applyTransform();

    Ui::BufferView *ui;
    CanvasScene *scene;
    QGraphicsPixmapItem *pixmapItem;
    Buffer *buffer;
    QPoint lastMousePoint;
    Tool *altPreviousTool_;
    bool cursorHidden_;
    bool pendingZoom_;
    int zoomLevel_;
    QPoint guideStartPoint_;
    bool mouseOverCanvas_ = false;
    qreal aspectX_;
    qreal aspectY_;
};

#endif // BUFFERVIEW_H
