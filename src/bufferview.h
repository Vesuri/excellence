#ifndef BUFFERVIEW_H
#define BUFFERVIEW_H

#include <QWidget>

namespace Ui {
class ImageWindow;
}

class QGraphicsScene;
class QGraphicsPixmapItem;
class Buffer;
class QPixmap;
class Tool;

class BufferView : public QWidget
{
    Q_OBJECT

public:
    explicit BufferView(QWidget *parent = 0);
    ~BufferView();

    void setTool(Tool *tool);
    void setBuffer(Buffer *buffer);
    virtual bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void setPixmap(const QRect &area);

private:
    Ui::ImageWindow *ui;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixmapItem;
    Buffer *buffer;
    Tool *tool;
};

#endif // BUFFERVIEW_H
