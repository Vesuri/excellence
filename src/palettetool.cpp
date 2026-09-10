#include <QGridLayout>
#include <QColorDialog>
#include "buffer.h"
#include "palettetool.h"

PaletteTool PaletteTool::instance;

PaletteTool::PaletteTool(QObject *parent) : Tool(parent)
{
}

void PaletteTool::setBuffer(Buffer *buffer)
{
    if (buffer_)
        disconnect(buffer_, &Buffer::paintColorChanged, this, &PaletteTool::setPaintColor);

    Tool::setBuffer(buffer);

    if (buffer_) {
        setPaintColor(buffer_->paintColor(), buffer_->image().color(static_cast<int>(buffer_->paintColor())));
        connect(buffer_, &Buffer::paintColorChanged, this, &PaletteTool::setPaintColor);
    }
}

void PaletteTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/palette.png"));
    button_->setToolTip("Palette – Edit active color\nRight-click: open palette window");
    connect(button_, &QToolButton::clicked, this, &PaletteTool::toggleColorDialogVisibility);

    disconnect(button_, &QToolButton::customContextMenuRequested, this, &Tool::toggleOptionsWidget);
    connect(button_, &QToolButton::customContextMenuRequested,
            this, &PaletteTool::toggleColorDialogVisibility);

    colorDialog_ = new QColorDialog;
    colorDialog_->setOption(QColorDialog::DontUseNativeDialog);
    colorDialog_->setOption(QColorDialog::NoButtons);
    connect(colorDialog_, &QColorDialog::currentColorChanged,
            this, &PaletteTool::editPaintColor);
}

void PaletteTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 1);
}

void PaletteTool::toggleColorDialogVisibility()
{
    colorDialog_->setVisible(!colorDialog_->isVisible());
}

void PaletteTool::setPaintColor(unsigned, const QColor &color)
{
    colorDialog_->setCurrentColor(color);
}

void PaletteTool::editPaintColor(const QColor &color)
{
    if (buffer_) {
        buffer_->setColor(buffer_->paintColor(), color);
    }
}
