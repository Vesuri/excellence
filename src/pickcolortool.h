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

    void activateOneShotForeground(Tool *previousTool);
    void activateOneShotBackground(Tool *previousTool);

protected:
    void registerTool() override;
    void activate() override;

private:
    enum OneShotTarget { None, Foreground, Background };

    OneShotTarget oneShotTarget_;
    Tool *previousTool_;

    static PickColorTool instance;
};

#endif // PICKCOLORTOOL_H
