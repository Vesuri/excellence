#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QRandomGenerator>
#include <QtCore/qmath.h>
#include <cmath>
#include "pen.h"
#include "brush.h"
#include "buffer.h"
#include "airtool.h"

AirTool AirTool::instance;
const char *AirTool::icons[] = {
    ":/finesprayairbrush.png",
    ":/splatter.png",
    ":/shapeairbrush.png"
};

AirTool::AirTool(QObject *parent) : Tool(parent),
    sprayMode_(FineSpray), erasing_(false),
    timer_(new QTimer(this)),
    nozzleRadius_(20), flow_(50), focus_(0)
{
    timer_->setInterval(16);
    connect(timer_, &QTimer::timeout, this, &AirTool::sprayTick);
}

void AirTool::setMode(SprayMode mode)
{
    sprayMode_ = mode;
    button_->setIcon(QIcon(icons[mode]));
}

void AirTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr)
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    timer_->stop();
    Tool::setBuffer(buffer);
    if (buffer_ != nullptr)
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
}

// ── Spray helpers ──────────────────────────────────────────────────────────

QPoint AirTool::randomNozzlePoint() const
{
    QRandomGenerator *rng = QRandomGenerator::global();
    double theta = rng->generateDouble() * 2.0 * M_PI;
    double spread = 1.0 - focus_ / 100.0;
    double r = nozzleRadius_ * spread * std::sqrt(rng->generateDouble());
    return QPoint(center_.x() + qRound(r * std::cos(theta)),
                  center_.y() + qRound(r * std::sin(theta)));
}

QRect AirTool::paintDot(const QPoint &point)
{
    QImage &img = buffer_->image();
    if (!img.rect().contains(point))
        return QRect();

    switch (sprayMode_) {
    case FineSpray:
        img.setPixel(point.x(), point.y(),
                     erasing_ ? buffer_->eraseColor() : buffer_->paintColor());
        return QRect(point, point);

    case Splatter:
        if (erasing_)
            return buffer_->pen()->erase(point, buffer_);
        return buffer_->pen()->paint(point, buffer_);

    case ShapeAirbrush: {
        Brush *brush = qobject_cast<Brush *>(buffer_->pen());
        if (!brush) {
            // No brush loaded — fall back to fine spray
            img.setPixel(point.x(), point.y(),
                         erasing_ ? buffer_->eraseColor() : buffer_->paintColor());
            return QRect(point, point);
        }
        // Pick a random pixel within the brush and paint it if non-background
        const QImage &bimg = brush->image();
        int bx = QRandomGenerator::global()->bounded(bimg.width());
        int by = QRandomGenerator::global()->bounded(bimg.height());
        if (bimg.pixelIndex(bx, by) != 0) {
            QPoint cp(point.x() + bx - bimg.width() / 2,
                      point.y() + by - bimg.height() / 2);
            if (img.rect().contains(cp)) {
                img.setPixel(cp.x(), cp.y(),
                             erasing_ ? buffer_->eraseColor() : buffer_->paintColor());
                return QRect(cp, cp);
            }
        }
        return QRect();
    }
    }
    return QRect();
}

QRect AirTool::sprayDots()
{
    int dots = qMax(1, flow_ * qMax(1, nozzleRadius_) / 100);
    QRect changedRect;
    for (int i = 0; i < dots; i++)
        changedRect = changedRect.united(paintDot(randomNozzlePoint()));
    return changedRect;
}

void AirTool::sprayTick()
{
    if (mouseButton_ == Qt::NoButton || buffer_ == nullptr)
        return;
    QRect r = sprayDots();
    if (!r.isEmpty())
        buffer_->notifyModified(r);
}

// ── press / move / release ─────────────────────────────────────────────────

QRect AirTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    erasing_ = (mouseButton_ == Qt::RightButton);
    center_ = point;
    timer_->start();
    return sprayDots();
}

QRect AirTool::move(const QPoint &point)
{
    if (mouseButton_ == Qt::NoButton)
        return QRect();
    center_ = point;
    return sprayDots();
}

QRect AirTool::release(const QPoint &)
{
    timer_->stop();
    return QRect();
}

// ── options window ─────────────────────────────────────────────────────────

QWidget* AirTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    w->setWindowTitle("Airbrush");
    QVBoxLayout *vl = new QVBoxLayout(w);

    auto addRow = [&](const QString &label, QWidget *ctrl) {
        QHBoxLayout *hl = new QHBoxLayout;
        hl->addWidget(new QLabel(label));
        hl->addWidget(ctrl);
        vl->addLayout(hl);
    };

    QSpinBox *sizeBox = new QSpinBox;
    sizeBox->setRange(1, 200);
    sizeBox->setValue(nozzleRadius_);
    sizeBox->setSuffix(" px");
    connect(sizeBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int v) { nozzleRadius_ = v; });
    addRow("Size:", sizeBox);

    QSlider *flowSlider = new QSlider(Qt::Horizontal);
    flowSlider->setRange(0, 100);
    flowSlider->setValue(flow_);
    connect(flowSlider, &QSlider::valueChanged,
            [this](int v) { flow_ = v; });
    addRow("Flow:", flowSlider);

    QSlider *focusSlider = new QSlider(Qt::Horizontal);
    focusSlider->setRange(0, 100);
    focusSlider->setValue(focus_);
    connect(focusSlider, &QSlider::valueChanged,
            [this](int v) { focus_ = v; });
    addRow("Focus:", focusSlider);

    return w;
}

void AirTool::setSprayMode(SprayMode mode)
{
    setMode(mode);
}

// ── Tool registration ──────────────────────────────────────────────────────

void AirTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/finesprayairbrush.png"));
    button_->setCheckable(true);
    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void AirTool::activate()
{
    timer_->stop();
    if (buffer_->tool() == this) {
        setMode(static_cast<SprayMode>((sprayMode_ + 1) % (ShapeAirbrush + 1)));
        button_->setChecked(true);
    }
    Tool::activate();
}

void AirTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 3);
}
