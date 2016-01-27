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
    pixmapItem(new QGraphicsPixmapItem),
    image(0),
    tool(0)
{
    ui->setupUi(this);

    scene->addItem(pixmapItem);
    ui->graphicsView->setScene(scene);

    scene->installEventFilter(this);
}

ImageWindow::~ImageWindow()
{
    delete ui;
}

void ImageWindow::setTool(Tool *tool)
{
    this->tool = tool;
}

void ImageWindow::setImage(QImage *image)
{
    this->image = image;
    pixmapItem->setPixmap(QPixmap::fromImage(*image));
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
