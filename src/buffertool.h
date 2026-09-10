#ifndef BUFFERTOOL_H
#define BUFFERTOOL_H

#include "tool.h"

class BufferTool : public Tool
{
    Q_OBJECT

public:
    static BufferTool instance;

    explicit BufferTool(QObject *parent = nullptr);

    void addButtonToGridLayout(QGridLayout *layout) override;

signals:
    void showBuffersRequested();

protected:
    void registerTool() override;
    void activate() override;
};

#endif // BUFFERTOOL_H
