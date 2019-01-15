#ifndef UNDOTOOL_H
#define UNDOTOOL_H

#include "tool.h"

class UndoTool : public Tool
{
    Q_OBJECT

public:
    explicit UndoTool(QObject *parent = nullptr);

    virtual void setBuffer(Buffer *buffer);
    virtual QRect press(const QPoint &point);
    virtual QRect move(const QPoint &point);
    virtual QRect release(const QPoint &point);
    virtual void addButtonToGridLayout(QGridLayout *layout);

protected:
    virtual void registerTool();

private:
    static UndoTool instance;
};

#endif // UNDOTOOL_H
