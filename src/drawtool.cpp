#include <QImage>
#include <QLabel>
#include <QRect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include "pen.h"
#include "pentip.h"
#include "buffer.h"
#include "algorithms.h"
#include "drawtool.h"

DrawTool DrawTool::instance;
const char *DrawTool::icons[] = {
    ":/draw.png",
    ":/connecteddraw.png",
    ":/filledshape.png"
};

DrawTool::DrawTool(QObject *parent) : Tool(parent)
{
}

void DrawTool::setDrawMode(const DrawMode &drawMode)
{
    this->drawMode = drawMode;

    button_->setIcon(QIcon(icons[drawMode]));

    static const char *tips[] = {
        "Draw – Dotted [D]",
        "Draw – Connected [D]",
        "Draw – Filled Shape [D]"
    };
    button_->setToolTip(tips[drawMode]);
}

void DrawTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect DrawTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    startingPoint = point;
    previousPoint = point;
    lastStampedPoint = point;
    drawnBounds_ = buffer_->pen()->rect(point).intersected(buffer_->image().rect());
    return draw(point);
}

QRect DrawTool::move(const QPoint &point)
{
    if (mouseButton_ == Qt::NoButton) {
        return draw(point);
    } else if (drawMode == Dotted) {
        QPoint delta = point - lastStampedPoint;
        constexpr int threshold = 1;
        if (delta.x() * delta.x() + delta.y() * delta.y() >= threshold * threshold) {
            lastStampedPoint = point;
            return draw(point);
        }
        return QRect();
    } else {
        QRect changedRect;
        Algorithms::line(previousPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        previousPoint = point;
        drawnBounds_ = drawnBounds_.united(changedRect);
        return changedRect;
    }
}

QRect DrawTool::hover(const QPoint &point)
{
    return buffer_->pen()->rect(point);
}

QRect DrawTool::release(const QPoint &point)
{
    if (drawMode == Dotted) {
        return QRect();
    } else if (drawMode == ConnectedDraw) {
        QRect changedRect;
        Algorithms::line(previousPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        return changedRect;
    } else {
        QRect changedRect;
        Algorithms::line(previousPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        Algorithms::line(point, startingPoint, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        drawnBounds_ = drawnBounds_.united(changedRect);
        // Flood fill the interior of the closed shape
        QImage &img = buffer_->image();
        QPoint seed = drawnBounds_.intersected(img.rect()).center();
        int fillColor = static_cast<int>(mouseButton_ == Qt::RightButton
                                         ? buffer_->eraseColor()
                                         : buffer_->paintColor());
        if (img.rect().contains(seed)) {
            int targetColor = img.pixelIndex(seed);
            if (targetColor != fillColor)
                changedRect = changedRect.united(Algorithms::floodFill(img, seed, targetColor, fillColor));
        }
        return changedRect;
    }
}

QRect DrawTool::draw(const QPoint &point)
{
    if (mouseButton_ == Qt::RightButton) {
        return buffer_->pen()->erase(point, buffer_);
    } else {
        return buffer_->pen()->paint(point, buffer_);
    }
}

void DrawTool::registerTool()
{
    Tool::registerTool();

    setDrawMode(Dotted);
    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void DrawTool::activate()
{
    if (buffer_->tool() == this) {
        setDrawMode(static_cast<DrawMode>((drawMode + 1) % (FilledShape + 1)));
        button_->setChecked(true);
    }

    Tool::activate();
}

QWidget *DrawTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    w->setWindowTitle("Draw");

    QVBoxLayout *vbox = new QVBoxLayout(w);
    vbox->setSpacing(4);
    vbox->setContentsMargins(4, 4, 4, 4);

    vbox->addWidget(new QLabel("Pen Tip Size:", w));

    QHBoxLayout *row = new QHBoxLayout;
    for (int size : {1, 3, 5, 7}) {
        QPushButton *btn = new QPushButton(QString("%1px").arg(size), w);
        btn->setFixedSize(40, 24);
        connect(btn, &QPushButton::clicked, [this, size]() {
            PenTip *tip = qobject_cast<PenTip *>(buffer_->pen());
            if (tip) tip->setSize(size);
        });
        row->addWidget(btn);
    }
    vbox->addLayout(row);
    vbox->addStretch();
    return w;
}

void DrawTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 2);
}
