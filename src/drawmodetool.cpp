#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include "buffer.h"
#include "drawmodetool.h"
#include "gradientrange.h"

DrawModeTool DrawModeTool::instance;

DrawModeTool::DrawModeTool(QObject *parent) : Tool(parent),
    selectedMode_(Buffer::Normal)
{
}

void DrawModeTool::setBuffer(Buffer *buffer)
{
    if (buffer_)
        disconnect(buffer_, &Buffer::toolChanged, this, &DrawModeTool::onToolChanged);
    Tool::setBuffer(buffer);
    if (buffer_)
        connect(buffer_, &Buffer::toolChanged, this, &DrawModeTool::onToolChanged);
    applyMode();
}

void DrawModeTool::applyMode()
{
    if (!buffer_)
        return;
    bool active = button_->isChecked();
    buffer_->setPaintMode(active ? selectedMode_ : Buffer::Normal);
    drawModeActive = active;
}

void DrawModeTool::onToolChanged(Tool *tool)
{
    if (fillGroupWidget_)
        fillGroupWidget_->setEnabled(tool && tool->hasFill());
}

void DrawModeTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/drawmode.png"));
    button_->setToolTip("Draw Mode");
    button_->setCheckable(true);
    connect(button_, &QToolButton::clicked, this, &DrawModeTool::activate);
}

void DrawModeTool::activate()
{
    // Toggle the active state; do NOT call Tool::activate() — the active
    // drawing tool must not change when the draw mode button is pressed.
    applyMode();
}

QWidget *DrawModeTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    w->setWindowTitle("Draw Mode");

    QVBoxLayout *vbox = new QVBoxLayout(w);
    vbox->setSpacing(4);
    vbox->setContentsMargins(4, 4, 4, 4);

    QHBoxLayout *topRow = new QHBoxLayout;
    topRow->setSpacing(8);
    topRow->setAlignment(Qt::AlignTop);

    // Draw mode buttons
    static const struct { const char *label; Buffer::PaintMode mode; } kModes[] = {
        {"Color",      Buffer::Normal},
        {"Replace",    Buffer::Replace},
        {"Smear",      Buffer::Smear},
        {"Smooth",     Buffer::Smooth},
        {"Range",      Buffer::Range},
        {"Avg Smear",  Buffer::AverageSmear},
        {"Cycle",      Buffer::Cycle},
        {"Random",     Buffer::Random},
        {"Tint",       Buffer::Tint},
        {"Colorize",   Buffer::Colorize},
        {"Brighten",   Buffer::Brighten},
        {"Darken",     Buffer::Darken},
        {"Mix",        Buffer::Mix},
        {"Negative",   Buffer::Negative},
        {"Dither 1",   Buffer::Dither1},
        {"Dither 2",   Buffer::Dither2},
        {"Transparent",Buffer::Transparent},
    };

    QButtonGroup *modeGroup = new QButtonGroup(w);
    modeGroup->setExclusive(true);
    QGridLayout *modeGrid = new QGridLayout;
    modeGrid->setSpacing(2);
    for (int i = 0; i < 17; i++) {
        QPushButton *btn = new QPushButton(kModes[i].label, w);
        btn->setFixedSize(72, 24);
        btn->setCheckable(true);
        btn->setChecked(selectedMode_ == kModes[i].mode);
        modeGroup->addButton(btn);
        Buffer::PaintMode m = kModes[i].mode;
        connect(btn, &QPushButton::clicked, [this, m]() {
            selectedMode_ = m;
            applyMode();
        });
        modeGrid->addWidget(btn, i / 4, i % 4);
    }
    topRow->addLayout(modeGrid);

    // Gradient fill section
    QGroupBox *fillGroup = new QGroupBox("Gradient Fill", w);
    fillGroupWidget_ = fillGroup;
    bool hasFill = buffer_ && buffer_->tool() && buffer_->tool()->hasFill();
    fillGroup->setEnabled(hasFill);

    static const struct { const char *label; GradientFillMode mode; } kFillModes[] = {
        {"Horizontal", FillHorizontal},
        {"Vertical",   FillVertical},
        {"Linear",     FillLinear},
        {"Radial",     FillRadial},
        {"Spherical",  FillSpherical},
    };

    QButtonGroup *fillModeGroup = new QButtonGroup(w);
    fillModeGroup->setExclusive(true);
    QVBoxLayout *fillVbox = new QVBoxLayout(fillGroup);
    fillVbox->setSpacing(2);
    for (int i = 0; i < 5; i++) {
        QPushButton *btn = new QPushButton(kFillModes[i].label, fillGroup);
        btn->setFixedSize(80, 24);
        btn->setCheckable(true);
        btn->setChecked(activeGradientFillMode == kFillModes[i].mode);
        fillModeGroup->addButton(btn);
        GradientFillMode fm = kFillModes[i].mode;
        connect(btn, &QPushButton::clicked, [fm]() {
            activeGradientFillMode = fm;
        });
        fillVbox->addWidget(btn);
    }
    topRow->addWidget(fillGroup);
    topRow->addStretch();

    vbox->addLayout(topRow);

    vbox->addWidget(new QLabel("Amount (Dither/Brighten/Darken/Transparent):", w));
    QHBoxLayout *amountRow = new QHBoxLayout;
    QSlider *slider = new QSlider(Qt::Horizontal, w);
    slider->setRange(0, 100);
    slider->setValue(buffer_->drawModeAmount());
    QSpinBox *spin = new QSpinBox(w);
    spin->setRange(0, 100);
    spin->setValue(buffer_->drawModeAmount());
    spin->setFixedWidth(48);
    connect(slider, &QSlider::valueChanged, spin, &QSpinBox::setValue);
    connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), slider, &QSlider::setValue);
    connect(slider, &QSlider::valueChanged, [this](int v) { buffer_->setDrawModeAmount(v); });
    amountRow->addWidget(slider);
    amountRow->addWidget(spin);
    vbox->addLayout(amountRow);

    vbox->addStretch();
    return w;
}

void DrawModeTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 10);
}
