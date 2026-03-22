#ifndef FILLEDPOLYGONTOOL_H
#define FILLEDPOLYGONTOOL_H

#include <QPoint>
#include <QList>
#include "tool.h"

class FilledPolygonTool : public Tool
{
    Q_OBJECT

public:
    explicit FilledPolygonTool(QObject *parent = nullptr);

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
    QRect paint(const QPoint &point);
    QRect lineBoundingRect(const QPoint &from, const QPoint &to) const;
    QPoint centroid() const;
    QRect floodFill(const QPoint &seed);

    bool active_;
    QPoint firstPoint_;
    QPoint lastPoint_;
    QList<QPoint> vertices_;

    static FilledPolygonTool instance;
};

#endif // FILLEDPOLYGONTOOL_H
