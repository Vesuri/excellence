#ifndef ELLIPSETOOL_H
#define ELLIPSETOOL_H

#include <QPoint>
#include "tool.h"
#include "undobuffer.h"

class EllipseTool : public Tool
{
    Q_OBJECT

public:
    enum DrawMode { Ellipse, FilledEllipse };
    enum AnchorMode { CornerToCorner, CenterToCorner };

    explicit EllipseTool(QObject *parent = nullptr);

    void setDrawMode(DrawMode mode);
    void setBuffer(Buffer *buffer) override;
    bool hasFill() const override { return drawMode_ == FilledEllipse; }
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    QRect hover(const QPoint &point) override;
    void cancel() override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;
    QWidget* createOptionsWidget() override;

private slots:
    void setAnchorMode(bool centerToCorner);
    void setRotateMode(bool rotate);

private:
    void resetState();
    void cornerPoints(const QPoint &current, QPoint &p0, QPoint &p1) const;
    void computeEllipseParams(const QPoint &p0, const QPoint &p1);
    QRect drawEllipseShape(double angle, bool applyGradient = false);
    QRect ellipseBoundingRect(double angle) const;
    QRect draw(const QPoint &point);

    DrawMode drawMode_;
    AnchorMode anchorMode_;
    bool rotateMode_;
    bool erasing_;
    int phase_;
    QPoint startPoint_;
    int cx_, cy_, rx_, ry_;
    double rotationAngle_;
    UndoBuffer *undoBuffer_;

    static EllipseTool instance;
    static const char *icons[];
};

#endif // ELLIPSETOOL_H
