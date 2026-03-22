#ifndef LINETOOL_H
#define LINETOOL_H

#include <QList>
#include <QPoint>
#include "tool.h"

class UndoBuffer;

class LineTool : public Tool
{
    Q_OBJECT

public:
    enum Mode { Line, ConnectedLines, FilledPolygon };

    explicit LineTool(QObject *parent = nullptr);

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
    void updateButton();
    void resetConnectedState();
    QRect drawPixel(const QPoint &point);
    QRect paintPixel(const QPoint &point);
    QRect lineBoundingRect(const QPoint &from, const QPoint &to) const;
    QRect polygonFill();

    Mode mode_;

    // Single-line state
    QPoint startPoint_;
    UndoBuffer *undoBuffer_;

    // Connected / filled polygon state
    bool active_;
    QPoint firstPoint_;
    QPoint lastPoint_;
    QList<QPoint> vertices_;
    UndoBuffer *dragUndoBuffer_;

    static LineTool instance;
};

#endif // LINETOOL_H
