#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPixmap>
#include <QGraphicsSceneMouseEvent>
#include "tool.h"
#include "imagewindow.h"
#include "ui_imagewindow.h"

ImageWindow::ImageWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageWindow),
    scene(new QGraphicsScene(this)),
    image(new QImage(320, 256, QImage::Format_Indexed8))
{
    ui->setupUi(this);

    QVector<QRgb> colors;
    colors.append(0xff959595);
    colors.append(0xff000000);
    colors.append(0xffffffff);
    colors.append(0xff3b67a2);
    colors.append(0xff7b7b7b);
    colors.append(0xffafafaf);
    colors.append(0xffaa907c);
    colors.append(0xffffa997);
    image->setColorTable(colors);
    pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));
    scene->addItem(pixmapItem);
    ui->graphicsView->setScene(scene);

    scene->installEventFilter(this);
}

ImageWindow::~ImageWindow()
{
    delete ui;
    delete image;
}

void ImageWindow::setTool(Tool *tool)
{
    this->tool = tool;
}

bool ImageWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->graphicsView->scene()) {
        switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
            changedRect = tool->press(((QGraphicsSceneMouseEvent *)event)->scenePos().toPoint(), *image);
            break;
        case QEvent::GraphicsSceneMouseMove:
            changedRect = changedRect.united(tool->move(((QGraphicsSceneMouseEvent *)event)->scenePos().toPoint(), *image));
            break;
        case QEvent::GraphicsSceneMouseRelease:
            changedRect = changedRect.united(tool->release(((QGraphicsSceneMouseEvent *)event)->scenePos().toPoint(), *image));
            break;
        default:
            break;
        }

        if (!changedRect.isEmpty()) {
            pixmapItem->setPixmap(QPixmap::fromImage(*image));
        }
    }
    return false;
}
