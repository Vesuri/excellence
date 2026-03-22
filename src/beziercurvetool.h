#ifndef BEZIERCURVETOOL_H
#define BEZIERCURVETOOL_H

#include <QPoint>
#include "tool.h"
#include "undobuffer.h"

class BezierCurveTool : public Tool
{
    Q_OBJECT

public:
    explicit BezierCurveTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    QRect hover(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;

private:
    QRect drawCubicBezier(const QPoint &p0, const QPoint &p1,
                          const QPoint &p2, const QPoint &p3);
    QRect drawHandle(const QPoint &center);
    QRect drawDashedLine(const QPoint &from, const QPoint &to);
    QRect drawPreview(const QPoint &cursor);
    QRect previewBoundingRect(const QPoint &cursor) const;
    int nearestHandle(const QPoint &point) const;
    void resetState();

    int phase_;         // 0=idle, 1=P0, 2=P0+P3, 3=P0+P3+P1, 4=reshape
    QPoint p0_, p1_, p2_, p3_;
    int draggedHandle_; // -1=none, 0=P0, 1=P1, 2=P2, 3=P3
    UndoBuffer *undoBuffer_;

    static BezierCurveTool instance;
};

#endif // BEZIERCURVETOOL_H
