#include <QCheckBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>
#include "buffer.h"
#include "zoomtool.h"
#include "ui_zoomtool.h"

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
    ui_ = new Ui::ZoomToolOptions;
    ui_->setupUi(w);
    ui_->gridCheck->setChecked(buffer_ && buffer_->pixelGrid());
    ui_->zoomSpin->setValue(magnifierZoom_);
    connect(ui_->gridCheck, &QCheckBox::toggled, [this](bool checked) {
        if (buffer_) buffer_->setPixelGrid(checked);
    });
    if (buffer_) {
        connect(buffer_, &Buffer::pixelGridChanged, ui_->gridCheck, [this](bool enabled) {
            QSignalBlocker b(ui_->gridCheck);
            ui_->gridCheck->setChecked(enabled);
        });
    }
    connect(ui_->zoomSpin, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        magnifierZoom_ = v;
    });
    connect(ui_->placeBtn, &QPushButton::clicked, [this]() {
        magnifierZoom_ = ui_->zoomSpin->value();
        placeMagnifierMode_ = true;
        activate();
        hideOptionsPanel();
    });
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

void ZoomTool::enterPlaceMagnifierMode(int zoom)
{
    magnifierZoom_ = zoom;
    placeMagnifierMode_ = true;
    activate();
}

QString ZoomTool::name() const
{
    return placeMagnifierMode_ ? "Place Magnifier" : Tool::name();
}
