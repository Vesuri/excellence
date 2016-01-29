#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPixmap>
#include <QGraphicsSceneMouseEvent>
#include "tool.h"
#include "buffer.h"
#include "bufferview.h"
#include "ui_imagewindow.h"

BufferView::BufferView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageWindow),
    scene(new QGraphicsScene(this)),
    pixmapItem(new QGraphicsPixmapItem),
    buffer(0),
    tool(0)
{
    ui->setupUi(this);

    scene->addItem(pixmapItem);
    ui->graphicsView->setScene(scene);

    scene->installEventFilter(this);
}

BufferView::~BufferView()
{
    delete ui;
}

void BufferView::setTool(Tool *tool)
{
    this->tool = tool;
}

void BufferView::setBuffer(Buffer *buffer)
{
    if (this->buffer != 0) {
        disconnect(this->buffer, SIGNAL(modified(QRect)));
    }

    this->buffer = buffer;

    if (buffer != 0) {
        connect(buffer, SIGNAL(modified(QRect)), this, SLOT(setPixmap(QRect)));
        pixmapItem->setPixmap(QPixmap::fromImage(*buffer->image()));
    }
}

bool BufferView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->graphicsView->scene()) {
        switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
        case QEvent::GraphicsSceneMouseMove:
        case QEvent::GraphicsSceneMouseRelease:
        {
            QGraphicsSceneMouseEvent *mouseEvent = (QGraphicsSceneMouseEvent *)event;

            if (mouseEvent->buttons() != Qt::NoButton) {
                switch (event->type()) {
                case QEvent::GraphicsSceneMousePress:
                    changedRect = tool->press(((QGraphicsSceneMouseEvent *)event)->scenePos().toPoint(), *buffer->image());
                    break;
                case QEvent::GraphicsSceneMouseMove:
                    changedRect = changedRect.united(tool->move(((QGraphicsSceneMouseEvent *)event)->scenePos().toPoint(), *buffer->image()));
                    break;
                case QEvent::GraphicsSceneMouseRelease:
                    changedRect = changedRect.united(tool->release(((QGraphicsSceneMouseEvent *)event)->scenePos().toPoint(), *buffer->image()));
                    break;
                default:
                    break;
                }

                if (!changedRect.isEmpty()) {
                    pixmapItem->setPixmap(QPixmap::fromImage(*buffer->image()));
                }
            }
        }
        default:
            break;
        }
    }
    return false;
}

void BufferView::setPixmap(const QRect &area)
{
    if (!area.isEmpty()) {
        pixmapItem->setPixmap(QPixmap::fromImage(*buffer->image()));
    }
}
