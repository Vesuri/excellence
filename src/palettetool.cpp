#include <QGridLayout>
#include <QColorDialog>
#include "buffer.h"
#include "palettetool.h"

PaletteTool PaletteTool::instance;

PaletteTool::PaletteTool(QObject *parent) : Tool(parent),
    colorDialog(nullptr)
{
}

void PaletteTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer, SIGNAL(paintColorChanged(unsigned, QColor)), this, SLOT(setPaintColor(unsigned, QColor)));
        disconnect(colorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(editPaintColor(QColor)));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        setPaintColor(buffer->paintColor(), buffer->image().color(static_cast<int>(buffer->paintColor())));

        connect(buffer, SIGNAL(paintColorChanged(unsigned, QColor)), this, SLOT(setPaintColor(unsigned, QColor)));
        connect(colorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(editPaintColor(QColor)));
    }
}

QRect PaletteTool::press(const QPoint &, const Qt::KeyboardModifiers &)
{
    return QRect();
}

QRect PaletteTool::move(const QPoint &)
{
    return QRect();
}

QRect PaletteTool::release(const QPoint &)
{
    return QRect();
}

void PaletteTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/palette.png"));
    connect(button_, SIGNAL(clicked(bool)), this, SLOT(toggleColorDialogVisibility()));

    colorDialog = new QColorDialog;
    colorDialog->setOption(QColorDialog::DontUseNativeDialog);
    colorDialog->setOption(QColorDialog::NoButtons);
}

void PaletteTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 1);
}

void PaletteTool::toggleColorDialogVisibility()
{
    colorDialog->setVisible(!colorDialog->isVisible());
}

void PaletteTool::setPaintColor(unsigned, const QColor &color)
{
    colorDialog->setCurrentColor(color);
}

void PaletteTool::editPaintColor(const QColor &color)
{
    if (buffer_) {
        buffer_->setColor(buffer_->paintColor(), color);
    }
}
