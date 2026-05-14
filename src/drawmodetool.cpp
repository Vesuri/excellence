#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include "brush.h"
#include "buffer.h"
#include "drawmodetool.h"
#include "gradientrange.h"
#include "pentip.h"

DrawModeTool DrawModeTool::instance;

DrawModeTool::DrawModeTool(QObject *parent) : Tool(parent),
    selectedMode_(Buffer::Normal)
{
}

void DrawModeTool::setBuffer(Buffer *buffer)
{
    if (buffer_) {
        disconnect(buffer_, &Buffer::toolChanged, this, &DrawModeTool::onToolChanged);
        disconnect(buffer_, &Buffer::paintModeChanged, this, &DrawModeTool::onPaintModeChanged);
        disconnect(buffer_, &Buffer::penChanged, this, &DrawModeTool::onPenChanged);
    }
    Tool::setBuffer(buffer);
    if (buffer_) {
        connect(buffer_, &Buffer::toolChanged, this, &DrawModeTool::onToolChanged);
        connect(buffer_, &Buffer::paintModeChanged, this, &DrawModeTool::onPaintModeChanged);
        connect(buffer_, &Buffer::penChanged, this, &DrawModeTool::onPenChanged);
    }
    applyMode();
}

void DrawModeTool::onToolChanged(Tool *tool)
{
    if (fillGroupWidget_)
        fillGroupWidget_->setEnabled(tool && tool->hasFill());
}

void DrawModeTool::onPaintModeChanged(Buffer::PaintMode mode)
{
    if (mode != Buffer::Normal) {
        selectedMode_ = mode;
        button_->setChecked(true);
        drawModeActive = true;
    } else {
        button_->setChecked(false);
        drawModeActive = false;
    }
    emit selectedModeChanged(selectedMode_);
}

void DrawModeTool::onPenChanged(Pen *pen)
{
    bool brushActive = qobject_cast<Brush *>(pen) != nullptr;
    emit brushModeAvailableChanged(brushActive);
    if (!brushActive && buffer_->paintMode() == Buffer::BrushMode) {
        selectedMode_ = Buffer::Normal;
        buffer_->setPaintMode(Buffer::Normal);
    }
}

void DrawModeTool::applyMode()
{
    if (!buffer_)
        return;
    bool active = button_->isChecked();
    buffer_->setPaintMode(active ? selectedMode_ : Buffer::Normal);
    drawModeActive = active;
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

    QButtonGroup *modeGroup = new QButtonGroup(w);
    modeGroup->setExclusive(true);

    auto makeMode = [&](const char *label, Buffer::PaintMode mode, const char *tooltip = nullptr) -> QRadioButton * {
        QRadioButton *btn = new QRadioButton(label, w);
        if (tooltip) btn->setToolTip(tooltip);
        btn->setChecked(selectedMode_ == mode);
        modeGroup->addButton(btn);
        if (mode == Buffer::BrushMode) {
            btn->setEnabled(qobject_cast<Brush *>(buffer_->pen()) != nullptr);
            connect(this, &DrawModeTool::brushModeAvailableChanged, btn, &QWidget::setEnabled);
        }
        connect(btn, &QRadioButton::clicked, [this, mode]() {
            selectedMode_ = mode;
            button_->setChecked(mode != Buffer::Normal);
            applyMode();
        });
        connect(this, &DrawModeTool::selectedModeChanged, btn, [btn, mode](Buffer::PaintMode active) {
            btn->setChecked(active == mode);
        });
        return btn;
    };

    auto makeStub = [&](const char *label, const char *tooltip = nullptr) -> QRadioButton * {
        QRadioButton *btn = new QRadioButton(label, w);
        if (tooltip) btn->setToolTip(tooltip);
        btn->setEnabled(false);
        return btn;
    };

    auto addHSep = [&](QVBoxLayout *col) {
        QFrame *sep = new QFrame(w);
        sep->setFrameShape(QFrame::HLine);
        sep->setFrameShadow(QFrame::Sunken);
        col->addWidget(sep);
    };

    auto addVSep = [&](QHBoxLayout *row) {
        QFrame *sep = new QFrame(w);
        sep->setFrameShape(QFrame::VLine);
        sep->setFrameShadow(QFrame::Sunken);
        row->addWidget(sep);
    };

    QHBoxLayout *mainRow = new QHBoxLayout(w);
    mainRow->setSpacing(4);
    mainRow->setContentsMargins(4, 4, 4, 4);
    mainRow->setAlignment(Qt::AlignTop);

    // ── Col 1: Brush | sep | transforms | sep | Replace ──────────────
    QVBoxLayout *col1 = new QVBoxLayout;
    col1->setSpacing(8);
    col1->addWidget(makeMode("Brush",       Buffer::BrushMode, "Brush draw mode"));
    addHSep(col1);
    col1->addWidget(makeStub("Stretch",    "Brush fill mode"));
    col1->addWidget(makeStub("Pattern",    "Brush fill mode"));
    col1->addWidget(makeStub("Shape",      "Brush fill mode"));
    col1->addWidget(makeStub("Perspective","Brush fill mode"));
    col1->addStretch();
    addHSep(col1);
    col1->addWidget(makeMode("Replace",    Buffer::Replace, "Replace draw brush"));
    mainRow->addLayout(col1);

    addVSep(mainRow);

    // ── Middle: cols 2-4 side by side + Amount at bottom ─────────────
    QVBoxLayout *middleVBox = new QVBoxLayout;
    middleVBox->setSpacing(2);

    QHBoxLayout *modeCols = new QHBoxLayout;
    modeCols->setSpacing(8);
    modeCols->setAlignment(Qt::AlignTop);

    // Col 2
    QVBoxLayout *col2 = new QVBoxLayout;
    col2->setSpacing(8);
    col2->setAlignment(Qt::AlignTop);
    static constexpr const char *kGeneral = "General draw mode";
    col2->addWidget(makeMode("Color",    Buffer::Normal,       kGeneral));
    col2->addWidget(makeMode("Tint",     Buffer::Tint,         kGeneral));
    col2->addWidget(makeMode("Colorize", Buffer::Colorize,     kGeneral));
    col2->addWidget(makeMode("Brighten", Buffer::Brighten,     kGeneral));
    col2->addWidget(makeMode("Darken",   Buffer::Darken,       kGeneral));
    col2->addWidget(makeStub("Stencil",                        kGeneral));
    modeCols->addLayout(col2);

    // Col 3
    QVBoxLayout *col3 = new QVBoxLayout;
    col3->setSpacing(8);
    col3->setAlignment(Qt::AlignTop);
    col3->addWidget(makeMode("Mix",       Buffer::Mix,          kGeneral));
    col3->addWidget(makeMode("Smooth",    Buffer::Smooth,       kGeneral));
    col3->addWidget(makeMode("Smear",     Buffer::Smear,        kGeneral));
    col3->addWidget(makeMode("Avg Smear", Buffer::AverageSmear, kGeneral));
    col3->addWidget(makeMode("Range",     Buffer::Range,        kGeneral));
    col3->addWidget(makeMode("Cycle",     Buffer::Cycle,        kGeneral));
    modeCols->addLayout(col3);

    // Col 4
    QVBoxLayout *col4 = new QVBoxLayout;
    col4->setSpacing(8);
    col4->setAlignment(Qt::AlignTop);
    col4->addWidget(makeMode("Random",      Buffer::Random,      kGeneral));
    col4->addWidget(makeMode("Dither 1",    Buffer::Dither1,     kGeneral));
    col4->addWidget(makeMode("Dither 2",    Buffer::Dither2,     kGeneral));
    col4->addWidget(makeMode("Negative",    Buffer::Negative,    kGeneral));
    col4->addWidget(makeStub("Halferite",                        kGeneral));
    col4->addWidget(makeMode("Transparent", Buffer::Transparent, kGeneral));
    modeCols->addLayout(col4);

    middleVBox->addLayout(modeCols);
    middleVBox->addStretch();
    {
        QFrame *sep = new QFrame(w);
        sep->setFrameShape(QFrame::HLine);
        sep->setFrameShadow(QFrame::Sunken);
        middleVBox->addWidget(sep);
    }

    QHBoxLayout *amountRow = new QHBoxLayout;
    amountRow->setSpacing(4);
    amountRow->addWidget(new QLabel("Amount:", w));
    QSlider *slider = new QSlider(Qt::Horizontal, w);
    slider->setToolTip("Dither or dark/brite");
    slider->setRange(0, 100);
    slider->setValue(buffer_->drawModeAmount());
    QSpinBox *spin = new QSpinBox(w);
    spin->setRange(0, 100);
    spin->setValue(buffer_->drawModeAmount());
    spin->setFixedWidth(48);
    connect(slider, &QSlider::valueChanged, spin, &QSpinBox::setValue);
    connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), slider, &QSlider::setValue);
    connect(slider, &QSlider::valueChanged, [this](int v) { buffer_->setDrawModeAmount(v); });
    amountRow->addWidget(slider, 1);
    amountRow->addWidget(spin);
    middleVBox->addLayout(amountRow);

    mainRow->addLayout(middleVBox);

    addVSep(mainRow);

    // ── Col 5: Gradient fill | sep | Conform / Center stubs ──────────
    static const struct { const char *label; GradientFillMode mode; } kFill[] = {
        {"Horizontal", FillHorizontal},
        {"Vertical",   FillVertical},
        {"Linear",     FillLinear},
        {"Highlight",  FillHighlight},
        {"Spherical",  FillSpherical},
        {"Radial",     FillRadial},
    };
    QButtonGroup *fillModeGroup = new QButtonGroup(w);
    fillModeGroup->setExclusive(true);

    QWidget *col5 = new QWidget(w);
    fillGroupWidget_ = col5;
    col5->setEnabled(buffer_ && buffer_->tool() && buffer_->tool()->hasFill());
    QVBoxLayout *col5vb = new QVBoxLayout(col5);
    col5vb->setSpacing(8);
    col5vb->setContentsMargins(0, 0, 0, 0);
    for (const auto &f : kFill) {
        QRadioButton *btn = new QRadioButton(f.label, col5);
        btn->setToolTip("Gradient fill mode");
        btn->setChecked(activeGradientFillMode == f.mode);
        fillModeGroup->addButton(btn);
        GradientFillMode fm = f.mode;
        connect(btn, &QRadioButton::clicked, [fm]() { activeGradientFillMode = fm; });
        col5vb->addWidget(btn);
    }
    col5vb->addStretch();
    {
        QFrame *sep = new QFrame(col5);
        sep->setFrameShape(QFrame::HLine);
        sep->setFrameShadow(QFrame::Sunken);
        col5vb->addWidget(sep);
    }
    auto makeCol5Stub = [&](const char *label, const char *tooltip) {
        QRadioButton *b = new QRadioButton(label, col5);
        b->setToolTip(tooltip);
        b->setEnabled(false);
        col5vb->addWidget(b);
    };
    makeCol5Stub("Conform", "Conform fill to edges");
    makeCol5Stub("Center",  "Auto center to fill");

    mainRow->addWidget(col5);

    return w;
}

void DrawModeTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 10);
}
