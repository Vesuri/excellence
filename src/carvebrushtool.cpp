#include <QPainter>
#include <QGridLayout>
#include "algorithms.h"
#include "brush.h"
#include "buffer.h"
#include "drawtool.h"
#include "undobuffer.h"
#include "carvebrushtool.h"

CarveBrushTool CarveBrushTool::instance;

CarveBrushTool::CarveBrushTool(QObject *parent) : Tool(parent),
    undoBuffer_(nullptr)
{
}

void CarveBrushTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr)
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    Tool::setBuffer(buffer);
    if (buffer_ != nullptr)
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
}

QRect CarveBrushTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    polygon_.clear();
    polygon_ << point;
    prevPoint_ = point;

    // Save full canvas for non-destructive restoration on release
    undoBuffer_ = new UndoBuffer(QPoint(), buffer_->image().copy(), this);

    return buffer_->toolPen()->paint(point, buffer_);
}

QRect CarveBrushTool::move(const QPoint &point)
{
    if (mouseButton_ == Qt::NoButton)
        return QRect();

    polygon_ << point;

    QRect changedRect;
    Algorithms::line(prevPoint_, point, [this, &changedRect](const QPoint &p) {
        changedRect = changedRect.united(buffer_->toolPen()->paint(p, buffer_));
    });
    prevPoint_ = point;
    return changedRect;
}

QRect CarveBrushTool::release(const QPoint &point)
{
    polygon_ << point;

    // Restore the canvas before capturing pixels
    undoBuffer_->apply(buffer_);
    delete undoBuffer_;
    undoBuffer_ = nullptr;

    QRect bounds = polygon_.boundingRect().intersected(buffer_->image().rect());
    if (bounds.isEmpty())
        return buffer_->image().rect();

    // Capture pixels from the original (restored) canvas
    QImage areaImage = buffer_->image().copy(bounds);

    // Rasterize the polygon into a mask
    QImage mask(bounds.size(), QImage::Format_ARGB32);
    mask.fill(Qt::transparent);
    {
        QPainter p(&mask);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);
        p.drawPolygon(polygon_.translated(-bounds.topLeft()));
    }

    // Set pixels outside the polygon to the erase color
    int eraseIdx = static_cast<int>(buffer_->eraseColor());
    for (int y = 0; y < bounds.height(); y++) {
        for (int x = 0; x < bounds.width(); x++) {
            if (qAlpha(mask.pixel(x, y)) == 0)
                areaImage.setPixel(x, y, static_cast<uint>(eraseIdx));
        }
    }

    // Create brush with erase color as transparent, activate DrawTool
    buffer_->setPen(new Brush(areaImage, eraseIdx, buffer_));
    for (Tool *t : tools) {
        if (qobject_cast<DrawTool *>(t)) {
            t->click();
            break;
        }
    }

    return buffer_->image().rect();
}

void CarveBrushTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/carvebrush.png"));
    button_->setToolTip("Carve Brush – Freehand shape selection");
    button_->setCheckable(true);
    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void CarveBrushTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 12);
}
