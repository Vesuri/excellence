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
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void setPixmap(const QRect &area);
    void setZoom(const QRect &area);
    void updateWindowTitle(const QPoint &mouseCoordinates = QPoint(-1, -1));

private:
    Ui::BufferView *ui;
    CanvasScene *scene;
    QGraphicsPixmapItem *pixmapItem;
    Buffer *buffer;
    QPoint lastMousePoint;
    Tool *altPreviousTool_;
    bool cursorHidden_;
};

#endif // BUFFERVIEW_H
