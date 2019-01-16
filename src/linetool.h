#ifndef LINETOOL_H
#define LINETOOL_H

#include <QPoint>
#include "tool.h"

class UndoBuffer;

class LineTool : public Tool
{
    Q_OBJECT

public:
    explicit LineTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;

private:
    QRect changes(const QPoint &point);
    QRect draw(const QPoint &point);

    QPoint startPoint;
    UndoBuffer *undoBuffer;

    static LineTool instance;
};

#endif // LINETOOL_H
