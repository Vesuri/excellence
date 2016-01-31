#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPixmap>
#include <QGraphicsSceneMouseEvent>
#include "tool.h"
#include "buffer.h"
#include "bufferview.h"
#include "ui_bufferview.h"

BufferView::BufferView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BufferView),
    scene(new QGraphicsScene(this)),
    pixmapItem(new QGraphicsPixmapItem),
    buffer(0),
    tool(0)
{
    ui->setupUi(this);

    scene->addItem(pixmapItem);
    ui->graphicsView->setStyleSheet( "QGraphicsView { border-style: none; }" );
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
        pixmapItem->setPixmap(QPixmap::fromImage(buffer->image()));
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

            if (event->type() == QEvent::GraphicsSceneMouseMove && mouseEvent->buttons() != Qt::NoButton) {
                buffer->move(mouseEvent->scenePos().toPoint(), tool);
            } else {
                switch (event->type()) {
                case QEvent::GraphicsSceneMousePress:
                    tool->setMode(mouseEvent->button() == Qt::LeftButton ? Tool::Paint : Tool::Erase);
                    buffer->press(mouseEvent->scenePos().toPoint(), tool);
                    break;
                case QEvent::GraphicsSceneMouseRelease:
                    buffer->release(mouseEvent->scenePos().toPoint(), tool);
                    break;
                default:
                    break;
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
        pixmapItem->setPixmap(QPixmap::fromImage(buffer->image()));
    }
}
