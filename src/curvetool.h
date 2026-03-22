#ifndef CURVETOOL_H
#define CURVETOOL_H

#include <QPoint>
#include "tool.h"

class CurveTool : public Tool
{
    Q_OBJECT

public:
    explicit CurveTool(QObject *parent = nullptr);

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
    QRect drawCurve(const QPoint &p0, const QPoint &p2, const QPoint &controlMid);
    QRect curveBoundingRect(const QPoint &p0, const QPoint &p2, const QPoint &controlMid) const;
    QRect draw(const QPoint &point);

    int phase_;       // 0=idle, 1=P0 set, 2=P0+P2 set (bending)
    QPoint p0_;
    QPoint p2_;

    static CurveTool instance;
};

#endif // CURVETOOL_H
