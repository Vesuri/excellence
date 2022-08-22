#ifndef RECTANGLETOOL_H
#define RECTANGLETOOL_H

#include <QPoint>
#include "tool.h"

class UndoBuffer;

class RectangleTool : public Tool
{
    Q_OBJECT

public:
    enum DrawMode { Rectangle, FilledRectangle };

    explicit RectangleTool(QObject *parent = nullptr);

    void setDrawMode(const DrawMode &drawMode);
    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;

private:
    QRect changes(const QPoint &point);
    QRect draw(const QPoint &point);

    DrawMode drawMode;
    QPoint startPoint;
    UndoBuffer *undoBuffer;

    static RectangleTool instance;
    static const char* icons[];
};

#endif // RECTANGLETOOL_H
