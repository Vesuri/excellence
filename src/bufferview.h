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
    virtual bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void setPixmap(const QRect &area);

private:
    Ui::BufferView *ui;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixmapItem;
    Buffer *buffer;
};

#endif // BUFFERVIEW_H
