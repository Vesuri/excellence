#include <QDockWidget>
#include <QMainWindow>
#include <QSizePolicy>
#include <QTimer>
#include <QToolButton>
#include "buffer.h"
#include "tool.h"

QList<Tool *> tools;
QMainWindow *Tool::mainWindow_ = nullptr;
bool Tool::floatPanelsByDefault_ = true;

Tool::Tool(QObject *parent) : QObject(parent),
    mouseButton_(Qt::NoButton),
    buffer_(nullptr),
    optionsWidget_(nullptr),
    dockWidget_(nullptr)
{
    QTimer::singleShot(0, this, SLOT(registerTool()));
}

void Tool::setMainWindow(QMainWindow *mainWindow)
{
    mainWindow_ = mainWindow;
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

void Tool::connectToolChecked()
{
    if (buffer_) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
        setCheckedIfEqual(buffer_->tool());
    }
}

void Tool::disconnectToolChecked()
{
    if (buffer_)
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
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
    button_ = new QToolButton;
    button_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    button_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(button_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(toggleOptionsWidget()));
    tools.append(this);
}

void Tool::toggleOptionsWidget()
{
    if (dockWidget_ == nullptr) {
        optionsWidget_ = createOptionsWidget();
        if (optionsWidget_ == nullptr) return;
        dockWidget_ = new QDockWidget(name());
        dockWidget_->setWidget(optionsWidget_);
        QDockWidget *splitFrom = nullptr;
        if (!floatPanelsByDefault_) {
            for (Tool *t : tools) {
                if (t->dockWidget_ && !t->dockWidget_->isFloating() && t->dockWidget_->isVisible())
                    splitFrom = t->dockWidget_;
            }
        }
        if (splitFrom)
            mainWindow_->splitDockWidget(splitFrom, dockWidget_, Qt::Vertical);
        else
            mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, dockWidget_);
        QMainWindow *mw = mainWindow_;
        auto shrink = [mw]() { QTimer::singleShot(0, mw, &QWidget::adjustSize); };
        QDockWidget *dw = dockWidget_;
        connect(dw, &QDockWidget::visibilityChanged, [shrink](bool visible) { if (!visible) shrink(); });
        connect(dw, &QDockWidget::topLevelChanged, [dw, shrink](bool floating) {
            if (floating) {
                dw->setMinimumSize(0, 0);
                if (QWidget *w = dw->widget())
                    dw->setMaximumSize(w->sizeHint());
                QTimer::singleShot(0, dw, [dw]() { dw->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX); });
            }
            shrink();
        });
        if (floatPanelsByDefault_)
            dockWidget_->setFloating(true);
    } else {
        if (!dockWidget_->isVisible()) {
            if (floatPanelsByDefault_ && !dockWidget_->isFloating())
                dockWidget_->setFloating(true);
            else if (!floatPanelsByDefault_ && dockWidget_->isFloating())
                dockWidget_->setFloating(false);
        }
        dockWidget_->setVisible(!dockWidget_->isVisible());
    }
}

void Tool::hideOptionsPanel()
{
    if (dockWidget_)
        dockWidget_->hide();
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
