#ifndef CLEARTOOL_H
#define CLEARTOOL_H

#include "tool.h"

class ClearTool : public Tool
{
    Q_OBJECT

public:
    explicit ClearTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;

private:
    static ClearTool instance;
};

#endif // CLEARTOOL_H
