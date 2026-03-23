#include <QCheckBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>
#include "buffer.h"
#include "mirrortool.h"

MirrorTool MirrorTool::instance;

MirrorTool::MirrorTool(QObject *parent) : Tool(parent),
    checkX_(nullptr), checkY_(nullptr),
    spinCX_(nullptr), spinCY_(nullptr)
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
    if (!buffer_)
        return;
    if (checkX_) {
        checkX_->blockSignals(true);
        checkX_->setChecked(buffer_->mirrorX());
        checkX_->blockSignals(false);
    }
    if (checkY_) {
        checkY_->blockSignals(true);
        checkY_->setChecked(buffer_->mirrorY());
        checkY_->blockSignals(false);
    }
    if (spinCX_) {
        spinCX_->blockSignals(true);
        spinCX_->setValue(buffer_->mirrorCenterX());
        spinCX_->blockSignals(false);
    }
    if (spinCY_) {
        spinCY_->blockSignals(true);
        spinCY_->setValue(buffer_->mirrorCenterY());
        spinCY_->blockSignals(false);
    }
}

QWidget *MirrorTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    w->setWindowTitle("Mirror Draw");

    QVBoxLayout *vbox = new QVBoxLayout(w);
    vbox->setSpacing(4);
    vbox->setContentsMargins(6, 6, 6, 6);

    checkX_ = new QCheckBox("Mirror X", w);
    checkY_ = new QCheckBox("Mirror Y", w);

    if (buffer_) {
        checkX_->setChecked(buffer_->mirrorX());
        checkY_->setChecked(buffer_->mirrorY());
    }

    vbox->addWidget(checkX_);
    vbox->addWidget(checkY_);

    QFormLayout *form = new QFormLayout;
    form->setSpacing(4);
    spinCX_ = new QSpinBox(w);
    spinCX_->setRange(0, 4096);
    spinCY_ = new QSpinBox(w);
    spinCY_->setRange(0, 4096);
    if (buffer_) {
        spinCX_->setValue(buffer_->mirrorCenterX());
        spinCY_->setValue(buffer_->mirrorCenterY());
    }
    form->addRow("Center X:", spinCX_);
    form->addRow("Center Y:", spinCY_);
    vbox->addLayout(form);
    vbox->addStretch();

    connect(checkX_, &QCheckBox::toggled, [this](bool v) {
        if (buffer_) buffer_->setMirrorX(v);
    });
    connect(checkY_, &QCheckBox::toggled, [this](bool v) {
        if (buffer_) buffer_->setMirrorY(v);
    });
    connect(spinCX_, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        if (buffer_) buffer_->setMirrorCenter(v, buffer_->mirrorCenterY());
    });
    connect(spinCY_, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v) {
        if (buffer_) buffer_->setMirrorCenter(buffer_->mirrorCenterX(), v);
    });

    return w;
}

void MirrorTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 5);
}
