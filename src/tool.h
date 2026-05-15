#ifndef TOOL_H
#define TOOL_H

#include <QObject>
#include <QToolButton>
#include <QList>
#include <QWidget>

class Buffer;
class QToolButton;
class QGridLayout;

class Tool : public QObject
{
    Q_OBJECT

public:
    enum Type{ Modify, Zoom };

    explicit Tool(QObject *parent = nullptr);

    void setMouseButton(const Qt::MouseButton &mouseButton);
    Qt::MouseButton mouseButton() const;
    virtual void setBuffer(Buffer *buffer);

    void click();
    virtual QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) = 0;
    virtual QRect move(const QPoint &point) = 0;
    virtual QRect release(const QPoint &point) = 0;
    virtual QRect hover(const QPoint &point);
    virtual QRect doubleClick(const QPoint &point) { Q_UNUSED(point) return QRect(); }
    virtual void cancel() {}
    virtual void addButtonToGridLayout(QGridLayout *layout) = 0;
    virtual Type type() const;
    virtual QString name() const;
    virtual QString status() const;
    virtual bool hasFill() const { return false; }
    virtual bool restrictToColorAndRandom() const { return false; }
    virtual bool allowsBrushModeButton() const { return true; }

public slots:
    void toggleOptionsWidget();

protected slots:
    virtual void registerTool();
    virtual void activate();
    virtual void setCheckedIfEqual(Tool *tool);

protected:
    virtual QWidget* createOptionsWidget();

    Qt::MouseButton mouseButton_;
    Buffer *buffer_;
    QToolButton *button_;
    QWidget *optionsWidget_;
};

extern QList<Tool *> tools;

#endif // TOOL_H
