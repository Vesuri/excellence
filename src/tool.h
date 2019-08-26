#ifndef TOOL_H
#define TOOL_H

#include <QObject>
#include <QToolButton>
#include <QList>

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
    virtual void setBuffer(Buffer *buffer);

    virtual QRect press(const QPoint &point) = 0;
    virtual QRect move(const QPoint &point) = 0;
    virtual QRect release(const QPoint &point) = 0;
    virtual void addButtonToGridLayout(QGridLayout *layout) = 0;
    virtual Type type() const;

protected slots:
    virtual void registerTool();
    virtual void activate();
    virtual void setCheckedIfEqual(Tool *tool);

protected:
    Qt::MouseButton mouseButton_;
    Buffer *buffer_;
    QToolButton *button_;
};

extern QList<Tool *> tools;

#endif // TOOL_H
