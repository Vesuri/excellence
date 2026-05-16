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
    if (!optionsWidget_) return;

    // Update range selector button highlights
    for (int i = 0; i < rangeButtons_.size(); i++) {
        rangeButtons_[i]->setDown(i == activeGradientRange);
        rangeButtons_[i]->setFlat(i != activeGradientRange);
    }

    GradientRange *range = &gradientRanges[activeGradientRange];

    if (markerBox_) {
        markerBox_->setRange(range);
        if (buffer_)
            markerBox_->setBuffer(buffer_);
    }

    if (spreadSpin_) {
        spreadSpin_->blockSignals(true);
        spreadSpin_->setValue(range->spread());
        spreadSpin_->blockSignals(false);
    }

    if (colorsLabel_)
        colorsLabel_->setText(QString("Colors: %1").arg(range->colorCount()));

    if (randomCheck_) {
        randomCheck_->blockSignals(true);
        randomCheck_->setChecked(range->random());
        randomCheck_->blockSignals(false);
    }
    if (hardEdgesCheck_) {
        hardEdgesCheck_->blockSignals(true);
        hardEdgesCheck_->setChecked(range->hardEdges());
        hardEdgesCheck_->blockSignals(false);
    }
    if (ditherSlider_) {
        ditherSlider_->blockSignals(true);
        ditherSlider_->setValue(range->ditherAmount());
        ditherSlider_->setEnabled(range->random());
        ditherSlider_->blockSignals(false);
    }

    if (cycleButton_) {
        cycleButton_->blockSignals(true);
        cycleButton_->setChecked(range->cycling());
        cycleButton_->blockSignals(false);
    }
    if (speedSlider_) {
        speedSlider_->blockSignals(true);
        speedSlider_->setValue(range->cycleSpeed());
        speedSlider_->blockSignals(false);
    }
}

void GradientTool::onRangeChanged()
{
    if (colorsLabel_)
        colorsLabel_->setText(
            QString("Colors: %1").arg(gradientRanges[activeGradientRange].colorCount()));
}

void GradientTool::setBuffer(Buffer *buffer)
{
    Tool::setBuffer(buffer);
    if (markerBox_)
        markerBox_->setBuffer(buffer);
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
    GradientRange *range = &gradientRanges[activeGradientRange];

    QWidget *w = new QWidget;
    w->setWindowTitle("Gradient Range Editor");
    w->setWindowFlags(Qt::Tool);

    QVBoxLayout *vbox = new QVBoxLayout(w);
    vbox->setSpacing(8);
    vbox->setContentsMargins(6, 6, 6, 6);

    // Range selector (1-8 buttons)
    QHBoxLayout *rangeRow = new QHBoxLayout;
    rangeRow->setSpacing(2);
    rangeButtons_.clear();
    for (int i = 0; i < kGradientRangeCount; i++) {
        QPushButton *btn = new QPushButton(QString::number(i + 1), w);
        btn->setFixedSize(28, 22);
        btn->setFlat(i != activeGradientRange);
        btn->setDown(i == activeGradientRange);
        int idx = i;
        connect(btn, &QPushButton::clicked, [this, idx]() { setActiveRange(idx); });
        rangeButtons_.append(btn);
        rangeRow->addWidget(btn);
    }
    rangeRow->addStretch();
    vbox->addLayout(rangeRow);

    // Marker box + preview
    markerBox_ = new GradientMarkerBox(w);
    markerBox_->setRange(range);
    if (buffer_)
        markerBox_->setBuffer(buffer_);
    connect(markerBox_, &GradientMarkerBox::rangeChanged, this, &GradientTool::onRangeChanged);
    vbox->addWidget(markerBox_);

    // COLORS label, SPREAD spinbox
    QHBoxLayout *infoRow = new QHBoxLayout;
    infoRow->setSpacing(6);
    colorsLabel_ = new QLabel(QString("Colors: %1").arg(range->colorCount()), w);
    infoRow->addWidget(colorsLabel_);

    infoRow->addStretch();

    infoRow->addWidget(new QLabel("Spread:", w));
    spreadSpin_ = new QSpinBox(w);
    spreadSpin_->setRange(0, 254);
    spreadSpin_->setValue(range->spread());
    spreadSpin_->setFixedWidth(60);
    connect(spreadSpin_, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        gradientRanges[activeGradientRange].setSpread(v);
        onRangeChanged();
    });
    infoRow->addWidget(spreadSpin_);
    vbox->addLayout(infoRow);

    // Dither controls
    QHBoxLayout *ditherRow = new QHBoxLayout;
    ditherRow->setSpacing(6);
    randomCheck_ = new QCheckBox("Random", w);
    randomCheck_->setChecked(range->random());
    connect(randomCheck_, &QCheckBox::toggled, [this](bool v) {
        gradientRanges[activeGradientRange].setRandom(v);
        if (ditherSlider_) ditherSlider_->setEnabled(v);
        if (markerBox_) markerBox_->update();
    });
    ditherRow->addWidget(randomCheck_);

    hardEdgesCheck_ = new QCheckBox("Hard Edges", w);
    hardEdgesCheck_->setChecked(range->hardEdges());
    connect(hardEdgesCheck_, &QCheckBox::toggled, [this](bool v) {
        gradientRanges[activeGradientRange].setHardEdges(v);
        if (markerBox_) markerBox_->update();
    });
    ditherRow->addWidget(hardEdgesCheck_);

    ditherRow->addStretch();
    ditherRow->addWidget(new QLabel("Dither:", w));

    ditherSlider_ = new QSlider(Qt::Horizontal, w);
    ditherSlider_->setRange(0, 100);
    ditherSlider_->setValue(range->ditherAmount());
    ditherSlider_->setFixedWidth(80);
    ditherSlider_->setEnabled(range->random());
    connect(ditherSlider_, &QSlider::valueChanged, [this](int v) {
        gradientRanges[activeGradientRange].setDitherAmount(v);
        if (markerBox_) markerBox_->update();
    });
    ditherRow->addWidget(ditherSlider_);
    vbox->addLayout(ditherRow);

    // Cycle controls
    QHBoxLayout *cycleRow = new QHBoxLayout;
    cycleRow->setSpacing(6);
    cycleButton_ = new QPushButton("Cycle", w);
    cycleButton_->setCheckable(true);
    cycleButton_->setChecked(range->cycling());
    cycleButton_->setFixedHeight(24);
    connect(cycleButton_, &QPushButton::toggled, [this](bool v) {
        gradientRanges[activeGradientRange].setCycling(v);
        cycleAccumulators_[activeGradientRange] = 0.0;
        updateTimer();
    });
    cycleRow->addWidget(cycleButton_);

    cycleRow->addStretch();
    cycleRow->addWidget(new QLabel("Speed:", w));

    speedSlider_ = new QSlider(Qt::Horizontal, w);
    speedSlider_->setRange(0, 71);
    speedSlider_->setValue(range->cycleSpeed());
    speedSlider_->setFixedWidth(80);
    connect(speedSlider_, &QSlider::valueChanged, [this](int v) {
        gradientRanges[activeGradientRange].setCycleSpeed(v);
        updateTimer();
    });
    cycleRow->addWidget(speedSlider_);
    vbox->addLayout(cycleRow);

    // Operation buttons
    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->setSpacing(4);

    auto makeBtn = [&](const QString &label, auto slot) {
        QPushButton *btn = new QPushButton(label, w);
        btn->setFixedHeight(24);
        connect(btn, &QPushButton::clicked, slot);
        btnRow->addWidget(btn);
    };

    makeBtn("Flip", [this]() {
        gradientRanges[activeGradientRange].flip();
        refreshPanel();
    });
    makeBtn("Clear", [this]() {
        gradientRanges[activeGradientRange].clear();
        refreshPanel();
    });
    makeBtn("Undo", [this]() {
        gradientRanges[activeGradientRange].undo();
        refreshPanel();
    });
    makeBtn("Restore", [this]() {
        gradientRanges[activeGradientRange].restore();
        refreshPanel();
    });

    vbox->addLayout(btnRow);

    return w;
}
