#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QRandomGenerator>
#include "ui_airbrushtool.h"
#include <QtCore/qmath.h>
#include <cmath>
#include "pen.h"
#include "pentip.h"
#include "brush.h"
#include "buffer.h"
#include "airbrushtool.h"

AirbrushTool AirbrushTool::instance;
const char *AirbrushTool::icons[] = {
    ":/finesprayairbrush.png",
    ":/splatter.png",
    ":/shapeairbrush.png"
};

AirbrushTool::AirbrushTool(QObject *parent) : Tool(parent),
    sprayMode_(FineSpray), erasing_(false),
    timer_(new QTimer(this)),
    nozzleRadius_(20), flow_(50), focus_(0)
{
    timer_->setInterval(16);
    connect(timer_, &QTimer::timeout, this, &AirbrushTool::sprayTick);
}

void AirbrushTool::setMode(SprayMode mode)
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

QString AirbrushTool::name() const
{
    switch (sprayMode_) {
    case FineSpray:     return "Fine Spray Airbrush";
    case Splatter:      return "Splatter";
    case ShapeAirbrush: return "Shape Airbrush";
    }
    return QString();
}

void AirbrushTool::setBuffer(Buffer *buffer)
{
    disconnectToolChecked();
    timer_->stop();
    sprayBrush_ = nullptr;
    sprayTip_ = nullptr;
    Tool::setBuffer(buffer);
    connectToolChecked();
}

// ── Spray helpers ──────────────────────────────────────────────────────────

QPoint AirbrushTool::randomNozzlePoint() const
{
    QRandomGenerator *rng = QRandomGenerator::global();
    double theta = rng->generateDouble() * 2.0 * M_PI;
    double spread = 1.0 - focus_ / 100.0;
    double r = nozzleRadius_ * spread * std::sqrt(rng->generateDouble());
    return QPoint(center_.x() + qRound(r * std::cos(theta)),
                  center_.y() + qRound(r * std::sin(theta)));
}

QRect AirbrushTool::paintDot(const QPoint &point)
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
        auto *rng = QRandomGenerator::global();

        if (sprayBrush_) {
            const QImage &bimg = sprayBrush_->image();
            for (int attempt = 0; attempt < 16; attempt++) {
                int bx = rng->bounded(bimg.width());
                int by = rng->bounded(bimg.height());
                if (bimg.pixelIndex(bx, by) == sprayBrush_->transparentIndex())
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

        if (sprayTip_) {
            if (sprayTip_->width() == 1 && sprayTip_->height() == 1) {
                img.setPixel(point, color);
                return QRect(point, point);
            }
            const int hw = sprayTip_->width() / 2, hh = sprayTip_->height() / 2;
            for (int attempt = 0; attempt < 16; attempt++) {
                int dx = static_cast<int>(rng->bounded(2 * hw + 1)) - hw;
                int dy = static_cast<int>(rng->bounded(2 * hh + 1)) - hh;
                if (sprayTip_->shape() == PenTip::Circle && dx * dx + dy * dy > hw * hw + hw / 2)
                    continue;
                QPoint cp(point.x() + dx, point.y() + dy);
                if (img.rect().contains(cp)) {
                    img.setPixel(cp, color);
                    return QRect(cp, cp);
                }
            }
            return QRect();
        }

        img.setPixel(point, color);
        return QRect(point, point);
    }
    }
    return QRect();
}

QRect AirbrushTool::sprayDots()
{
    int dots = qMax(1, flow_ * qMax(1, nozzleRadius_) / 100);
    QRect changedRect;
    for (int i = 0; i < dots; i++) {
        QPoint p = (sprayMode_ == ShapeAirbrush) ? center_ : randomNozzlePoint();
        changedRect = changedRect.united(paintDot(p));
    }
    return changedRect;
}

void AirbrushTool::sprayTick()
{
    if (mouseButton_ == Qt::NoButton || buffer_ == nullptr)
        return;
    QRect r = sprayDots();
    if (!r.isEmpty())
        buffer_->notifyModified(r);
}

// ── press / move / release ─────────────────────────────────────────────────

QRect AirbrushTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    erasing_ = (mouseButton_ == Qt::RightButton);
    if (sprayMode_ == ShapeAirbrush) {
        sprayBrush_ = qobject_cast<Brush *>(buffer_->pen());
        sprayTip_ = sprayBrush_ ? nullptr : qobject_cast<PenTip *>(buffer_->pen());
    }
    center_ = point;
    timer_->start();
    return sprayDots();
}

QRect AirbrushTool::move(const QPoint &point)
{
    if (mouseButton_ == Qt::NoButton)
        return QRect();
    center_ = point;
    return sprayDots();
}

QRect AirbrushTool::release(const QPoint &)
{
    timer_->stop();
    sprayBrush_ = nullptr;
    sprayTip_ = nullptr;
    return QRect();
}

// ── options window ─────────────────────────────────────────────────────────

QWidget* AirbrushTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    ui_ = new Ui::AirbrushToolOptions;
    ui_->setupUi(w);
    ui_->sizeBox->setValue(nozzleRadius_);
    ui_->flowSlider->setValue(flow_);
    ui_->focusSlider->setValue(focus_);
    connect(ui_->sizeBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int v) { nozzleRadius_ = v; });
    connect(ui_->flowSlider, &QSlider::valueChanged,
            [this](int v) { flow_ = v; });
    connect(ui_->focusSlider, &QSlider::valueChanged,
            [this](int v) { focus_ = v; });
    return w;
}

// ── Tool registration ──────────────────────────────────────────────────────

void AirbrushTool::registerTool()
{
    Tool::registerTool();
    button_->setCheckable(true);
    setMode(sprayMode_);
    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void AirbrushTool::activate()
{
    timer_->stop();
    if (buffer_->tool() == this) {
        setMode(static_cast<SprayMode>((sprayMode_ + 1) % (ShapeAirbrush + 1)));
        button_->setChecked(true);
    }
    Tool::activate();
}

void AirbrushTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 7);
}
