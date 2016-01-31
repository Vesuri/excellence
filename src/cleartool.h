#ifndef CLEARTOOL_H
#define CLEARTOOL_H

#include "tool.h"

class ClearTool : public Tool
{
    Q_OBJECT

public:
    explicit ClearTool(QObject *parent = 0);

    virtual void setBuffer(Buffer *buffer);
    virtual QRect press(const QPoint &point);
    virtual QRect move(const QPoint &point);
    virtual QRect release(const QPoint &point);
    virtual void addButtonToGridLayout(QGridLayout *layout);

protected:
    virtual void registerTool();

private:
    static ClearTool instance;
};

#endif // CLEARTOOL_H
