#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

#include <QWidget>

namespace Ui {
class ImageWindow;
}

class QGraphicsScene;
class QGraphicsPixmapItem;
class QImage;
class QPixmap;
class Tool;

class ImageWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWindow(QWidget *parent = 0);
    ~ImageWindow();

    void setTool(Tool *tool);
    virtual bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::ImageWindow *ui;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixmapItem;
    QImage *image;
    Tool *tool;
    QRect changedRect;
};

#endif // IMAGEWINDOW_H
