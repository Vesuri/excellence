#ifndef RECTANGLETOOL_H
#define RECTANGLETOOL_H

#include <QPoint>
#include "tool.h"
#include "gradientrubberband.h"

class UndoBuffer;

namespace Ui { class RectangleToolOptions; }

class RectangleTool : public Tool
{
    Q_OBJECT

public:
    enum DrawMode { Rectangle, FilledRectangle };
    enum AnchorMode { CornerToCorner, CenterToCorner };

    explicit RectangleTool(QObject *parent = nullptr);

    void setDrawMode(const DrawMode &drawMode);
    void setBuffer(Buffer *buffer) override;
    QString name() const override;
    bool hasFill() const override { return drawMode == FilledRectangle; }
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    QRect hover(const QPoint &point) override;
    void cancel() override;
    QString status() const override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;
    QWidget* createOptionsWidget() override;

private slots:
    void setAnchorMode(bool centerToCorner);

private:
    QRect changes(const QPoint &point);
    QRect draw(const QPoint &point);
    QRect applyGradientRect(const QRect &fillRect, const QPoint &gradFrom, const QPoint &gradTo);
    QRect drawGradientRect(const QRect &fillRect, const QPoint &current);
    void cornerPoints(const QPoint &current, QPoint &p0, QPoint &p1) const;

    DrawMode drawMode;
    AnchorMode anchorMode_;
    QPoint startPoint;
    UndoBuffer *undoBuffer;
    GradientRubberBand rubberBand_;
    QRect pendingFillRect_;

    Ui::RectangleToolOptions *ui_ = nullptr;

    static RectangleTool instance;
    static const char* icons[];
};

#endif // RECTANGLETOOL_H
