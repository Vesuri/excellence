#ifndef PENTIPTOOL_H
#define PENTIPTOOL_H

#include "tool.h"

class PenTipTool : public Tool
{
    Q_OBJECT

public:
    explicit PenTipTool(QObject *parent = nullptr);

    QRect press(const QPoint &, const Qt::KeyboardModifiers &) override { return {}; }
    QRect move(const QPoint &) override { return {}; }
    QRect release(const QPoint &) override { return {}; }
    void setBuffer(Buffer *buffer) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;
    QWidget *createOptionsWidget() override;

private:
    void updateButtonIcon();

    static PenTipTool instance;
};

#endif // PENTIPTOOL_H
