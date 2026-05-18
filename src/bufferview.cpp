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
#include "brushtool.h"
#include "drawtool.h"
#include "rectangletool.h"
#include "linetool.h"
#include "pickcolortool.h"
#include "curvetool.h"
#include "ellipsetool.h"
#include "airbrushtool.h"
#include "filltool.h"
#include "texttool.h"
#include "cleartool.h"
#include "buffer.h"
#include "bufferview.h"
#include "drawmodetool.h"
#include "gradientrange.h"
#include "gridlocktool.h"
#include "gradienttool.h"
#include "mirrortool.h"
#include "segmenttool.h"
#include "zoomtool.h"
#include "ui_bufferview.h"

class CanvasScene : public QGraphicsScene
{
public:
    explicit CanvasScene(QObject *parent = nullptr) : QGraphicsScene(parent), pixelGrid_(false), zoomLevel_(1) {}
    void setBuffer(Buffer *) {}
    void setPixelGrid(bool enabled) { pixelGrid_ = enabled; update(); }
    bool pixelGrid() const { return pixelGrid_; }
    void setZoomLevel(int level) { zoomLevel_ = level; update(); }

    void setGuides(bool visible, QPoint point, QSize imageSize)
    {
        guidesVisible_ = visible;
        guidePoint_ = point;
        imageSize_ = imageSize;
        update();
    }

    void setStartGuide(bool visible, QPoint startPoint)
    {
        startGuideVisible_ = visible;
        startGuidePoint_ = startPoint;
        update();
    }

protected:
    void drawForeground(QPainter *painter, const QRectF &rect) override
    {
        if (pixelGrid_ && zoomLevel_ >= 3) {
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

        if (guidesVisible_ && imageSize_.isValid()) {
            painter->save();
            painter->setCompositionMode(QPainter::CompositionMode_Difference);
            painter->setPen(QPen(Qt::white, 1));
            auto drawCrosshair = [&](QPoint p) {
                painter->drawLine(QLineF(0, p.y() + 0.5, imageSize_.width(), p.y() + 0.5));
                painter->drawLine(QLineF(p.x() + 0.5, 0, p.x() + 0.5, imageSize_.height()));
            };
            drawCrosshair(guidePoint_);
            if (startGuideVisible_)
                drawCrosshair(startGuidePoint_);
            painter->restore();
        }
    }

private:
    bool pixelGrid_;
    int zoomLevel_;
    bool guidesVisible_ = false;
    bool startGuideVisible_ = false;
    QPoint guidePoint_;
    QPoint startGuidePoint_;
    QSize imageSize_;
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
        disconnect(this->buffer, &Buffer::pixelGridChanged, scene, &CanvasScene::setPixelGrid);
        disconnect(this->buffer, &Buffer::penModified, this, &BufferView::onToolChanged);
    }

    this->buffer = buffer;

    if (buffer != nullptr) {
        connect(buffer, SIGNAL(pathChanged(QString)), this, SLOT(updateWindowTitle()));
        connect(buffer, SIGNAL(modified(QRect)), this, SLOT(setPixmap(QRect)));
        connect(buffer, SIGNAL(zoomed(QRect)), this, SLOT(setZoom(QRect)));
        connect(buffer, SIGNAL(toolChanged(Tool*)), this, SLOT(updateWindowTitle()));
        connect(buffer, SIGNAL(toolChanged(Tool*)), this, SLOT(onToolChanged()));
        connect(buffer, &Buffer::penModified, this, &BufferView::onToolChanged);
        connect(buffer, &Buffer::paintModeChanged, this, [this](Buffer::PaintMode) { updateWindowTitle(); });
        connect(buffer, SIGNAL(paintColorChanged(unsigned,QColor)), this, SLOT(updateWindowTitle()));
        connect(buffer, SIGNAL(eraseColorChanged(unsigned,QColor)), this, SLOT(updateWindowTitle()));
        connect(buffer, &Buffer::pixelGridChanged, scene, &CanvasScene::setPixelGrid);
        scene->setPixelGrid(buffer->pixelGrid());
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
            emit cursorMoved(point, buffer && buffer->image().rect().contains(point));
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

            if (panMode_) {
                switch (event->type()) {
                case QEvent::GraphicsSceneMousePress:
                    panDragging_ = true;
                    panLastScreenPos_ = mouseEvent->screenPos();
                    ui->graphicsView->viewport()->setCursor(Qt::ClosedHandCursor);
                    break;
                case QEvent::GraphicsSceneMouseMove:
                    mouseOverCanvas_ = true;
                    lastMousePoint = point;
                    if (panDragging_) {
                        QPoint delta = mouseEvent->screenPos() - panLastScreenPos_;
                        panLastScreenPos_ = mouseEvent->screenPos();
                        ui->graphicsView->horizontalScrollBar()->setValue(
                            ui->graphicsView->horizontalScrollBar()->value() - delta.x());
                        ui->graphicsView->verticalScrollBar()->setValue(
                            ui->graphicsView->verticalScrollBar()->value() - delta.y());
                    }
                    break;
                case QEvent::GraphicsSceneMouseRelease:
                    panDragging_ = false;
                    panMode_ = false;
                    ui->graphicsView->viewport()->setCursor(Qt::CrossCursor);
                    break;
                default:
                    break;
                }
                return true;
            }

            switch (event->type()) {
            case QEvent::GraphicsSceneMousePress: {
                Tool *pressTool = buffer->tool();
                pendingZoom_ = pressTool && pressTool->type() == Tool::Zoom;
                if (pressTool && pressTool->showGuides())
                    guideStartPoint_ = point;
                buffer->press(point, mouseEvent->button(), mouseEvent->modifiers());
                pendingZoom_ = false;
                break;
            }
            case QEvent::GraphicsSceneMouseMove:
                mouseOverCanvas_ = true;
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

            {
                Tool *tool = buffer->tool();
                scene->setGuides(tool && tool->showGuides(), point, buffer->image().size());
                scene->setStartGuide(tool && tool->showStartGuide(), guideStartPoint_);
            }

            updateWindowTitle(point);
            emit cursorMoved(point, buffer && buffer->image().rect().contains(point));
            lastMousePoint = point;
            break;
        }
        default:
            break;
        }
    } else if (watched == this) {
        switch (event->type()) {
        case QEvent::HoverLeave:
            mouseOverCanvas_ = false;
            if (buffer)
                buffer->clearHoverPreview();
            scene->setGuides(false, {}, {});
            updateWindowTitle();
            emit cursorMoved(QPoint(), false);
            break;
        default:
            break;
        }
    }

    return false;
}

void BufferView::keyPressEvent(QKeyEvent *event)
{
    handleKey(event);
}

void BufferView::handleKey(QKeyEvent *event)
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
    case Qt::Key_S:
        if (buffer) {
            QSize vp = ui->graphicsView->viewport()->size();
            int fw = qMax(1, qCeil(buffer->image().width()  * aspectX_));
            int fh = qMax(1, qCeil(buffer->image().height() * aspectY_));
            setZoomLevel(qBound(1, qMin(vp.width() / fw, vp.height() / fh), 32));
            ui->graphicsView->centerOn(pixmapItem);
        }
        break;
    case Qt::Key_N:
        if (event->modifiers() & Qt::ShiftModifier) {
            panMode_ = !panMode_;
            panDragging_ = false;
            ui->graphicsView->viewport()->setCursor(
                panMode_ ? Qt::OpenHandCursor : Qt::CrossCursor);
        } else {
            if (mouseOverCanvas_)
                ui->graphicsView->centerOn(QPointF(lastMousePoint));
            else if (buffer)
                ui->graphicsView->centerOn(
                    QPointF(buffer->image().width() / 2.0, buffer->image().height() / 2.0));
        }
        break;
    case Qt::Key_M:
        for (Tool *tool : tools) {
            if (auto *zt = qobject_cast<ZoomTool *>(tool)) {
                zt->enterPlaceMagnifierMode(event->modifiers() & Qt::ShiftModifier ? 8 : 4);
                break;
            }
        }
        break;
    case Qt::Key_P:
        if (buffer) buffer->setPixelGrid(!buffer->pixelGrid());
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
            if (auto *at = qobject_cast<AirbrushTool *>(tool)) {
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
                tool->click();
                if (event->modifiers() & Qt::ShiftModifier)
                    et->setDrawMode(EllipseTool::FilledEllipse);
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
    case Qt::Key_Tab:
        GradientTool::instance.toggle();
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
        } else {
            for (Tool *tool : tools) {
                if (auto *rt = qobject_cast<RectangleTool *>(tool)) {
                    tool->click();
                    if (event->modifiers() & Qt::ShiftModifier)
                        rt->setDrawMode(RectangleTool::FilledRectangle);
                    break;
                }
            }
        }
        break;
    case Qt::Key_B:
        for (Tool *tool : tools) {
            if (auto *bt = qobject_cast<BrushTool *>(tool)) {
                if (event->modifiers() & Qt::ShiftModifier)
                    bt->brushRestore();
                else
                    tool->click();
                break;
            }
        }
        break;
    case Qt::Key_O:
        for (Tool *tool : tools) {
            if (auto *bt = qobject_cast<BrushTool *>(tool)) {
                if (event->modifiers() & Qt::ShiftModifier)
                    bt->brushTrim();
                else
                    bt->brushOutline();
                break;
            }
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
        else if (buffer->tool() && buffer->tool()->isInRubberBandMode())
            buffer->tool()->cancel();
        else
            buffer->undo();
        break;
    default:
        break;
    }
}

void BufferView::keyReleaseEvent(QKeyEvent *event)
{
    handleKeyRelease(event);
}

void BufferView::handleKeyRelease(QKeyEvent *event)
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


void BufferView::updateWindowTitle(const QPoint &mouseCoordinates)
{
    if (!buffer) return;
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

    title += " | " + effectiveDrawModeName(buffer->paintMode());

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

void BufferView::onToolChanged()
{
    if (mouseOverCanvas_ && buffer && buffer->tool() && buffer->tool()->mouseButton() == Qt::NoButton)
        buffer->move(lastMousePoint);
}
