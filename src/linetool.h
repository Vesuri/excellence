#ifndef LINETOOL_H
#define LINETOOL_H

#include <QPoint>
#include "tool.h"

class Pen;
class UndoBuffer;

class LineTool : public Tool
{
    Q_OBJECT

public:
    explicit LineTool(QObject *parent = 0);

    void setPen(Pen *pen);
    virtual QRect press(const QPoint &point);
    virtual QRect move(const QPoint &point);
    virtual QRect release(const QPoint &point);
    virtual void addButtonToGridLayout(QGridLayout *layout);

protected:
    virtual void registerTool();

private:
    QRect changes(const QPoint &point);
    QRect draw(const QPoint &point);

    Pen *pen;
    QPoint startPoint;
    UndoBuffer *undoBuffer;

    static LineTool instance;
};

#endif // LINETOOL_H
