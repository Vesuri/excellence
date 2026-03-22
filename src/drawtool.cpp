#include <QImage>
#include <QLabel>
#include <QRect>
#include <QButtonGroup>
#include <QSlider>
#include <QSpinBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include "pen.h"
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
    buffer_->setSmearDirection(QPoint(0, 0));
    buffer_->resetCycle();
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
            buffer_->setSmearDirection(point - lastStampedPoint);
            lastStampedPoint = point;
            return draw(point);
        }
        return QRect();
    } else {
        QRect changedRect;
        buffer_->setSmearDirection(point - previousPoint);
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

    vbox->addWidget(new QLabel("Draw Mode:", w));

    static const struct { const char *label; Buffer::PaintMode mode; } kModes[] = {
        {"Normal",    Buffer::Normal},
        {"Replace",   Buffer::Replace},
        {"Smear",     Buffer::Smear},
        {"Smooth",    Buffer::Smooth},
        {"Range",     Buffer::Range},
        {"AvgSmear",  Buffer::AverageSmear},
        {"Cycle",     Buffer::Cycle},
        {"Random",    Buffer::Random},
        {"Tint",      Buffer::Tint},
        {"Colorize",  Buffer::Colorize},
        {"Brighten",  Buffer::Brighten},
        {"Darken",    Buffer::Darken},
        {"Mix",       Buffer::Mix},
        {"Negative",  Buffer::Negative},
        {"Dither1",   Buffer::Dither1},
        {"Dither2",   Buffer::Dither2},
        {"Transp.",   Buffer::Transparent},
    };
    QButtonGroup *modeGroup = new QButtonGroup(w);
    modeGroup->setExclusive(true);
    QGridLayout *modeGrid = new QGridLayout;
    for (int i = 0; i < 17; i++) {
        QPushButton *btn = new QPushButton(kModes[i].label, w);
        btn->setFixedSize(60, 24);
        btn->setCheckable(true);
        btn->setChecked(buffer_->paintMode() == kModes[i].mode);
        modeGroup->addButton(btn);
        Buffer::PaintMode m = kModes[i].mode;
        connect(btn, &QPushButton::clicked, [this, m]() { buffer_->setPaintMode(m); });
        modeGrid->addWidget(btn, i / 4, i % 4);
    }
    vbox->addLayout(modeGrid);

    vbox->addWidget(new QLabel("Amount (Dither/Brighten/Darken/Transp.):", w));
    QHBoxLayout *amountRow = new QHBoxLayout;
    QSlider *amountSlider = new QSlider(Qt::Horizontal, w);
    amountSlider->setRange(0, 100);
    amountSlider->setValue(buffer_->drawModeAmount());
    QSpinBox *amountSpin = new QSpinBox(w);
    amountSpin->setRange(0, 100);
    amountSpin->setValue(buffer_->drawModeAmount());
    amountSpin->setFixedWidth(48);
    connect(amountSlider, &QSlider::valueChanged, amountSpin, &QSpinBox::setValue);
    connect(amountSpin, QOverload<int>::of(&QSpinBox::valueChanged), amountSlider, &QSlider::setValue);
    connect(amountSlider, &QSlider::valueChanged, [this](int v) { buffer_->setDrawModeAmount(v); });
    amountRow->addWidget(amountSlider);
    amountRow->addWidget(amountSpin);
    vbox->addLayout(amountRow);

    vbox->addStretch();
    return w;
}

void DrawTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 2);
}
