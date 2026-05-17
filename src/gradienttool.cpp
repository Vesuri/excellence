#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include "buffer.h"
#include "gradienttool.h"
#include "ui_gradienttool.h"

GradientTool GradientTool::instance;

GradientTool::GradientTool() : Tool()
{
    cycleTimer_ = new QTimer(this);
    cycleTimer_->setInterval(14); // ~71 Hz
    connect(cycleTimer_, &QTimer::timeout, this, &GradientTool::onCycleTick);
}

void GradientTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/cyclegradient.png"));
    button_->setToolTip("Gradient / Color Cycling\nRight-click for options");
    button_->setCheckable(true);
    connect(button_, &QToolButton::clicked, this, &GradientTool::activate);
}

void GradientTool::activate()
{
    cyclingEnabled_ = !cyclingEnabled_;
    memset(cycleAccumulators_, 0, sizeof(cycleAccumulators_));
    updateTimer();
    syncButtonState();
    // Do NOT call Tool::activate() — this does not change the active drawing tool.
}

void GradientTool::toggle()
{
    activate();
}

void GradientTool::syncButtonState()
{
    if (button_)
        button_->setChecked(cyclingEnabled_);
}

void GradientTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 1);
}

void GradientTool::setActiveRange(int index)
{
    activeGradientRange = qBound(0, index, kGradientRangeCount - 1);
    refreshPanel();
}

void GradientTool::refreshPanel()
{
    if (!optionsWidget_ || !ui_) return;

    { QSignalBlocker b(ui_->rangeTabBar); ui_->rangeTabBar->setCurrentIndex(activeGradientRange); }

    GradientRange *range = &gradientRanges[activeGradientRange];

    ui_->markerBox->setRange(range);
    if (buffer_) ui_->markerBox->setBuffer(buffer_);

    { QSignalBlocker b(ui_->spreadSpin);    ui_->spreadSpin->setValue(range->spread()); }

    ui_->colorsGroup->setTitle(QString("Colors (%1)").arg(range->colorCount()));

    { QSignalBlocker b(ui_->randomCheck);   ui_->randomCheck->setChecked(range->random()); }
    { QSignalBlocker b(ui_->hardEdgesCheck); ui_->hardEdgesCheck->setChecked(range->hardEdges()); }
    {
        QSignalBlocker b1(ui_->ditherSlider), b2(ui_->ditherSpin);
        ui_->ditherSlider->setValue(range->ditherAmount());
        ui_->ditherSpin->setValue(range->ditherAmount());
        ui_->ditherSlider->setEnabled(range->random());
        ui_->ditherSpin->setEnabled(range->random());
    }
    { QSignalBlocker b(ui_->cycleButton);   ui_->cycleButton->setChecked(range->cycling()); }
    {
        QSignalBlocker b1(ui_->speedSlider), b2(ui_->speedSpin);
        ui_->speedSlider->setValue(range->cycleSpeed());
        ui_->speedSpin->setValue(range->cycleSpeed());
    }
}

void GradientTool::onRangeChanged()
{
    if (ui_)
        ui_->colorsGroup->setTitle(
            QString("Colors (%1)").arg(gradientRanges[activeGradientRange].colorCount()));
}

void GradientTool::setBuffer(Buffer *buffer)
{
    bool firstBuffer = (buffer_ == nullptr);
    Tool::setBuffer(buffer);
    if (ui_)
        ui_->markerBox->setBuffer(buffer);
    if (firstBuffer && buffer)
        gradientRanges[0].setDefault(buffer->image());
}

void GradientTool::updateTimer()
{
    bool needed = false;
    if (cyclingEnabled_) {
        for (int r = 0; r < kGradientRangeCount; r++) {
            if (gradientRanges[r].cycling() && gradientRanges[r].cycleSpeed() > 0) {
                needed = true;
                break;
            }
        }
    }
    if (needed && !cycleTimer_->isActive())
        cycleTimer_->start();
    else if (!needed && cycleTimer_->isActive())
        cycleTimer_->stop();
}

void GradientTool::onCycleTick()
{
    if (!buffer_) return;
    for (int r = 0; r < kGradientRangeCount; r++) {
        GradientRange &range = gradientRanges[r];
        if (!range.cycling() || range.cycleSpeed() == 0) continue;

        cycleAccumulators_[r] += range.cycleSpeed() / 71.0;
        if (cycleAccumulators_[r] < 1.0) continue;
        cycleAccumulators_[r] -= 1.0;

        const QVector<GradientMarker> &markers = range.markers();
        if (markers.size() < 2) continue;

        int n = markers.size();
        QRgb saved = buffer_->image().color(markers[n - 1].colorIndex);
        for (int i = n - 1; i > 0; i--)
            buffer_->setColor(markers[i].colorIndex, QColor(buffer_->image().color(markers[i - 1].colorIndex)));
        buffer_->setColor(markers[0].colorIndex, QColor(saved));
    }
}

QWidget *GradientTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    delete ui_;
    ui_ = new Ui::GradientToolOptions;
    ui_->setupUi(w);

    for (int i = 0; i < kGradientRangeCount; i++)
        ui_->rangeTabBar->addTab(QString::number(i + 1));
    connect(ui_->rangeTabBar, &QTabBar::currentChanged, this, &GradientTool::setActiveRange);

    connect(ui_->markerBox, &GradientMarkerBox::rangeChanged, this, &GradientTool::onRangeChanged);

    connect(ui_->spreadSpin, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        gradientRanges[activeGradientRange].setSpread(v);
        onRangeChanged();
    });

    connect(ui_->randomCheck, &QCheckBox::toggled, [this](bool v) {
        gradientRanges[activeGradientRange].setRandom(v);
        ui_->ditherSlider->setEnabled(v);
        ui_->ditherSpin->setEnabled(v);
        ui_->markerBox->update();
    });

    connect(ui_->hardEdgesCheck, &QCheckBox::toggled, [this](bool v) {
        gradientRanges[activeGradientRange].setHardEdges(v);
        ui_->markerBox->update();
    });

    connect(ui_->ditherSlider, &QSlider::valueChanged, [this](int v) {
        QSignalBlocker b(ui_->ditherSpin);
        ui_->ditherSpin->setValue(v);
        gradientRanges[activeGradientRange].setDitherAmount(v);
        ui_->markerBox->update();
    });
    connect(ui_->ditherSpin, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        QSignalBlocker b(ui_->ditherSlider);
        ui_->ditherSlider->setValue(v);
        gradientRanges[activeGradientRange].setDitherAmount(v);
        ui_->markerBox->update();
    });

    connect(ui_->cycleButton, &QCheckBox::toggled, [this](bool v) {
        gradientRanges[activeGradientRange].setCycling(v);
        cycleAccumulators_[activeGradientRange] = 0.0;
        updateTimer();
    });

    connect(ui_->speedSlider, &QSlider::valueChanged, [this](int v) {
        QSignalBlocker b(ui_->speedSpin);
        ui_->speedSpin->setValue(v);
        gradientRanges[activeGradientRange].setCycleSpeed(v);
        updateTimer();
    });
    connect(ui_->speedSpin, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        QSignalBlocker b(ui_->speedSlider);
        ui_->speedSlider->setValue(v);
        gradientRanges[activeGradientRange].setCycleSpeed(v);
        updateTimer();
    });

    connect(ui_->flipBtn, &QPushButton::clicked, [this]() {
        gradientRanges[activeGradientRange].flip();
        refreshPanel();
    });
    connect(ui_->clearBtn, &QPushButton::clicked, [this]() {
        gradientRanges[activeGradientRange].clear();
        refreshPanel();
    });
    connect(ui_->undoBtn, &QPushButton::clicked, [this]() {
        gradientRanges[activeGradientRange].undo();
        refreshPanel();
    });
    connect(ui_->restoreBtn, &QPushButton::clicked, [this]() {
        gradientRanges[activeGradientRange].restore();
        refreshPanel();
    });

    optionsWidget_ = w;
    refreshPanel();
    return w;
}
