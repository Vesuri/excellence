#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>
#include "brush.h"
#include "buffer.h"
#include "gridlocktool.h"

GridLockTool GridLockTool::instance;

GridLockTool::GridLockTool(QObject *parent) : Tool(parent),
    spinW_(nullptr), spinH_(nullptr), spinOX_(nullptr), spinOY_(nullptr)
{
}

void GridLockTool::setBuffer(Buffer *buffer)
{
    if (buffer_)
        disconnect(buffer_, SIGNAL(gridChanged()), this, SLOT(syncButtonState()));
    Tool::setBuffer(buffer);
    if (buffer_) {
        connect(buffer_, SIGNAL(gridChanged()), this, SLOT(syncButtonState()));
        syncButtonState();
        syncSpinboxes();
    }
}

void GridLockTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/gridlock.png"));
    button_->setToolTip("Grid Lock [G]");
    button_->setCheckable(true);
    connect(button_, &QPushButton::clicked, this, &GridLockTool::activate);
}

void GridLockTool::activate()
{
    if (buffer_)
        buffer_->setGridEnabled(!buffer_->gridEnabled());
    // Do NOT call Tool::activate() — grid lock does not change the active drawing tool.
}

void GridLockTool::syncButtonState()
{
    if (buffer_)
        button_->setChecked(buffer_->gridEnabled());
}

void GridLockTool::syncSpinboxes()
{
    if (!buffer_ || !spinW_)
        return;
    spinW_->setValue(buffer_->gridW());
    spinH_->setValue(buffer_->gridH());
    spinOX_->setValue(buffer_->gridOffsetX());
    spinOY_->setValue(buffer_->gridOffsetY());
}

void GridLockTool::fitToBrush()
{
    if (!buffer_)
        return;
    Brush *brush = qobject_cast<Brush *>(buffer_->pen());
    if (!brush)
        return;
    if (spinW_) spinW_->setValue(brush->image().width());
    if (spinH_) spinH_->setValue(brush->image().height());
    buffer_->setGridSpacing(brush->image().width(), brush->image().height());
}

QWidget *GridLockTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    w->setWindowTitle("Grid Settings");

    QVBoxLayout *vbox = new QVBoxLayout(w);
    vbox->setSpacing(4);
    vbox->setContentsMargins(6, 6, 6, 6);

    QFormLayout *form = new QFormLayout;
    form->setSpacing(4);

    spinW_ = new QSpinBox(w);
    spinW_->setRange(1, 1024);
    spinH_ = new QSpinBox(w);
    spinH_->setRange(1, 1024);
    spinOX_ = new QSpinBox(w);
    spinOX_->setRange(0, 1023);
    spinOY_ = new QSpinBox(w);
    spinOY_->setRange(0, 1023);

    syncSpinboxes();

    form->addRow("Grid W:", spinW_);
    form->addRow("Grid H:", spinH_);
    form->addRow("Offset X:", spinOX_);
    form->addRow("Offset Y:", spinOY_);
    vbox->addLayout(form);

    QPushButton *fitBtn = new QPushButton("Fit to Brush", w);
    vbox->addWidget(fitBtn);
    vbox->addStretch();

    connect(spinW_, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        if (buffer_) buffer_->setGridSpacing(v, buffer_->gridH());
    });
    connect(spinH_, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        if (buffer_) buffer_->setGridSpacing(buffer_->gridW(), v);
    });
    connect(spinOX_, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        if (buffer_) buffer_->setGridOffset(v, buffer_->gridOffsetY());
    });
    connect(spinOY_, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        if (buffer_) buffer_->setGridOffset(buffer_->gridOffsetX(), v);
    });
    connect(fitBtn, &QPushButton::clicked, this, &GridLockTool::fitToBrush);

    return w;
}

void GridLockTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 4);
}
