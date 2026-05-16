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
#include "ui_gridlocktool.h"

GridLockTool GridLockTool::instance;

GridLockTool::GridLockTool(QObject *parent) : Tool(parent)
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
    connect(button_, &QToolButton::clicked, this, &GridLockTool::activate);
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
    if (!buffer_ || !ui_)
        return;
    ui_->spinW->setValue(buffer_->gridW());
    ui_->spinH->setValue(buffer_->gridH());
    ui_->spinOX->setValue(buffer_->gridOffsetX());
    ui_->spinOY->setValue(buffer_->gridOffsetY());
}

void GridLockTool::fitToBrush()
{
    if (!buffer_)
        return;
    Brush *brush = qobject_cast<Brush *>(buffer_->pen());
    if (!brush)
        return;
    if (ui_) {
        ui_->spinW->setValue(brush->image().width());
        ui_->spinH->setValue(brush->image().height());
    }
    buffer_->setGridSpacing(brush->image().width(), brush->image().height());
}

QWidget *GridLockTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    ui_ = new Ui::GridLockToolOptions;
    ui_->setupUi(w);
    syncSpinboxes();
    connect(ui_->spinW, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        if (buffer_) buffer_->setGridSpacing(v, buffer_->gridH());
    });
    connect(ui_->spinH, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        if (buffer_) buffer_->setGridSpacing(buffer_->gridW(), v);
    });
    connect(ui_->spinOX, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        if (buffer_) buffer_->setGridOffset(v, buffer_->gridOffsetY());
    });
    connect(ui_->spinOY, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        if (buffer_) buffer_->setGridOffset(buffer_->gridOffsetX(), v);
    });
    connect(ui_->fitBtn, &QPushButton::clicked, this, &GridLockTool::fitToBrush);
    return w;
}

void GridLockTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 4);
}
