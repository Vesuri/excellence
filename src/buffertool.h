#ifndef BUFFERTOOL_H
#define BUFFERTOOL_H

#include "tool.h"

class BufferTool : public Tool
{
    Q_OBJECT

public:
    static BufferTool instance;

    explicit BufferTool(QObject *parent = nullptr);

    QRect press(const QPoint &, const Qt::KeyboardModifiers &) override { return QRect(); }
    QRect move(const QPoint &) override { return QRect(); }
    QRect release(const QPoint &) override { return QRect(); }
    void addButtonToGridLayout(QGridLayout *layout) override;

signals:
    void showBuffersRequested();

protected:
    void registerTool() override;
    void activate() override;
};

#endif // BUFFERTOOL_H
