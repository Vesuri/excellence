#include <QImage>
#include <QRect>
#include <QGridLayout>
#include <QRadioButton>
#include <QVBoxLayout>
#include "pen.h"
#include "buffer.h"
#include "undobuffer.h"
#include "algorithms.h"
#include "rectangletool.h"

RectangleTool RectangleTool::instance;
const char *RectangleTool::icons[] = {
    ":/rectangle.png",
    ":/filledrectangle.png"
};

RectangleTool::RectangleTool(QObject *parent) : Tool(parent),
    anchorMode_(CornerToCorner), undoBuffer(nullptr)
{
}

void RectangleTool::setDrawMode(const DrawMode &drawMode)
{
    this->drawMode = drawMode;

    button_->setIcon(QIcon(icons[drawMode]));

    static const char *tips[] = {
        "Rectangle – Outline\nRight-click: anchor options",
        "Rectangle – Filled\nRight-click: anchor options"
    };
    button_->setToolTip(tips[drawMode]);
}

void RectangleTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect RectangleTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    startPoint = point;

    QRect rect = changes(point);
    undoBuffer = new UndoBuffer(rect.topLeft(), buffer_->image().copy(rect), this);
    return draw(point);
}

QRect RectangleTool::hover(const QPoint &point)
{
    return buffer_->pen()->rect(point);
}

QRect RectangleTool::move(const QPoint &point)
{
    if (mouseButton_ == Qt::NoButton) {
        return buffer_->pen()->paint(point, buffer_);
    }

    if (!undoBuffer)
        return QRect();

    undoBuffer->apply(buffer_);
    delete undoBuffer;

    QPoint p0, p1;
    cornerPoints(point, p0, p1);
    QRect changedRect;
    auto changesLambda = [this, &changedRect](const QPoint &p) { changedRect = changedRect.united(this->changes(p)); };
    auto drawLambda = [this](const QPoint &p) { this->draw(p); };
    if (drawMode == Rectangle) {
        Algorithms::rectangle(p0, p1, changesLambda);
        undoBuffer = new UndoBuffer(changedRect.topLeft(), buffer_->image().copy(changedRect), this);
        Algorithms::rectangle(p0, p1, drawLambda);
    } else {
        Algorithms::fillRectangle(p0, p1, changesLambda);
        undoBuffer = new UndoBuffer(changedRect.topLeft(), buffer_->image().copy(changedRect), this);
        Algorithms::fillRectangle(p0, p1, drawLambda);
    }
    return changedRect;
}

QRect RectangleTool::release(const QPoint &point)
{
    if (!undoBuffer)
        return QRect();

    undoBuffer->apply(buffer_);
    delete undoBuffer;
    undoBuffer = nullptr;

    QPoint p0, p1;
    cornerPoints(point, p0, p1);
    QRect changedRect;
    if (drawMode == Rectangle) {
        Algorithms::rectangle(p0, p1, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
    } else {
        Algorithms::fillRectangle(p0, p1, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
    }
    return changedRect;
}

void RectangleTool::cornerPoints(const QPoint &current, QPoint &p0, QPoint &p1) const
{
    if (anchorMode_ == CornerToCorner) {
        p0 = startPoint;
        p1 = current;
    } else {
        int dx = qAbs(current.x() - startPoint.x());
        int dy = qAbs(current.y() - startPoint.y());
        QRect ir = buffer_->image().rect();
        p0 = QPoint(qMax(ir.left(),   startPoint.x() - dx),
                    qMax(ir.top(),    startPoint.y() - dy));
        p1 = QPoint(qMin(ir.right(),  startPoint.x() + dx),
                    qMin(ir.bottom(), startPoint.y() + dy));
    }
}

QWidget* RectangleTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    w->setWindowTitle("Rectangle");
    QVBoxLayout *layout = new QVBoxLayout(w);
    QRadioButton *cornerBtn = new QRadioButton("Corner to Corner");
    QRadioButton *centerBtn = new QRadioButton("Center to Corner");
    cornerBtn->setChecked(anchorMode_ == CornerToCorner);
    centerBtn->setChecked(anchorMode_ == CenterToCorner);
    connect(centerBtn, &QRadioButton::toggled, this, &RectangleTool::setAnchorMode);
    layout->addWidget(cornerBtn);
    layout->addWidget(centerBtn);
    return w;
}

void RectangleTool::setAnchorMode(bool centerToCorner)
{
    anchorMode_ = centerToCorner ? CenterToCorner : CornerToCorner;
}

QRect RectangleTool::changes(const QPoint &point)
{
    return buffer_->pen()->rect(point);
}

QRect RectangleTool::draw(const QPoint &point)
{
    if (mouseButton_ == Qt::LeftButton) {
        return buffer_->pen()->paint(point, buffer_);
    } else {
        return buffer_->pen()->erase(point, buffer_);
    }
}

void RectangleTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/rectangle.png"));
    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void RectangleTool::activate()
{
    if (buffer_->tool() == this) {
        setDrawMode(static_cast<DrawMode>((drawMode + 1) % (FilledRectangle + 1)));
        button_->setChecked(true);
    }

    Tool::activate();
}

void RectangleTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 5);
}
