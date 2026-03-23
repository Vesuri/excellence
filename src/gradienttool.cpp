#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include "buffer.h"
#include "gradienttool.h"

GradientTool GradientTool::instance;

GradientTool::GradientTool() : Tool()
{
}

void GradientTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/cyclegradient.png"));
    button_->setToolTip("Gradient Range Editor");
    button_->setCheckable(false);
    connect(button_, &QToolButton::clicked, this, &GradientTool::activate);
}

void GradientTool::activate()
{
    toggleOptionsWidget();
}

void GradientTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 0);
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
        colorsLabel_->setText(QString("COLORS: %1").arg(range->colorCount()));
}

void GradientTool::onRangeChanged()
{
    if (colorsLabel_)
        colorsLabel_->setText(
            QString("COLORS: %1").arg(gradientRanges[activeGradientRange].colorCount()));
}

void GradientTool::setBuffer(Buffer *buffer)
{
    Tool::setBuffer(buffer);
    if (markerBox_)
        markerBox_->setBuffer(buffer);
}

QWidget *GradientTool::createOptionsWidget()
{
    GradientRange *range = &gradientRanges[activeGradientRange];

    QWidget *w = new QWidget;
    w->setWindowTitle("Gradient Range Editor");
    w->setWindowFlags(Qt::Tool);

    QVBoxLayout *vbox = new QVBoxLayout(w);
    vbox->setSpacing(4);
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

    // SHOW checkbox, COLORS label, SPREAD spinbox
    QHBoxLayout *infoRow = new QHBoxLayout;
    QCheckBox *showCheck = new QCheckBox("SHOW", w);
    showCheck->setChecked(true);
    connect(showCheck, &QCheckBox::toggled, markerBox_, &GradientMarkerBox::setShowPreview);
    infoRow->addWidget(showCheck);

    colorsLabel_ = new QLabel(QString("COLORS: %1").arg(range->colorCount()), w);
    infoRow->addWidget(colorsLabel_);

    infoRow->addStretch();

    infoRow->addWidget(new QLabel("SPREAD:", w));
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
