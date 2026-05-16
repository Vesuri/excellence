#include <QButtonGroup>
#include <QCheckBox>
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
#include "ui_drawmodetool.h"

DrawModeTool DrawModeTool::instance;

static bool isSelectableMode(Buffer::PaintMode mode)
{
    return mode != Buffer::Color && mode != Buffer::BrushMode;
}

DrawModeTool::DrawModeTool(QObject *parent) : Tool(parent),
    previousMode_(Buffer::Cycle)
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
    if (ui_) ui_->replaceModeBtn->setChecked(buffer_ && buffer_->replaceMode());
}

void DrawModeTool::onToolChanged(Tool *)
{
    updateAvailability();
}

void DrawModeTool::onPaintModeChanged(Buffer::PaintMode mode)
{
    if (isSelectableMode(mode)) {
        previousMode_ = mode;
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
        emit activeModeChanged(mode);
}

void DrawModeTool::onPenChanged(Pen *)
{
    updateAvailability();
    if (buffer_ && !isModeAvailable(buffer_->paintMode()))
        buffer_->setPaintMode(Buffer::Color);
    if (buffer_ && !qobject_cast<Brush *>(buffer_->pen()) && buffer_->replaceMode()) {
        buffer_->setReplaceMode(false);
        if (ui_) ui_->replaceModeBtn->setChecked(false);
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

    if (ui_) {
        ui_->brushModeBtn->setEnabled(brushActive && allowsBrushBtn);
        ui_->replaceModeBtn->setEnabled(brushActive);
        ui_->randomModeBtn->setEnabled(restricted);
        ui_->fillGroupWidget->setEnabled(hasFill);
    }
    for (auto *btn : generalModeBtns_)
        btn->setEnabled(!restricted);
    for (auto *btn : fillSensitiveBtns_)
        btn->setEnabled(!restricted && !hasFill);

    // Sync radio-button selection with fill availability.
    if (!hasFill && fillModeSelected_) {
        fillModeSelected_ = false;
        emit activeModeChanged(buffer_->paintMode());
    } else if (hasFill && !fillModeSelected_ && activeGradientFillMode != FillFlat) {
        fillModeSelected_ = true;
        for (auto &fb : fillModeBtns_) {
            if (fb.second == activeGradientFillMode) {
                fb.first->setChecked(true);
                break;
            }
        }
    }

    if (button_->isChecked() && !fillModeSelected_ && !isModeAvailable(previousMode_))
        buffer_->setPaintMode(Buffer::Color);
}

bool DrawModeTool::isModeAvailable(Buffer::PaintMode mode) const
{
    if (!buffer_) return mode == Buffer::Color;

    bool brushActive = qobject_cast<Brush *>(buffer_->pen()) != nullptr;
    Tool *tool = buffer_->tool();
    bool hasFill = tool && tool->hasFill();
    bool restricted = tool && tool->restrictToColorAndRandom();
    bool allowsBrushBtn = !tool || tool->allowsBrushModeButton();

    switch (mode) {
    case Buffer::Color:      return true;
    case Buffer::BrushMode:   return brushActive && allowsBrushBtn;
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
    drawModeActive = active;
    if (active)
        buffer_->setPaintMode(previousMode_);
    else if (ui_ && ui_->brushModeBtn->isChecked())
        buffer_->setPaintMode(Buffer::BrushMode);
    else
        buffer_->setPaintMode(Buffer::Color);
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
    ui_ = new Ui::DrawModeToolOptions;
    ui_->setupUi(w);

    // Set initial state
    ui_->brushModeBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::BrushMode);
    ui_->colorBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Color);
    ui_->tintBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Tint);
    ui_->colorizeBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Colorize);
    ui_->brightenBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Brighten);
    ui_->darkenBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Darken);
    ui_->mixBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Mix);
    ui_->smoothBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Smooth);
    ui_->smearBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Smear);
    ui_->avgSmearBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::AverageSmear);
    ui_->rangeBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Range);
    ui_->cycleBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Cycle);
    ui_->randomModeBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Random);
    ui_->dither1Btn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Dither1);
    ui_->dither2Btn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Dither2);
    ui_->negativeBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Negative);
    ui_->transpBtn->setChecked(!fillModeSelected_ && buffer_ && buffer_->paintMode() == Buffer::Transparent);

    ui_->fillHorizontal->setChecked(fillModeSelected_ && activeGradientFillMode == FillHorizontal);
    ui_->fillVertical->setChecked(fillModeSelected_ && activeGradientFillMode == FillVertical);
    ui_->fillLinear->setChecked(fillModeSelected_ && activeGradientFillMode == FillLinear);
    ui_->fillHighlight->setChecked(fillModeSelected_ && activeGradientFillMode == FillHighlight);
    ui_->fillSpherical->setChecked(fillModeSelected_ && activeGradientFillMode == FillSpherical);
    ui_->fillRadial->setChecked(fillModeSelected_ && activeGradientFillMode == FillRadial);

    ui_->replaceModeBtn->setChecked(buffer_ && buffer_->replaceMode());
    ui_->amountSlider->setValue(buffer_ ? buffer_->drawModeAmount() : 0);
    ui_->amountSpin->setValue(buffer_ ? buffer_->drawModeAmount() : 0);
    ui_->rgbMixBtn->setChecked(buffer_ && !buffer_->transparentMixHSV());
    ui_->hsvMixBtn->setChecked(buffer_ && buffer_->transparentMixHSV());
    ui_->conformCheck->setChecked(conformFill);
    ui_->centerCheck->setChecked(centerFill);

    ui_->fillGroupWidget->setEnabled(buffer_ && buffer_->tool() && buffer_->tool()->hasFill());

    // Build lists
    generalModeBtns_.clear();
    generalModeBtns_ << ui_->tintBtn << ui_->colorizeBtn << ui_->brightenBtn << ui_->darkenBtn
                     << ui_->smoothBtn << ui_->rangeBtn << ui_->dither1Btn << ui_->dither2Btn
                     << ui_->negativeBtn << ui_->transpBtn;
    fillSensitiveBtns_.clear();
    fillSensitiveBtns_ << ui_->mixBtn << ui_->smearBtn << ui_->avgSmearBtn << ui_->cycleBtn;
    fillModeBtns_.clear();
    fillModeBtns_ = {
        {ui_->fillHorizontal, FillHorizontal},
        {ui_->fillVertical,   FillVertical},
        {ui_->fillLinear,     FillLinear},
        {ui_->fillHighlight,  FillHighlight},
        {ui_->fillSpherical,  FillSpherical},
        {ui_->fillRadial,     FillRadial},
    };

    // Connect mode buttons
    auto connectMode = [&](QRadioButton *btn, Buffer::PaintMode mode) {
        connect(btn, &QRadioButton::clicked, [this, mode]() {
            fillModeSelected_ = false;
            if (isSelectableMode(mode)) previousMode_ = mode;
            button_->setChecked(isSelectableMode(mode));
            applyMode();
        });
        connect(this, &DrawModeTool::activeModeChanged, btn, [this, btn, mode](Buffer::PaintMode active) {
            if (!fillModeSelected_) btn->setChecked(active == mode);
        });
    };
    connectMode(ui_->brushModeBtn,  Buffer::BrushMode);
    connectMode(ui_->colorBtn,      Buffer::Color);
    connectMode(ui_->tintBtn,       Buffer::Tint);
    connectMode(ui_->colorizeBtn,   Buffer::Colorize);
    connectMode(ui_->brightenBtn,   Buffer::Brighten);
    connectMode(ui_->darkenBtn,     Buffer::Darken);
    connectMode(ui_->mixBtn,        Buffer::Mix);
    connectMode(ui_->smoothBtn,     Buffer::Smooth);
    connectMode(ui_->smearBtn,      Buffer::Smear);
    connectMode(ui_->avgSmearBtn,   Buffer::AverageSmear);
    connectMode(ui_->rangeBtn,      Buffer::Range);
    connectMode(ui_->cycleBtn,      Buffer::Cycle);
    connectMode(ui_->randomModeBtn, Buffer::Random);
    connectMode(ui_->dither1Btn,    Buffer::Dither1);
    connectMode(ui_->dither2Btn,    Buffer::Dither2);
    connectMode(ui_->negativeBtn,   Buffer::Negative);
    connectMode(ui_->transpBtn,     Buffer::Transparent);

    for (auto &fb : fillModeBtns_) {
        connect(fb.first, &QRadioButton::clicked, [this, fm = fb.second]() {
            fillModeSelected_ = true;
            activeGradientFillMode = fm;
            button_->setChecked(true);
            applyMode();
        });
    }

    connect(ui_->replaceModeBtn, &QCheckBox::toggled, [this](bool on) {
        if (buffer_) buffer_->setReplaceMode(on);
    });
    connect(ui_->amountSlider, &QSlider::valueChanged, ui_->amountSpin, &QSpinBox::setValue);
    connect(ui_->amountSpin, QOverload<int>::of(&QSpinBox::valueChanged), ui_->amountSlider, &QSlider::setValue);
    connect(ui_->amountSlider, &QSlider::valueChanged, [this](int v) { buffer_->setDrawModeAmount(v); });
    connect(ui_->rgbMixBtn, &QRadioButton::clicked, [this]() { buffer_->setTransparentMixHSV(false); });
    connect(ui_->hsvMixBtn, &QRadioButton::clicked, [this]() { buffer_->setTransparentMixHSV(true); });
    connect(ui_->conformCheck, &QCheckBox::toggled, [](bool v) { conformFill = v; });
    connect(ui_->centerCheck,  &QCheckBox::toggled, [](bool v) { centerFill = v; });

    updateAvailability();
    return w;
}

void DrawModeTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 10);
}
