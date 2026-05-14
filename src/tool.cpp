#include <QTimer>
#include <QPushButton>
#include "buffer.h"
#include "tool.h"

QList<Tool *> tools;

Tool::Tool(QObject *parent) : QObject(parent),
    mouseButton_(Qt::NoButton),
    buffer_(nullptr),
    optionsWidget_(nullptr)
{
    QTimer::singleShot(0, this, SLOT(registerTool()));
}

void Tool::setMouseButton(const Qt::MouseButton &mouseButton)
{
    mouseButton_ = mouseButton;
}

Qt::MouseButton Tool::mouseButton() const
{
    return mouseButton_;
}

void Tool::setBuffer(Buffer *buffer)
{
    buffer_ = buffer;
}

QRect Tool::hover(const QPoint &)
{
    return QRect();
}

Tool::Type Tool::type() const
{
    return Modify;
}

QString Tool::name() const
{
    QString tip = button_->toolTip();
    int nl = tip.indexOf('\n');
    if (nl >= 0) tip = tip.left(nl);
    int bracket = tip.indexOf(" [");
    return bracket >= 0 ? tip.left(bracket) : tip;
}

QString Tool::status() const
{
    return QString();
}

void Tool::click()
{
    button_->click();
}

void Tool::registerTool()
{
    button_ = new QPushButton;
    button_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(button_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(toggleOptionsWidget()));
    tools.append(this);
}

void Tool::toggleOptionsWidget()
{
    if (optionsWidget_ == nullptr) {
        optionsWidget_ = createOptionsWidget();
        if (optionsWidget_ == nullptr) {
            return;
        }
        optionsWidget_->setWindowFlags(Qt::Tool);
    }
    optionsWidget_->setVisible(!optionsWidget_->isVisible());
}

QWidget* Tool::createOptionsWidget()
{
    return nullptr;
}

void Tool::activate()
{
    buffer_->setTool(this);
}

void Tool::setCheckedIfEqual(Tool *tool)
{
    button_->setChecked(tool == this);
}
