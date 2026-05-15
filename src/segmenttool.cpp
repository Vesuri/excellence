#include <QFormLayout>
#include <QGridLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>
#include "buffer.h"
#include "segmenttool.h"

SegmentTool SegmentTool::instance;

SegmentTool::SegmentTool(QObject *parent) : Tool(parent),
    radioDistance_(nullptr), radioPoints_(nullptr), spinValue_(nullptr)
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
    if (!buffer_) return;
    if (radioDistance_) {
        radioDistance_->blockSignals(true);
        radioDistance_->setChecked(buffer_->segmentByDistance());
        radioDistance_->blockSignals(false);
    }
    if (radioPoints_) {
        radioPoints_->blockSignals(true);
        radioPoints_->setChecked(!buffer_->segmentByDistance());
        radioPoints_->blockSignals(false);
    }
    if (spinValue_) {
        spinValue_->blockSignals(true);
        spinValue_->setValue(buffer_->segmentValue());
        spinValue_->blockSignals(false);
    }
}

QWidget *SegmentTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    w->setWindowTitle("Segment Draw");

    QVBoxLayout *vbox = new QVBoxLayout(w);
    vbox->setSpacing(4);
    vbox->setContentsMargins(6, 6, 6, 6);

    radioDistance_ = new QRadioButton("Distance", w);
    radioPoints_   = new QRadioButton("# of Points", w);

    if (buffer_) {
        radioDistance_->setChecked(buffer_->segmentByDistance());
        radioPoints_->setChecked(!buffer_->segmentByDistance());
    } else {
        radioDistance_->setChecked(true);
    }

    vbox->addWidget(radioDistance_);
    vbox->addWidget(radioPoints_);

    QFormLayout *form = new QFormLayout;
    form->setSpacing(4);
    spinValue_ = new QSpinBox(w);
    spinValue_->setRange(1, 9999);
    if (buffer_) spinValue_->setValue(buffer_->segmentValue());
    form->addRow("Value:", spinValue_);
    vbox->addLayout(form);
    vbox->addStretch();

    connect(radioDistance_, &QRadioButton::toggled, [this](bool v) {
        if (buffer_) buffer_->setSegmentByDistance(v);
    });
    connect(spinValue_, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        if (buffer_) buffer_->setSegmentValue(v);
    });

    return w;
}

void SegmentTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 6);
}
