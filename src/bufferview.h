#ifndef BUFFERVIEW_H
#define BUFFERVIEW_H

#include <QWidget>

namespace Ui {
class BufferView;
}

class QGraphicsScene;
class QGraphicsPixmapItem;
class Buffer;
class QPixmap;

class BufferView : public QWidget
{
    Q_OBJECT

public:
    explicit BufferView(QWidget *parent = nullptr);
    ~BufferView();

    void setBuffer(Buffer *buffer);
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void setPixmap(const QRect &area);
    void setZoom(const QRect &area);
    void updateWindowTitle(const QPoint &mouseCoordinates = QPoint(-1, -1));

private:
    Ui::BufferView *ui;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixmapItem;
    Buffer *buffer;
};

#endif // BUFFERVIEW_H
