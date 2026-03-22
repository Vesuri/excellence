#include <QButtonGroup>
#include <QGridLayout>
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

DrawModeTool DrawModeTool::instance;

DrawModeTool::DrawModeTool(QObject *parent) : Tool(parent),
    selectedMode_(Buffer::Normal)
{
}

void DrawModeTool::setBuffer(Buffer *buffer)
{
    Tool::setBuffer(buffer);
    applyMode();
}

void DrawModeTool::applyMode()
{
    if (!buffer_)
        return;
    buffer_->setPaintMode(button_->isChecked() ? selectedMode_ : Buffer::Normal);
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

    QButtonGroup *group = new QButtonGroup(w);
    group->setExclusive(true);
    QGridLayout *grid = new QGridLayout;
    for (int i = 0; i < 17; i++) {
        QPushButton *btn = new QPushButton(kModes[i].label, w);
        btn->setFixedSize(72, 24);
        btn->setCheckable(true);
        btn->setChecked(selectedMode_ == kModes[i].mode);
        group->addButton(btn);
        Buffer::PaintMode m = kModes[i].mode;
        connect(btn, &QPushButton::clicked, [this, m]() {
            selectedMode_ = m;
            applyMode();
        });
        grid->addWidget(btn, i / 4, i % 4);
    }
    vbox->addLayout(grid);

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
    layout->addWidget(button_, 1, 12);
}
