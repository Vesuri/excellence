#include <QApplication>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPixmap>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QtCore/qmath.h>
#include "tool.h"
#include "brush.h"
#include "drawtool.h"
#include "linetool.h"
#include "pickcolortool.h"
#include "curvetool.h"
#include "ellipsetool.h"
#include "airtool.h"
#include "filltool.h"
#include "texttool.h"
#include "cleartool.h"
#include "buffer.h"
#include "bufferview.h"
#include "gridlocktool.h"
#include "gradienttool.h"
#include "mirrortool.h"
#include "ui_bufferview.h"

class CanvasScene : public QGraphicsScene
{
public:
    explicit CanvasScene(QObject *parent = nullptr) : QGraphicsScene(parent), pixelGrid_(false), zoomLevel_(1) {}
    void setBuffer(Buffer *) {}
    void setPixelGrid(bool enabled) { pixelGrid_ = enabled; update(); }
    bool pixelGrid() const { return pixelGrid_; }
    void setZoomLevel(int level) { zoomLevel_ = level; update(); }

protected:
    void drawForeground(QPainter *painter, const QRectF &rect) override
    {
        if (!pixelGrid_ || zoomLevel_ < 4) return;
        painter->save();
        painter->setPen(QPen(QColor(0, 0, 0, 80), 0));
        int left = qFloor(rect.left());
        int top  = qFloor(rect.top());
        int right  = qCeil(rect.right());
        int bottom = qCeil(rect.bottom());
        for (int x = left + 1; x <= right; x++)
            painter->drawLine(QLineF(x, rect.top(), x, rect.bottom()));
        for (int y = top + 1; y <= bottom; y++)
            painter->drawLine(QLineF(rect.left(), y, rect.right(), y));
        painter->restore();
    }

private:
    bool pixelGrid_;
    int zoomLevel_;
};

BufferView::BufferView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BufferView),
    scene(new CanvasScene(this)),
    pixmapItem(new QGraphicsPixmapItem),
    buffer(nullptr),
    lastMousePoint(),
    altPreviousTool_(nullptr),
    cursorHidden_(false),
    pendingZoom_(false),
    zoomLevel_(1),
    pixelGrid_(false),
    aspectX_(1.0),
    aspectY_(1.0)
{
    ui->setupUi(this);

    pixmapItem->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    scene->addItem(pixmapItem);
    ui->graphicsView->setStyleSheet( "QGraphicsView { border-style: none; }" );
    ui->graphicsView->setScene(scene);
    ui->graphicsView->viewport()->setCursor(Qt::CrossCursor);
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
        disconnect(this->buffer, SIGNAL(toolChanged(Tool*)));
        disconnect(this->buffer, SIGNAL(paintColorChanged(unsigned,QColor)));
        disconnect(this->buffer, SIGNAL(eraseColorChanged(unsigned,QColor)));
    }

    this->buffer = buffer;

    if (buffer != nullptr) {
        connect(buffer, SIGNAL(pathChanged(QString)), this, SLOT(updateWindowTitle()));
        connect(buffer, SIGNAL(modified(QRect)), this, SLOT(setPixmap(QRect)));
        connect(buffer, SIGNAL(zoomed(QRect)), this, SLOT(setZoom(QRect)));
        connect(buffer, SIGNAL(toolChanged(Tool*)), this, SLOT(updateWindowTitle()));
        connect(buffer, SIGNAL(paintColorChanged(unsigned,QColor)), this, SLOT(updateWindowTitle()));
        connect(buffer, SIGNAL(eraseColorChanged(unsigned,QColor)), this, SLOT(updateWindowTitle()));
        updateWindowTitle();
        pixmapItem->setPixmap(QPixmap::fromImage(buffer->image()));
        ui->graphicsView->setSceneRect(scene->itemsBoundingRect());

        if (buffer->image().dotsPerMeterX() != buffer->image().dotsPerMeterY()) {
            qreal aspectRatio = buffer->image().dotsPerMeterX() / static_cast<qreal>(buffer->image().dotsPerMeterY());
            aspectX_ = aspectRatio > 1.0 ? 1.0 : (1.0 / aspectRatio);
            aspectY_ = aspectRatio > 1.0 ? aspectRatio : 1.0;
            setGeometry(geometry().x(), geometry().y(), qCeil(buffer->image().width() * aspectX_), qCeil(buffer->image().height() * aspectY_));
        } else {
            aspectX_ = 1.0;
            aspectY_ = 1.0;
            setGeometry(geometry().x(), geometry().y(), buffer->image().width(), buffer->image().height());
        }
        zoomLevel_ = 1;
        applyTransform();
    }
}

bool BufferView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->graphicsView->scene()) {
        switch (event->type()) {
        case QEvent::GraphicsSceneWheel:
        {
            QGraphicsSceneWheelEvent *wheelEvent = static_cast<QGraphicsSceneWheelEvent *>(event);
            QPointF scenePos = wheelEvent->scenePos();
            QPoint point(qFloor(scenePos.x()), qFloor(scenePos.y()));
            setZoomLevel(zoomLevel_ + (wheelEvent->delta() > 0 ? 1 : -1));
            ui->graphicsView->centerOn(point);
            updateWindowTitle(point);
            return true;
        }
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
                pendingZoom_ = buffer->tool() && buffer->tool()->type() == Tool::Zoom;
                buffer->press(point, mouseEvent->button(), mouseEvent->modifiers());
                pendingZoom_ = false;
                break;
            case QEvent::GraphicsSceneMouseMove:
                buffer->move(point);
                break;
            case QEvent::GraphicsSceneMouseRelease:
                buffer->release(point);
                break;
            case QEvent::GraphicsSceneMouseDoubleClick:
                buffer->press(point, mouseEvent->button(), mouseEvent->modifiers());
                buffer->doubleClick(point);
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
            if (buffer)
                buffer->clearHoverPreview();
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
    case '>':
        setZoomLevel(zoomLevel_ + 1);
        break;
    case '-':
    case '<':
        setZoomLevel(zoomLevel_ - 1);
        break;
    case Qt::Key_M:
        emit magnifyRequested(event->modifiers() & Qt::ShiftModifier ? 8 : 4);
        break;
    case Qt::Key_P:
        pixelGrid_ = !pixelGrid_;
        scene->setPixelGrid(pixelGrid_);
        break;
    case Qt::Key_Left:
        ui->graphicsView->horizontalScrollBar()->setValue(
            ui->graphicsView->horizontalScrollBar()->value() - zoomLevel_);
        break;
    case Qt::Key_Right:
        ui->graphicsView->horizontalScrollBar()->setValue(
            ui->graphicsView->horizontalScrollBar()->value() + zoomLevel_);
        break;
    case Qt::Key_Up:
        ui->graphicsView->verticalScrollBar()->setValue(
            ui->graphicsView->verticalScrollBar()->value() - zoomLevel_);
        break;
    case Qt::Key_Down:
        ui->graphicsView->verticalScrollBar()->setValue(
            ui->graphicsView->verticalScrollBar()->value() + zoomLevel_);
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
            if (qobject_cast<LineTool *>(tool)) {
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
    case Qt::Key_Comma:
        for (Tool *tool : tools) {
            if (qobject_cast<PickColorTool *>(tool)) {
                tool->click();
                break;
            }
        }
        break;
    case Qt::Key_Alt:
        if (buffer && buffer->tool()) {
            if (!qobject_cast<PickColorTool *>(buffer->tool())) {
                altPreviousTool_ = buffer->tool();
                for (Tool *tool : tools) {
                    if (qobject_cast<PickColorTool *>(tool)) {
                        tool->click();
                        break;
                    }
                }
            }
        }
        break;
    case Qt::Key_G:
        buffer->setGridEnabled(!buffer->gridEnabled());
        break;
    case Qt::Key_Slash:
        MirrorTool::instance.toggle();
        break;
    case Qt::Key_Escape:
        if (buffer && buffer->tool())
            buffer->tool()->cancel();
        break;
    case Qt::Key_Delete:
        if (cursorHidden_) {
            QApplication::restoreOverrideCursor();
            cursorHidden_ = false;
        } else {
            QApplication::setOverrideCursor(Qt::BlankCursor);
            cursorHidden_ = true;
        }
        break;
    case Qt::Key_K:
        if (event->modifiers() & Qt::ShiftModifier)
            buffer->clearWithColor(buffer->eraseColor());
        else
            buffer->clear();
        break;
    case Qt::Key_BracketLeft: {
        int n = buffer->image().colorCount();
        if (event->modifiers() & Qt::ShiftModifier)
            buffer->setEraseColor((buffer->eraseColor() + n - 1) % n);
        else
            buffer->setPaintColor((buffer->paintColor() + n - 1) % n);
        break;
    }
    case Qt::Key_BracketRight: {
        int n = buffer->image().colorCount();
        if (event->modifiers() & Qt::ShiftModifier)
            buffer->setEraseColor((buffer->eraseColor() + 1) % n);
        else
            buffer->setPaintColor((buffer->paintColor() + 1) % n);
        break;
    }
    case Qt::Key_R:
        if (event->modifiers() & Qt::AltModifier) {
            gradientRanges[activeGradientRange].flip();
            GradientTool::instance.refreshPanel();
        }
        break;
    case Qt::Key_J:
        if (event->modifiers() & Qt::AltModifier) {
            if (event->modifiers() & Qt::ShiftModifier)
                GradientTool::instance.setActiveRange((activeGradientRange + kGradientRangeCount - 1) % kGradientRangeCount);
            else
                GradientTool::instance.setActiveRange((activeGradientRange + 1) % kGradientRangeCount);
        }
        break;
    case Qt::Key_U:
        if (event->modifiers() & Qt::ControlModifier)
            buffer->clearUndoBuffer();
        else if (event->modifiers() & Qt::AltModifier) {
            if (event->modifiers() & Qt::ShiftModifier)
                buffer->redoAll();
            else
                buffer->undoAll();
        } else if (event->modifiers() & Qt::ShiftModifier)
            buffer->redo();
        else
            buffer->undo();
        break;
    default:
        break;
    }
}

void BufferView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Alt && altPreviousTool_ && buffer) {
        altPreviousTool_->click();
        altPreviousTool_ = nullptr;
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
    if (!pendingZoom_ || area.isNull()) return;
    if (area.height() == 0) {
        emit magnifyAtPointRequested(area.width(), area.topLeft());
    } else {
        setZoomLevel(area.width() > 0 ? zoomLevel_ + 1 : zoomLevel_ - 1);
        ui->graphicsView->centerOn(area.topLeft());
    }
}

void BufferView::centerOn(const QPoint &point)
{
    ui->graphicsView->centerOn(point);
}

QSize BufferView::idealSize(int zoomLevel) const
{
    if (!buffer) return QSize();
    return QSize(qCeil(buffer->image().width()  * aspectX_ * zoomLevel),
                 qCeil(buffer->image().height() * aspectY_ * zoomLevel));
}

void BufferView::setZoomLevel(int level)
{
    zoomLevel_ = qBound(1, level, 32);
    applyTransform();
}

void BufferView::applyTransform()
{
    ui->graphicsView->setTransform(
        QTransform::fromScale(zoomLevel_ * aspectX_, zoomLevel_ * aspectY_));
    scene->setZoomLevel(zoomLevel_);
    updateWindowTitle();
}

static QString paintModeName(Buffer::PaintMode mode)
{
    switch (mode) {
    case Buffer::Normal:       return "Normal";
    case Buffer::Replace:      return "Replace";
    case Buffer::Smear:        return "Smear";
    case Buffer::Smooth:       return "Smooth";
    case Buffer::Range:        return "Range";
    case Buffer::AverageSmear: return "Avg Smear";
    case Buffer::Cycle:        return "Cycle";
    case Buffer::Random:       return "Random";
    case Buffer::Tint:         return "Tint";
    case Buffer::Colorize:     return "Colorize";
    case Buffer::Brighten:     return "Brighten";
    case Buffer::Darken:       return "Darken";
    case Buffer::Mix:          return "Mix";
    case Buffer::Negative:     return "Negative";
    case Buffer::Dither1:      return "Dither1";
    case Buffer::Dither2:      return "Dither2";
    case Buffer::Transparent:  return "Transparent";
    }
    return QString();
}

void BufferView::updateWindowTitle(const QPoint &mouseCoordinates)
{
    QString path = buffer->path();
    int depth = 1;
    for (int i = buffer->image().colorCount(); i > 2; i >>= 1)
        depth++;
    int zoom = qRound(ui->graphicsView->transform().m11() * 100.0);

    QString title = QString("%1 (%2x%3x%4) %5%")
        .arg(path.isEmpty() ? "Untitled" : path)
        .arg(buffer->image().width())
        .arg(buffer->image().height())
        .arg(depth)
        .arg(zoom);

    if (buffer->tool())
        title += " | " + buffer->tool()->name();

    title += " | " + paintModeName(buffer->paintMode());

    title += QString(" | %1/%2").arg(buffer->paintColor()).arg(buffer->eraseColor());

    if (Brush *brush = qobject_cast<Brush *>(buffer->pen()))
        title += QString(" | %1x%2").arg(brush->image().width()).arg(brush->image().height());

    bool coordsValid = mouseCoordinates.x() >= 0 && mouseCoordinates.y() >= 0
                    && mouseCoordinates.x() < buffer->image().width()
                    && mouseCoordinates.y() < buffer->image().height();
    if (coordsValid) {
        title += QString(" | %1,%2").arg(mouseCoordinates.x()).arg(mouseCoordinates.y());
        if (buffer->tool()) {
            QString s = buffer->tool()->status();
            if (!s.isEmpty())
                title += " " + s;
        }
    }

    setWindowTitle(title);
}
