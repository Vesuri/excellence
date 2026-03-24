#include <QCheckBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>
#include "buffer.h"
#include "zoomtool.h"

ZoomTool ZoomTool::instance;

ZoomTool::ZoomTool(QObject *parent) : Tool(parent),
    placeMagnifierMode_(false), magnifierZoom_(4)
{
}

void ZoomTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect ZoomTool::press(const QPoint &point, const Qt::KeyboardModifiers &modifiers)
{
    if (placeMagnifierMode_) {
        placeMagnifierMode_ = false;
        return QRect(point, QSize(magnifierZoom_, 0));
    }
    int factor = mouseButton_ == Qt::RightButton || (modifiers & Qt::ControlModifier) ? -2 : 2;
    return QRect(point, QSize(factor, factor));
}

QRect ZoomTool::move(const QPoint &)
{
    return QRect();
}

QRect ZoomTool::release(const QPoint &)
{
    return QRect();
}

QWidget *ZoomTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    w->setWindowTitle("Zoom Options");

    QVBoxLayout *vbox = new QVBoxLayout(w);
    vbox->setSpacing(4);
    vbox->setContentsMargins(6, 6, 6, 6);

    QCheckBox *gridCheck = new QCheckBox("Show Pixel Grid [P]", w);
    gridCheck->setChecked(buffer_ && buffer_->pixelGrid());
    connect(gridCheck, &QCheckBox::toggled, [this](bool checked) {
        if (buffer_) buffer_->setPixelGrid(checked);
    });
    if (buffer_) {
        connect(buffer_, &Buffer::pixelGridChanged, gridCheck, [gridCheck](bool enabled) {
            bool blocked = gridCheck->blockSignals(true);
            gridCheck->setChecked(enabled);
            gridCheck->blockSignals(blocked);
        });
    }
    vbox->addWidget(gridCheck);

    QFormLayout *form = new QFormLayout;
    form->setSpacing(4);
    QSpinBox *zoomSpin = new QSpinBox(w);
    zoomSpin->setRange(2, 32);
    zoomSpin->setValue(magnifierZoom_);
    connect(zoomSpin, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        magnifierZoom_ = v;
    });
    form->addRow("Magnifier Zoom:", zoomSpin);
    vbox->addLayout(form);

    QPushButton *placeBtn = new QPushButton("Place Magnifier", w);
    connect(placeBtn, &QPushButton::clicked, [this, zoomSpin]() {
        magnifierZoom_ = zoomSpin->value();
        placeMagnifierMode_ = true;
        activate();
        if (optionsWidget_) optionsWidget_->hide();
    });
    vbox->addWidget(placeBtn);

    vbox->addStretch();
    return w;
}

void ZoomTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/zoom.png"));
    button_->setToolTip("Zoom [+/-]");

    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void ZoomTool::activate()
{
    if (buffer_->tool() == this) {
        button_->setChecked(true);
    }

    Tool::activate();
}

void ZoomTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 11);
}

Tool::Type ZoomTool::type() const
{
    return Zoom;
}
