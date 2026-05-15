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

void DrawModeTool::onToolChanged(Tool *)
{
    updateAvailability();
}

void DrawModeTool::onPaintModeChanged(Buffer::PaintMode mode)
{
    if (mode != Buffer::Normal) {
        selectedMode_ = mode;
        button_->setChecked(true);
        drawModeActive = true;
    } else if (fillModeSelected_) {
        // Gradient fill is active with Normal paint mode — keep draw mode on.
        button_->setChecked(true);
        drawModeActive = true;
    } else {
        button_->setChecked(false);
        drawModeActive = false;
    }
    if (!fillModeSelected_)
        emit selectedModeChanged(selectedMode_);
}

void DrawModeTool::onPenChanged(Pen *)
{
    updateAvailability();
    if (buffer_ && !isModeAvailable(buffer_->paintMode())) {
        selectedMode_ = Buffer::Normal;
        buffer_->setPaintMode(Buffer::Normal);
    }
}

void DrawModeTool::updateAvailability()
{
    if (!buffer_) return;

    bool brushActive = qobject_cast<Brush *>(buffer_->pen()) != nullptr;
    Tool *tool = buffer_->tool();
    bool hasFill = tool && tool->hasFill();
    bool restricted = tool && tool->restrictToColorAndRandom();
    bool allowsBrushBtn = !tool || tool->allowsBrushModeButton();

    if (brushModeBtn_)
        brushModeBtn_->setEnabled(brushActive && allowsBrushBtn);
    if (replaceModeBtn_)
        replaceModeBtn_->setEnabled(brushActive);
    if (randomModeBtn_)
        randomModeBtn_->setEnabled(restricted);
    for (auto *btn : generalModeBtns_)
        btn->setEnabled(!restricted);
    for (auto *btn : fillSensitiveBtns_)
        btn->setEnabled(!restricted && !hasFill);
    if (fillGroupWidget_)
        fillGroupWidget_->setEnabled(hasFill);

    // Sync radio-button selection with fill availability.
    if (!hasFill && fillModeSelected_) {
        fillModeSelected_ = false;
        emit selectedModeChanged(selectedMode_);
    } else if (hasFill && !fillModeSelected_ && activeGradientFillMode != FillFlat) {
        fillModeSelected_ = true;
        for (auto &fb : fillModeBtns_) {
            if (fb.second == activeGradientFillMode) {
                fb.first->setChecked(true);
                break;
            }
        }
    }

    if (button_->isChecked() && !fillModeSelected_ && !isModeAvailable(selectedMode_)) {
        selectedMode_ = Buffer::Normal;
        buffer_->setPaintMode(Buffer::Normal);
    }
}

bool DrawModeTool::isModeAvailable(Buffer::PaintMode mode) const
{
    if (!buffer_) return mode == Buffer::Normal;

    bool brushActive = qobject_cast<Brush *>(buffer_->pen()) != nullptr;
    Tool *tool = buffer_->tool();
    bool hasFill = tool && tool->hasFill();
    bool restricted = tool && tool->restrictToColorAndRandom();
    bool allowsBrushBtn = !tool || tool->allowsBrushModeButton();

    switch (mode) {
    case Buffer::Normal:      return true;
    case Buffer::BrushMode:   return brushActive && allowsBrushBtn;
    case Buffer::Replace:     return brushActive;
    case Buffer::Random:      return restricted;
    case Buffer::Mix:
    case Buffer::Smear:
    case Buffer::AverageSmear:
    case Buffer::Cycle:       return !restricted && !hasFill;
    default:                  return !restricted;
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
        btn->setChecked(!fillModeSelected_ && selectedMode_ == mode);
        modeGroup->addButton(btn);
        connect(btn, &QRadioButton::clicked, [this, mode]() {
            fillModeSelected_ = false;
            selectedMode_ = mode;
            button_->setChecked(mode != Buffer::Normal);
            applyMode();
        });
        connect(this, &DrawModeTool::selectedModeChanged, btn, [this, btn, mode](Buffer::PaintMode active) {
            if (!fillModeSelected_)
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
    brushModeBtn_ = makeMode("Brush",       Buffer::BrushMode, "Brush draw mode");
    col1->addWidget(brushModeBtn_);
    addHSep(col1);
    col1->addWidget(makeStub("Stretch",    "Brush fill mode"));
    col1->addWidget(makeStub("Pattern",    "Brush fill mode"));
    col1->addWidget(makeStub("Shape",      "Brush fill mode"));
    col1->addWidget(makeStub("Perspective","Brush fill mode"));
    col1->addStretch();
    addHSep(col1);
    replaceModeBtn_ = makeMode("Replace",  Buffer::Replace, "Replace draw brush");
    col1->addWidget(replaceModeBtn_);
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
    auto *tintBtn      = makeMode("Tint",     Buffer::Tint,         kGeneral);
    auto *colorizeBtn  = makeMode("Colorize", Buffer::Colorize,     kGeneral);
    auto *brightenBtn  = makeMode("Brighten", Buffer::Brighten,     kGeneral);
    auto *darkenBtn    = makeMode("Darken",   Buffer::Darken,       kGeneral);
    col2->addWidget(tintBtn);
    col2->addWidget(colorizeBtn);
    col2->addWidget(brightenBtn);
    col2->addWidget(darkenBtn);
    col2->addWidget(makeStub("Stencil",                        kGeneral));
    generalModeBtns_ << tintBtn << colorizeBtn << brightenBtn << darkenBtn;
    modeCols->addLayout(col2);

    // Col 3
    QVBoxLayout *col3 = new QVBoxLayout;
    col3->setSpacing(8);
    col3->setAlignment(Qt::AlignTop);
    auto *mixBtn       = makeMode("Mix",       Buffer::Mix,          kGeneral);
    auto *smoothBtn    = makeMode("Smooth",    Buffer::Smooth,       kGeneral);
    auto *smearBtn     = makeMode("Smear",     Buffer::Smear,        kGeneral);
    auto *avgSmearBtn  = makeMode("Avg Smear", Buffer::AverageSmear, kGeneral);
    auto *rangeBtn     = makeMode("Range",     Buffer::Range,        kGeneral);
    auto *cycleBtn     = makeMode("Cycle",     Buffer::Cycle,        kGeneral);
    col3->addWidget(mixBtn);
    col3->addWidget(smoothBtn);
    col3->addWidget(smearBtn);
    col3->addWidget(avgSmearBtn);
    col3->addWidget(rangeBtn);
    col3->addWidget(cycleBtn);
    generalModeBtns_ << smoothBtn << rangeBtn;
    fillSensitiveBtns_ << mixBtn << smearBtn << avgSmearBtn << cycleBtn;
    modeCols->addLayout(col3);

    // Col 4
    QVBoxLayout *col4 = new QVBoxLayout;
    col4->setSpacing(8);
    col4->setAlignment(Qt::AlignTop);
    randomModeBtn_ = makeMode("Random",      Buffer::Random,      kGeneral);
    auto *dither1Btn   = makeMode("Dither 1",    Buffer::Dither1,     kGeneral);
    auto *dither2Btn   = makeMode("Dither 2",    Buffer::Dither2,     kGeneral);
    auto *negativeBtn  = makeMode("Negative",    Buffer::Negative,    kGeneral);
    auto *transpBtn    = makeMode("Transparent", Buffer::Transparent, kGeneral);
    col4->addWidget(randomModeBtn_);
    col4->addWidget(dither1Btn);
    col4->addWidget(dither2Btn);
    col4->addWidget(negativeBtn);
    col4->addWidget(makeStub("Halferite",                        kGeneral));
    col4->addWidget(transpBtn);
    generalModeBtns_ << dither1Btn << dither2Btn << negativeBtn << transpBtn;
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

    QHBoxLayout *mixingRow = new QHBoxLayout;
    mixingRow->setSpacing(4);
    mixingRow->addWidget(new QLabel("Mixing:", w));
    QRadioButton *rgbMixBtn = new QRadioButton("RGB", w);
    QRadioButton *hsvMixBtn = new QRadioButton("HSV", w);
    QButtonGroup *mixGroup = new QButtonGroup(w);
    mixGroup->addButton(rgbMixBtn);
    mixGroup->addButton(hsvMixBtn);
    rgbMixBtn->setChecked(!buffer_->transparentMixHSV());
    hsvMixBtn->setChecked(buffer_->transparentMixHSV());
    rgbMixBtn->setToolTip("Transparent: blend in RGB space");
    hsvMixBtn->setToolTip("Transparent: blend in HSV space (e.g. Blue 50% over Green → Cyan)");
    connect(rgbMixBtn, &QRadioButton::clicked, [this]() { buffer_->setTransparentMixHSV(false); });
    connect(hsvMixBtn, &QRadioButton::clicked, [this]() { buffer_->setTransparentMixHSV(true); });
    mixingRow->addWidget(rgbMixBtn);
    mixingRow->addWidget(hsvMixBtn);
    mixingRow->addStretch();
    middleVBox->addLayout(mixingRow);

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
    QWidget *col5 = new QWidget(w);
    fillGroupWidget_ = col5;
    col5->setEnabled(buffer_ && buffer_->tool() && buffer_->tool()->hasFill());
    QVBoxLayout *col5vb = new QVBoxLayout(col5);
    col5vb->setSpacing(8);
    col5vb->setContentsMargins(0, 0, 0, 0);
    for (const auto &f : kFill) {
        QRadioButton *btn = new QRadioButton(f.label, col5);
        btn->setToolTip("Gradient fill mode");
        btn->setChecked(fillModeSelected_ && activeGradientFillMode == f.mode);
        modeGroup->addButton(btn);
        GradientFillMode fm = f.mode;
        fillModeBtns_.append({btn, fm});
        connect(btn, &QRadioButton::clicked, [this, fm]() {
            fillModeSelected_ = true;
            activeGradientFillMode = fm;
            button_->setChecked(true);
            applyMode();
        });
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

    updateAvailability();

    return w;
}

void DrawModeTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 10);
}
