#ifndef BRUSHTOOL_H
#define BRUSHTOOL_H

#include <QPoint>
#include "tool.h"

class UndoBuffer;

class BrushTool : public Tool
{
    Q_OBJECT

public:
    explicit BrushTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
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

    static BrushTool instance;
};

#endif // BRUSHTOOL_H
