#ifndef UNDOTOOL_H
#define UNDOTOOL_H

#include "tool.h"

class UndoTool : public Tool
{
    Q_OBJECT

public:
    explicit UndoTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;

private:
    static UndoTool instance;
};

#endif // UNDOTOOL_H
