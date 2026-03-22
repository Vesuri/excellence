#ifndef CARVEBRUSHTOOL_H
#define CARVEBRUSHTOOL_H

#include <QPolygon>
#include "tool.h"

class UndoBuffer;

class CarveBrushTool : public Tool
{
    Q_OBJECT

public:
    explicit CarveBrushTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;

private:
    QPolygon polygon_;
    QPoint prevPoint_;
    UndoBuffer *undoBuffer_;

    static CarveBrushTool instance;
};

#endif // CARVEBRUSHTOOL_H
