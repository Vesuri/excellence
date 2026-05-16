#ifndef LINETOOL_H
#define LINETOOL_H

#include <QList>
#include <QPoint>
#include "tool.h"
#include "gradientrubberband.h"

class UndoBuffer;

class LineTool : public Tool
{
    Q_OBJECT

public:
    enum Mode { Line, ConnectedLines, FilledPolygon };

    explicit LineTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QString name() const override;
    bool hasFill() const override { return mode_ == FilledPolygon; }
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    QRect hover(const QPoint &point) override;
    QRect doubleClick(const QPoint &point) override;
    void cancel() override;
    bool isInRubberBandMode() const override { return rubberBand_.pending; }
    void addButtonToGridLayout(QGridLayout *layout) override;
    QString status() const override;

protected:
    void registerTool() override;
    void activate() override;

private:
    void updateButton();
    void resetConnectedState();
    QRect drawPixel(const QPoint &point);
    QRect paintPixel(const QPoint &point);
    QRect lineBoundingRect(const QPoint &from, const QPoint &to) const;
    QRect polygonFill();
    QRect applyPolygonGradient(const QList<QPoint> &verts, const QPoint &gradFrom, const QPoint &gradTo);
    QRect startLinearRubberBand(QRect changedRect);

    Mode mode_;

    // Single-line state
    QPoint startPoint_;
    QPoint currentPoint_;
    UndoBuffer *undoBuffer_;

    // Connected / filled polygon state
    bool active_;
    QPoint firstPoint_;
    QPoint lastPoint_;
    QList<QPoint> vertices_;
    UndoBuffer *dragUndoBuffer_;

    GradientRubberBand rubberBand_;
    QList<QPoint> pendingVertices_;

    static LineTool instance;
};

#endif // LINETOOL_H
