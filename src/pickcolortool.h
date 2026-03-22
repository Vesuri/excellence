#ifndef PICKCOLORTOOL_H
#define PICKCOLORTOOL_H

#include "tool.h"

class PickColorTool : public Tool
{
    Q_OBJECT

public:
    explicit PickColorTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;

private:
    static PickColorTool instance;
};

#endif // PICKCOLORTOOL_H
