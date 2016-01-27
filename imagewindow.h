#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

#include <QWidget>

namespace Ui {
class ImageWindow;
}

class QGraphicsScene;
class QImage;
class QPixmap;

class ImageWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWindow(QWidget *parent = 0);
    ~ImageWindow();

private:
    Ui::ImageWindow *ui;
    QGraphicsScene *scene;
    QImage *image;
    QPixmap *pixmap;
};

#endif // IMAGEWINDOW_H
