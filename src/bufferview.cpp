#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPixmap>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QtCore/qmath.h>
#include "tool.h"
#include "drawtool.h"
#include "connectedlinestool.h"
#include "curvetool.h"
#include "ellipsetool.h"
#include "airtool.h"
#include "filltool.h"
#include "texttool.h"
#include "cleartool.h"
#include "buffer.h"
#include "bufferview.h"
#include "ui_bufferview.h"

BufferView::BufferView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BufferView),
    scene(new QGraphicsScene(this)),
    pixmapItem(new QGraphicsPixmapItem),
    buffer(nullptr),
    lastMousePoint()
{
    ui->setupUi(this);

    pixmapItem->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    scene->addItem(pixmapItem);
    ui->graphicsView->setStyleSheet( "QGraphicsView { border-style: none; }" );
    ui->graphicsView->setScene(scene);
    setAttribute(Qt::WA_Hover);

    scene->installEventFilter(this);
    installEventFilter(this);
}

BufferView::~BufferView()
{
    delete ui;
}

void BufferView::setBuffer(Buffer *buffer)
{
    if (this->buffer != nullptr) {
        disconnect(this->buffer, SIGNAL(pathChanged(QString)));
        disconnect(this->buffer, SIGNAL(modified(QRect)));
        disconnect(this->buffer, SIGNAL(zoomed(QRect)));
    }

    this->buffer = buffer;

    if (buffer != nullptr) {
        connect(buffer, SIGNAL(pathChanged(QString)), this, SLOT(updateWindowTitle()));
        connect(buffer, SIGNAL(modified(QRect)), this, SLOT(setPixmap(QRect)));
        connect(buffer, SIGNAL(zoomed(QRect)), this, SLOT(setZoom(QRect)));
        updateWindowTitle();
        pixmapItem->setPixmap(QPixmap::fromImage(buffer->image()));
        ui->graphicsView->resetTransform();
        ui->graphicsView->setSceneRect(scene->itemsBoundingRect());

        if (buffer->image().dotsPerMeterX() != buffer->image().dotsPerMeterY()) {
            qreal aspectRatio = buffer->image().dotsPerMeterX() / static_cast<qreal>(buffer->image().dotsPerMeterY());
            qreal xAspect = aspectRatio > 1.0 ? 1.0 : (1.0 / aspectRatio);
            qreal yAspect = aspectRatio > 1.0 ? aspectRatio : 1.0;
            ui->graphicsView->scale(xAspect, yAspect);
            setGeometry(geometry().x(), geometry().y(), qCeil(buffer->image().width() * xAspect), qCeil(buffer->image().height() * yAspect));
        } else {
            setGeometry(geometry().x(), geometry().y(), buffer->image().width(), buffer->image().height());
        }
    }
}

bool BufferView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->graphicsView->scene()) {
        switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
        case QEvent::GraphicsSceneMouseMove:
        case QEvent::GraphicsSceneMouseRelease:
        case QEvent::GraphicsSceneMouseDoubleClick:
        {
            QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
            QPointF scenePos = mouseEvent->scenePos();
            QPoint point(qFloor(scenePos.x()), qFloor(scenePos.y()));

            switch (event->type()) {
            case QEvent::GraphicsSceneMousePress:
                buffer->press(point, mouseEvent->button(), mouseEvent->modifiers());
                break;
            case QEvent::GraphicsSceneMouseMove:
                buffer->move(point);
                break;
            case QEvent::GraphicsSceneMouseRelease:
                buffer->release(point);
                break;
            case QEvent::GraphicsSceneMouseDoubleClick:
                buffer->press(point, Qt::RightButton, mouseEvent->modifiers());
                buffer->release(point);
                break;
            default:
                break;
            }

            updateWindowTitle(point);
            lastMousePoint = point;
            break;
        }
        default:
            break;
        }
    } else if (watched == this) {
        switch (event->type()) {
        case QEvent::HoverLeave:
            updateWindowTitle();
            break;
        default:
            break;
        }
    }

    return false;
}

void BufferView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case '+':
        setZoom(QRect(lastMousePoint, QSize(2, 2)));
        break;
    case '-':
        setZoom(QRect(lastMousePoint, QSize(-2, -2)));
        break;
    case Qt::Key_D:
        for (Tool *tool : tools) {
            if (qobject_cast<DrawTool *>(tool)) {
                tool->click();
                break;
            }
        }
        break;
    case Qt::Key_Q:
        for (Tool *tool : tools) {
            if (qobject_cast<CurveTool *>(tool)) {
                tool->click();
                break;
            }
        }
        break;
    case Qt::Key_W:
        for (Tool *tool : tools) {
            if (qobject_cast<ConnectedLinesTool *>(tool)) {
                tool->click();
                break;
            }
        }
        break;
    case Qt::Key_F:
        for (Tool *tool : tools) {
            if (qobject_cast<FillTool *>(tool)) {
                tool->click();
                break;
            }
        }
        break;
    case Qt::Key_A:
        for (Tool *tool : tools) {
            if (auto *at = qobject_cast<AirTool *>(tool)) {
                if (event->modifiers() & Qt::ShiftModifier)
                    at->toggleOptionsWidget();
                else
                    tool->click();
                break;
            }
        }
        break;
    case Qt::Key_C:
        for (Tool *tool : tools) {
            if (auto *et = qobject_cast<EllipseTool *>(tool)) {
                if (event->modifiers() & Qt::ShiftModifier)
                    et->setDrawMode(EllipseTool::FilledEllipse);
                else
                    et->setDrawMode(EllipseTool::Ellipse);
                tool->click();
                break;
            }
        }
        break;
    case Qt::Key_T:
        for (Tool *tool : tools) {
            if (qobject_cast<TextTool *>(tool)) {
                tool->click();
                break;
            }
        }
        break;
    case Qt::Key_K:
        if (event->modifiers() & Qt::ShiftModifier)
            buffer->clearWithColor(buffer->eraseColor());
        else
            buffer->clear();
        break;
    default:
        break;
    }
}

void BufferView::setPixmap(const QRect &area)
{
    if (!area.isEmpty()) {
        pixmapItem->setPixmap(QPixmap::fromImage(buffer->image()));
    }
}

void BufferView::setZoom(const QRect &area)
{
    if (!area.isNull()) {
        qreal sx = area.width();
        qreal sy = area.height();

        if (sx < 0) {
            sx = 1.0 / -sx;
        }
        if (sy < 0) {
            sy = 1.0 / -sy;
        }

        ui->graphicsView->scale(sx, sy);
        updateWindowTitle();
    }
}

void BufferView::updateWindowTitle(const QPoint &mouseCoordinates)
{
    QString path = buffer->path();
    int depth = 1;
    for (int i = buffer->image().colorCount(); i > 2; i >>= 1) {
        depth++;
    }
    int zoom = qRound(ui->graphicsView->transform().m11() * 100.0);

    if (mouseCoordinates.x() >= 0 && mouseCoordinates.y() >= 0 && mouseCoordinates.x() < buffer->image().width() && mouseCoordinates.y() < buffer->image().height()) {
        setWindowTitle(QString("%1 (%2x%3x%4) %5% %6,%7")
                       .arg(path.isEmpty() ? "Untitled" : path)
                       .arg(buffer->image().width())
                       .arg(buffer->image().height())
                       .arg(depth)
                       .arg(zoom)
                       .arg(mouseCoordinates.x())
                       .arg(mouseCoordinates.y()));
    } else {
        setWindowTitle(QString("%1 (%2x%3x%4) %5%")
                       .arg(path.isEmpty() ? "Untitled" : path)
                       .arg(buffer->image().width())
                       .arg(buffer->image().height())
                       .arg(depth)
                       .arg(zoom));
    }
}
