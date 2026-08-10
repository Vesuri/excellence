#include <algorithm>
#include <QCheckBox>
#include <QEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include "buffer.h"
#include "palettebutton.h"
#include "stenciltool.h"

StencilTool StencilTool::instance;

StencilTool::StencilTool(QObject *parent) : Tool(parent)
{
}

void StencilTool::setBuffer(Buffer *buffer)
{
    if (buffer_) {
        disconnect(buffer_, &Buffer::stencilChanged, this, &StencilTool::syncButtonState);
        disconnect(buffer_, &Buffer::stencilColorsChanged, this, &StencilTool::syncWidgets);
        disconnect(buffer_, &Buffer::toolChanged, this, &StencilTool::onToolChanged);
    }
    Tool::setBuffer(buffer);
    if (buffer_) {
        connect(buffer_, &Buffer::stencilChanged, this, &StencilTool::syncButtonState);
        connect(buffer_, &Buffer::stencilColorsChanged, this, &StencilTool::syncWidgets);
        connect(buffer_, &Buffer::toolChanged, this, &StencilTool::onToolChanged);
        syncButtonState();
        syncWidgets();
    }
}

QRect StencilTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (!buffer_)
        return QRect();

    if (mode_ == Eyedropper) {
        if (buffer_->image().rect().contains(point)) {
            int idx = buffer_->image().pixelIndex(point);
            buffer_->setStencilColorSelected(idx, mouseButton_ != Qt::RightButton);
        }
        return QRect();
    }

    if (mode_ == Lasso) {
        lassoPath_.clear();
        lassoPath_.append(point);
        return QRect();
    }

    return QRect();
}

QRect StencilTool::move(const QPoint &point)
{
    if (mode_ == Lasso && mouseButton_ != Qt::NoButton)
        lassoPath_.append(point);
    return QRect();
}

QRect StencilTool::release(const QPoint &point)
{
    if (mode_ == Lasso) {
        lassoPath_.append(point);
        finishLasso();
    }
    return QRect();
}

void StencilTool::cancel()
{
    if (mode_ == Lasso) {
        lassoPath_.clear();
        stopPicking();
    }
}

void StencilTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/stencil.png"));
    button_->setToolTip("Stencil [']  Right-click: options");
    button_->setCheckable(true);
    connect(button_, &QToolButton::clicked, this, &StencilTool::activate);
}

void StencilTool::activate()
{
    // Left-click toggles protection on/off; does nothing until a mask exists.
    // Does NOT call Tool::activate() -- stencil does not change the active drawing tool.
    if (!buffer_ || !buffer_->hasStencil()) {
        // Qt auto-toggles a checkable QToolButton's visual state before clicked() fires;
        // undo that here since no actual state change happened.
        if (buffer_)
            button_->setChecked(buffer_->stencilEnabled());
        return;
    }
    buffer_->setStencilEnabled(!buffer_->stencilEnabled());
}

void StencilTool::toggle()
{
    activate();
}

void StencilTool::syncButtonState()
{
    if (!buffer_)
        return;
    // Stay enabled even without a mask yet -- right-click must still open the options
    // panel, since that's how a mask gets built in the first place. The no-op-without-a-
    // mask behavior for left-click lives in activate().
    button_->setChecked(buffer_->stencilEnabled());
}

void StencilTool::onToolChanged(Tool *tool)
{
    if (tool != this && mode_ != None) {
        mode_ = None;
        previousTool_ = nullptr;
    }
    syncWidgets();
}

void StencilTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 2);
}

bool StencilTool::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Show && buffer_)
        buffer_->setStencilSelectedColorsRestorePoint();
    return QObject::eventFilter(watched, event);
}

// ── Eyedropper / Lasso ──────────────────────────────────────────────────────

void StencilTool::startEyedropper()
{
    if (!buffer_)
        return;
    if (mode_ == Eyedropper) {
        stopPicking();
        return;
    }
    previousTool_ = buffer_->tool();
    mode_ = Eyedropper;
    buffer_->setTool(this);
    syncWidgets();
}

void StencilTool::startLasso()
{
    if (!buffer_)
        return;
    previousTool_ = buffer_->tool();
    mode_ = Lasso;
    lassoPath_.clear();
    buffer_->setTool(this);
    syncWidgets();
}

void StencilTool::stopPicking()
{
    mode_ = None;
    Tool *prev = previousTool_;
    previousTool_ = nullptr;
    if (prev && buffer_)
        buffer_->setTool(prev);
    syncWidgets();
}

void StencilTool::finishLasso()
{
    if (!buffer_ || lassoPath_.size() < 3) {
        lassoPath_.clear();
        stopPicking();
        return;
    }

    QImage &image = buffer_->image();
    const QRect imageRect = image.rect();
    const int n = lassoPath_.size();

    int minY = imageRect.bottom(), maxY = imageRect.top();
    for (const QPoint &p : lassoPath_) {
        minY = qMin(minY, p.y());
        maxY = qMax(maxY, p.y());
    }
    minY = qMax(minY, imageRect.top());
    maxY = qMin(maxY, imageRect.bottom());

    for (int y = minY; y <= maxY; y++) {
        QList<int> xs;
        for (int i = 0; i < n; i++) {
            const QPoint &p1 = lassoPath_[i];
            const QPoint &p2 = lassoPath_[(i + 1) % n];
            if ((p1.y() <= y && p2.y() > y) || (p2.y() <= y && p1.y() > y))
                xs.append(p1.x() + (y - p1.y()) * (p2.x() - p1.x()) / (p2.y() - p1.y()));
        }
        std::sort(xs.begin(), xs.end());
        for (int i = 0; i + 1 < xs.size(); i += 2) {
            int x1 = qMax(xs[i], imageRect.left());
            int x2 = qMin(xs[i + 1], imageRect.right());
            for (int x = x1; x <= x2; x++)
                buffer_->setStencilColorSelected(image.pixelIndex(x, y), true);
        }
    }

    lassoPath_.clear();
    stopPicking();
}

// ── Options window ──────────────────────────────────────────────────────────

void StencilTool::rebuildColorGrid()
{
    if (!buffer_ || !colorGrid_)
        return;
    int n = buffer_->image().colorCount();
    if (n == gridColorCount_)
        return;

    QLayoutItem *item;
    while ((item = colorGrid_->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    colorSwatches_.clear();

    static const int perRow = 16;
    for (int i = 0, row = 0, column = 0; i < n; i++) {
        PaletteButton *swatch = new PaletteButton();
        swatch->setPaletteIndex(static_cast<unsigned>(i));
        swatch->setColor(QColor(buffer_->image().color(i)));
        connect(swatch, &PaletteButton::paintColorSelected, this, [this](unsigned idx) {
            buffer_->setStencilColorSelected(static_cast<int>(idx), true);
        });
        connect(swatch, &PaletteButton::eraseColorSelected, this, [this](unsigned idx) {
            buffer_->setStencilColorSelected(static_cast<int>(idx), false);
        });
        colorGrid_->addWidget(swatch, row, column);
        colorSwatches_.append(swatch);
        column++;
        if (column >= perRow) { column = 0; row++; }
    }
    gridColorCount_ = n;
}

void StencilTool::syncWidgets()
{
    if (!buffer_)
        return;

    if (selectBtn_)
        selectBtn_->setChecked(mode_ == Eyedropper);

    if (colorGrid_) {
        rebuildColorGrid();
        bool show = showCheck_ && showCheck_->isChecked();
        for (PaletteButton *swatch : colorSwatches_) {
            swatch->setColor(QColor(buffer_->image().color(static_cast<int>(swatch->paletteIndex()))));
            swatch->setStencilSelected(show && buffer_->stencilColorSelected(static_cast<int>(swatch->paletteIndex())));
        }
    }
}

QWidget *StencilTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    auto *outer = new QVBoxLayout(w);
    outer->setSpacing(8);
    outer->setContentsMargins(4, 4, 4, 4);

    // Make section
    auto *makeGroup = new QGroupBox("Make");
    auto *makeLayout = new QVBoxLayout;
    makeLayout->setContentsMargins(6, 4, 6, 6);
    makeLayout->setSpacing(6);
    auto *foregroundBtn = new QPushButton("From Foreground");
    connect(foregroundBtn, &QPushButton::clicked, this, [this]() {
        if (buffer_) buffer_->stencilFromForeground();
    });
    makeLayout->addWidget(foregroundBtn);
    auto *colorsRow = new QHBoxLayout;
    colorsRow->setSpacing(6);
    auto *replaceBtn = new QPushButton("Replace");
    auto *addBtn = new QPushButton("Add");
    auto *subtractBtn = new QPushButton("Subtract");
    connect(replaceBtn, &QPushButton::clicked, this, [this]() {
        if (buffer_) buffer_->applyStencilColors(Buffer::StencilReplace);
    });
    connect(addBtn, &QPushButton::clicked, this, [this]() {
        if (buffer_) buffer_->applyStencilColors(Buffer::StencilAdd);
    });
    connect(subtractBtn, &QPushButton::clicked, this, [this]() {
        if (buffer_) buffer_->applyStencilColors(Buffer::StencilSubtract);
    });
    colorsRow->addWidget(replaceBtn);
    colorsRow->addWidget(addBtn);
    colorsRow->addWidget(subtractBtn);
    makeLayout->addLayout(colorsRow);
    makeGroup->setLayout(makeLayout);
    outer->addWidget(makeGroup);

    // Colors (selected-colors working set) section
    auto *colorsGroup = new QGroupBox("Colors");
    auto *colorsGroupLayout = new QVBoxLayout;
    colorsGroupLayout->setContentsMargins(6, 4, 6, 6);
    colorsGroupLayout->setSpacing(6);

    showCheck_ = new QCheckBox("Show");
    connect(showCheck_, &QCheckBox::toggled, this, [this](bool) { syncWidgets(); });
    colorsGroupLayout->addWidget(showCheck_);

    colorGrid_ = new QGridLayout;
    colorGrid_->setSpacing(2);
    colorsGroupLayout->addLayout(colorGrid_);

    auto *pickRow = new QHBoxLayout;
    pickRow->setSpacing(6);
    selectBtn_ = new QPushButton("Select");
    selectBtn_->setCheckable(true);
    connect(selectBtn_, &QPushButton::clicked, this, &StencilTool::startEyedropper);
    auto *lassoBtn = new QPushButton("Lasso");
    connect(lassoBtn, &QPushButton::clicked, this, &StencilTool::startLasso);
    pickRow->addWidget(selectBtn_);
    pickRow->addWidget(lassoBtn);
    colorsGroupLayout->addLayout(pickRow);

    auto *setOpsRow = new QHBoxLayout;
    setOpsRow->setSpacing(6);
    auto *invertColorsBtn = new QPushButton("Invert");
    auto *clearColorsBtn = new QPushButton("Clear");
    auto *restoreColorsBtn = new QPushButton("Restore");
    connect(invertColorsBtn, &QPushButton::clicked, this, [this]() {
        if (buffer_) buffer_->invertStencilSelectedColors();
    });
    connect(clearColorsBtn, &QPushButton::clicked, this, [this]() {
        if (buffer_) buffer_->clearStencilSelectedColors();
    });
    connect(restoreColorsBtn, &QPushButton::clicked, this, [this]() {
        if (buffer_) buffer_->restoreStencilSelectedColors();
    });
    setOpsRow->addWidget(invertColorsBtn);
    setOpsRow->addWidget(clearColorsBtn);
    setOpsRow->addWidget(restoreColorsBtn);
    colorsGroupLayout->addLayout(setOpsRow);

    colorsGroup->setLayout(colorsGroupLayout);
    outer->addWidget(colorsGroup);

    // Mask operations section
    auto *maskGroup = new QGroupBox("Mask");
    auto *maskLayout = new QHBoxLayout;
    maskLayout->setContentsMargins(6, 4, 6, 6);
    maskLayout->setSpacing(6);
    auto *maskInvertBtn = new QPushButton("Invert");
    auto *maskDeleteBtn = new QPushButton("Delete");
    connect(maskInvertBtn, &QPushButton::clicked, this, [this]() {
        if (buffer_) buffer_->invertStencilMask();
    });
    connect(maskDeleteBtn, &QPushButton::clicked, this, [this]() {
        if (buffer_) buffer_->deleteStencilMask();
    });
    maskLayout->addWidget(maskInvertBtn);
    maskLayout->addWidget(maskDeleteBtn);
    maskGroup->setLayout(maskLayout);
    outer->addWidget(maskGroup);

    w->installEventFilter(this);
    optionsWidget_ = w;
    syncWidgets();
    return w;
}
