#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QRect>
#include <QSignalMapper>
#include <QWidget>
#include "brush.h"
#include "buffer.h"
#include "undobuffer.h"
#include "algorithms.h"
#include "brushtool.h"

// ── BrushWellButton ──────────────────────────────────────────────────────────

BrushWellButton::BrushWellButton(QWidget *parent)
    : QAbstractButton(parent), empty_(true)
{
    setFixedSize(40, 40);
}

void BrushWellButton::store(const QImage &image)
{
    brushImage_ = image;
    empty_ = false;
    QImage rgb = image.convertToFormat(QImage::Format_RGB32);
    thumbnail_ = QPixmap::fromImage(
        rgb.scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    update();
}

void BrushWellButton::clear()
{
    brushImage_ = QImage();
    thumbnail_ = QPixmap();
    empty_ = true;
    update();
}

bool BrushWellButton::isEmpty() const
{
    return empty_;
}

const QImage &BrushWellButton::brushImage() const
{
    return brushImage_;
}

void BrushWellButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if (empty_) {
        p.fillRect(rect(), QColor(100, 100, 100));
    } else {
        p.fillRect(rect(), Qt::white);
        if (!thumbnail_.isNull())
            p.drawPixmap((width() - thumbnail_.width()) / 2,
                         (height() - thumbnail_.height()) / 2, thumbnail_);
    }
    p.setPen(Qt::black);
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}

void BrushWellButton::mouseReleaseEvent(QMouseEvent *event)
{
    QAbstractButton::mouseReleaseEvent(event);
    if (rect().contains(event->pos()) && event->button() == Qt::LeftButton) {
        if (event->modifiers() & Qt::ControlModifier)
            emit ctrlClicked();
        else
            emit clicked();
    }
}

// ── BrushTool ────────────────────────────────────────────────────────────────

BrushTool BrushTool::instance;

BrushTool::BrushTool(QObject *parent) : Tool(parent),
    undoBuffer(nullptr)
{
    for (int i = 0; i < WellCount; i++)
        wellButtons_[i] = nullptr;
}

void BrushTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect BrushTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    startPoint = point;

    QRect rect = changes(point);
    undoBuffer = new UndoBuffer(rect.topLeft(), buffer_->image().copy(rect), this);
    return draw(point);
}

QRect BrushTool::move(const QPoint &point)
{
    if (mouseButton_ == Qt::NoButton) {
        return QRect();
    }

    undoBuffer->apply(buffer_);
    delete undoBuffer;

    QRect changedRect;
    Algorithms::rectangle(startPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->changes(point)); });
    undoBuffer = new UndoBuffer(changedRect.topLeft(), buffer_->image().copy(changedRect), this);
    Algorithms::rectangle(startPoint, point, [this](const QPoint &point) { this->draw(point); });
    return changedRect;
}

QRect BrushTool::release(const QPoint &point)
{
    QRect changedRect = undoBuffer->rect();
    undoBuffer->apply(buffer_);
    delete undoBuffer;
    undoBuffer = nullptr;

    QImage image = buffer_->image().copy(QRect(startPoint, point));
    buffer_->setPen(new Brush(image));
    buffer_->setTool(tools.at(0));

    if (mouseButton_ == Qt::RightButton) {
        Algorithms::fillRectangle(startPoint, point, [this](const QPoint &point) { this->draw(point); });
    }

    return changedRect;
}

QRect BrushTool::changes(const QPoint &point)
{
    return buffer_->toolPen()->rect(point);
}

QRect BrushTool::draw(const QPoint &point)
{
    if (mouseButton_ == Qt::LeftButton) {
        return buffer_->toolPen()->paint(point, buffer_);
    } else {
        return buffer_->toolPen()->erase(point, buffer_);
    }
}

void BrushTool::storeToWell(int index)
{
    Brush *brush = qobject_cast<Brush *>(buffer_->pen());
    if (!brush)
        return;
    wells_[index] = brush->image();
    if (wellButtons_[index])
        wellButtons_[index]->store(wells_[index]);
}

void BrushTool::wellClicked(int index)
{
    if (wellButtons_[index]->isEmpty())
        storeToWell(index);
    else
        buffer_->setPen(new Brush(wells_[index], -1, buffer_));
}

void BrushTool::wellCtrlClicked(int index)
{
    storeToWell(index);
}

QWidget *BrushTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    w->setWindowTitle("Brush Wells");

    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setSpacing(2);
    layout->setContentsMargins(4, 4, 4, 4);

    QSignalMapper *clickMapper = new QSignalMapper(w);
    QSignalMapper *ctrlMapper  = new QSignalMapper(w);
    connect(clickMapper, SIGNAL(mapped(int)), this, SLOT(wellClicked(int)));
    connect(ctrlMapper,  SIGNAL(mapped(int)), this, SLOT(wellCtrlClicked(int)));

    for (int i = 0; i < WellCount; i++) {
        BrushWellButton *btn = new BrushWellButton(w);
        wellButtons_[i] = btn;
        if (!wells_[i].isNull())
            btn->store(wells_[i]);
        clickMapper->setMapping(btn, i);
        ctrlMapper->setMapping(btn, i);
        connect(btn, SIGNAL(clicked()),    clickMapper, SLOT(map()));
        connect(btn, SIGNAL(ctrlClicked()), ctrlMapper,  SLOT(map()));
        layout->addWidget(btn);
    }

    return w;
}

void BrushTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/cutbrush.png"));
    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void BrushTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 2);
}
