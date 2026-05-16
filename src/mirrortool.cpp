#include <QCheckBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>
#include "buffer.h"
#include "mirrortool.h"
#include "ui_mirrortooloptions.h"

MirrorTool MirrorTool::instance;

MirrorTool::MirrorTool(QObject *parent) : Tool(parent)
{
}

void MirrorTool::setBuffer(Buffer *buffer)
{
    if (buffer_)
        disconnect(buffer_, SIGNAL(mirrorChanged()), this, SLOT(syncButtonState()));
    Tool::setBuffer(buffer);
    if (buffer_) {
        connect(buffer_, SIGNAL(mirrorChanged()), this, SLOT(syncButtonState()));
        syncButtonState();
        syncWidgets();
    }
}

void MirrorTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/mirror.png"));
    button_->setToolTip("Mirror Draw [/]");
    button_->setCheckable(true);
    connect(button_, &QToolButton::clicked, this, &MirrorTool::activate);
}

void MirrorTool::activate()
{
    if (!buffer_)
        return;
    bool enabled = buffer_->mirrorX() || buffer_->mirrorY();
    // Toggle: if any mirror is active, turn both off; otherwise enable X mirror
    if (enabled) {
        buffer_->setMirrorX(false);
        buffer_->setMirrorY(false);
    } else {
        buffer_->setMirrorX(true);
    }
    // Do NOT call Tool::activate() — mirror does not change the active drawing tool.
}

void MirrorTool::toggle()
{
    activate();
}

void MirrorTool::syncButtonState()
{
    if (buffer_)
        button_->setChecked(buffer_->mirrorX() || buffer_->mirrorY());
    syncWidgets();
}

void MirrorTool::syncWidgets()
{
    if (!buffer_ || !ui_) return;
    { QSignalBlocker b(ui_->checkX);  ui_->checkX->setChecked(buffer_->mirrorX()); }
    { QSignalBlocker b(ui_->checkY);  ui_->checkY->setChecked(buffer_->mirrorY()); }
    { QSignalBlocker b(ui_->spinCX);  ui_->spinCX->setValue(buffer_->mirrorCenterX()); }
    { QSignalBlocker b(ui_->spinCY);  ui_->spinCY->setValue(buffer_->mirrorCenterY()); }
}

QWidget *MirrorTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    ui_ = new Ui::MirrorToolOptions;
    ui_->setupUi(w);
    if (buffer_) {
        ui_->checkX->setChecked(buffer_->mirrorX());
        ui_->checkY->setChecked(buffer_->mirrorY());
        ui_->spinCX->setValue(buffer_->mirrorCenterX());
        ui_->spinCY->setValue(buffer_->mirrorCenterY());
    }
    connect(ui_->checkX, &QCheckBox::toggled, [this](bool v) {
        if (buffer_) buffer_->setMirrorX(v);
    });
    connect(ui_->checkY, &QCheckBox::toggled, [this](bool v) {
        if (buffer_) buffer_->setMirrorY(v);
    });
    connect(ui_->spinCX, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        if (buffer_) buffer_->setMirrorCenter(v, buffer_->mirrorCenterY());
    });
    connect(ui_->spinCY, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        if (buffer_) buffer_->setMirrorCenter(buffer_->mirrorCenterX(), v);
    });
    return w;
}

void MirrorTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 5);
}
