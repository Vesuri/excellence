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
#include "pentip.h"
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

    static const char *tips[] = {
        "Airbrush – Fine Spray [A]  Shift+A: options",
        "Airbrush – Splatter [A]  Shift+A: options",
        "Airbrush – Shape [A]  Shift+A: options"
    };
    button_->setToolTip(tips[mode]);
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
        const uint color = erasing_ ? buffer_->eraseColor() : buffer_->paintColor();

        if (Brush *brush = qobject_cast<Brush *>(buffer_->pen())) {
            const QImage &bimg = brush->image();
            for (int attempt = 0; attempt < 16; attempt++) {
                int bx = QRandomGenerator::global()->bounded(bimg.width());
                int by = QRandomGenerator::global()->bounded(bimg.height());
                if (bimg.pixelIndex(bx, by) == 0)
                    continue;
                QPoint cp(point.x() + bx - bimg.width() / 2,
                          point.y() + by - bimg.height() / 2);
                if (img.rect().contains(cp)) {
                    img.setPixel(cp, color);
                    return QRect(cp, cp);
                }
            }
            return QRect();
        }

        if (PenTip *tip = qobject_cast<PenTip *>(buffer_->pen())) {
            if (tip->width() == 1 && tip->height() == 1) {
                if (!img.rect().contains(point)) return QRect();
                img.setPixel(point, color);
                return QRect(point, point);
            }
            const int hw = tip->width() / 2, hh = tip->height() / 2;
            for (int attempt = 0; attempt < 16; attempt++) {
                int dx = static_cast<int>(QRandomGenerator::global()->bounded(2 * hw + 1)) - hw;
                int dy = static_cast<int>(QRandomGenerator::global()->bounded(2 * hh + 1)) - hh;
                if (tip->shape() == PenTip::Circle && dx * dx + dy * dy > hw * hw + hw / 2)
                    continue;
                QPoint cp(point.x() + dx, point.y() + dy);
                if (img.rect().contains(cp)) {
                    img.setPixel(cp, color);
                    return QRect(cp, cp);
                }
            }
            return QRect();
        }

        // Fallback: fine spray
        if (img.rect().contains(point)) {
            img.setPixel(point, color);
            return QRect(point, point);
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
    for (int i = 0; i < dots; i++) {
        QPoint p = (sprayMode_ == ShapeAirbrush) ? center_ : randomNozzlePoint();
        changedRect = changedRect.united(paintDot(p));
    }
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
    button_->setCheckable(true);
    setMode(sprayMode_);
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
    layout->addWidget(button_, 0, 7);
}
