#ifndef CLEARTOOL_H
#define CLEARTOOL_H

#include "tool.h"

class ClearTool : public Tool
{
    Q_OBJECT

public:
    explicit ClearTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

public slots:
    void clearWithEraseColor();

protected:
    void registerTool() override;

private:
    static ClearTool instance;
};

#endif // CLEARTOOL_H
