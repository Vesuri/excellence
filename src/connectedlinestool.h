#ifndef CONNECTEDLINESTOOL_H
#define CONNECTEDLINESTOOL_H

#include <QPoint>
#include <QList>
#include "tool.h"

class UndoBuffer;

class ConnectedLinesTool : public Tool
{
    Q_OBJECT

public:
    enum DrawMode { Lines, FilledPolygon };

    explicit ConnectedLinesTool(QObject *parent = nullptr);

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
    void setDrawMode(DrawMode mode);
    QRect draw(const QPoint &point);
    QRect paint(const QPoint &point);
    QRect lineBoundingRect(const QPoint &from, const QPoint &to) const;
    QPoint centroid() const;
    QRect floodFill(const QPoint &seed);

    DrawMode drawMode_;
    bool active_;
    QPoint firstPoint_;
    QPoint lastPoint_;
    QList<QPoint> vertices_;
    UndoBuffer *dragUndoBuffer_;

    static ConnectedLinesTool instance;
};

#endif // CONNECTEDLINESTOOL_H
