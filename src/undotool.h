#ifndef UNDOTOOL_H
#define UNDOTOOL_H

#include "tool.h"

class UndoTool : public Tool
{
    Q_OBJECT

public:
    explicit UndoTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;

private:
    static UndoTool instance;
};

#endif // UNDOTOOL_H
