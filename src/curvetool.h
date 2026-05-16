#ifndef CURVETOOL_H
#define CURVETOOL_H

#include <QPoint>
#include "tool.h"
#include "undobuffer.h"

class CurveTool : public Tool
{
    Q_OBJECT

public:
    enum CurveMode { Quadratic, Bezier };

    explicit CurveTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QString name() const override;
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    QRect hover(const QPoint &point) override;
    QRect doubleClick(const QPoint &point) override;
    void cancel() override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;

private:
    // Shared helpers
    void setCurveMode(CurveMode mode);
    void resetState();

    // Quadratic mode
    QRect drawQuadraticCurve(const QPoint &p0, const QPoint &p2, const QPoint &controlMid);
    QRect quadraticBoundingRect(const QPoint &p0, const QPoint &p2, const QPoint &controlMid) const;
    QRect draw(const QPoint &point);

    // Bezier (cubic) mode
    QRect drawCubicBezier(const QPoint &p0, const QPoint &p1,
                          const QPoint &p2, const QPoint &p3);
    QRect drawHandle(const QPoint &center);
    QRect drawDashedLine(const QPoint &from, const QPoint &to);
    QRect drawBezierPreview(const QPoint &cursor);
    QRect bezierBoundingRect(const QPoint &cursor) const;
    int nearestHandle(const QPoint &point) const;

    CurveMode curveMode_;
    int phase_;
    bool erasing_;
    QPoint p0_, p1_, p2_, p3_;
    int draggedHandle_;
    UndoBuffer *undoBuffer_;

    static CurveTool instance;
};

#endif // CURVETOOL_H
