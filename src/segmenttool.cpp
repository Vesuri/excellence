#include <QFormLayout>
#include <QGridLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>
#include "buffer.h"
#include "segmenttool.h"
#include "ui_segmenttooloptions.h"

SegmentTool SegmentTool::instance;

SegmentTool::SegmentTool(QObject *parent) : Tool(parent)
{
}

void SegmentTool::setBuffer(Buffer *buffer)
{
    if (buffer_)
        disconnect(buffer_, &Buffer::segmentChanged, this, &SegmentTool::syncButtonState);
    Tool::setBuffer(buffer);
    if (buffer_) {
        connect(buffer_, &Buffer::segmentChanged, this, &SegmentTool::syncButtonState);
        syncButtonState();
        syncWidgets();
    }
}

void SegmentTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/segment.png"));
    button_->setToolTip("Segment Draw");
    button_->setCheckable(true);
    connect(button_, &QToolButton::clicked, this, &SegmentTool::activate);
}

void SegmentTool::activate()
{
    if (!buffer_) return;
    buffer_->setSegmentActive(!buffer_->segmentActive());
    // Do NOT call Tool::activate() — segment does not change the active drawing tool.
}

void SegmentTool::syncButtonState()
{
    if (buffer_)
        button_->setChecked(buffer_->segmentActive());
    syncWidgets();
}

void SegmentTool::syncWidgets()
{
    if (!buffer_ || !ui_) return;
    { QSignalBlocker b(ui_->radioDistance); ui_->radioDistance->setChecked(buffer_->segmentByDistance()); }
    { QSignalBlocker b(ui_->radioPoints);   ui_->radioPoints->setChecked(!buffer_->segmentByDistance()); }
    { QSignalBlocker b(ui_->spinValue);     ui_->spinValue->setValue(buffer_->segmentValue()); }
}

QWidget *SegmentTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    ui_ = new Ui::SegmentToolOptions;
    ui_->setupUi(w);
    if (buffer_) {
        ui_->radioDistance->setChecked(buffer_->segmentByDistance());
        ui_->radioPoints->setChecked(!buffer_->segmentByDistance());
        ui_->spinValue->setValue(buffer_->segmentValue());
    } else {
        ui_->radioDistance->setChecked(true);
    }
    connect(ui_->radioDistance, &QRadioButton::toggled, [this](bool v) {
        if (buffer_) buffer_->setSegmentByDistance(v);
    });
    connect(ui_->spinValue, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        if (buffer_) buffer_->setSegmentValue(v);
    });
    return w;
}

void SegmentTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 6);
}
