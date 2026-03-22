#ifndef GRIDLOCKTOOL_H
#define GRIDLOCKTOOL_H

#include "tool.h"

class QSpinBox;

class GridLockTool : public Tool
{
    Q_OBJECT

public:
    explicit GridLockTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &, const Qt::KeyboardModifiers &) override { return QRect(); }
    QRect move(const QPoint &) override { return QRect(); }
    QRect release(const QPoint &) override { return QRect(); }
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;
    QWidget *createOptionsWidget() override;

private slots:
    void syncButtonState();
    void fitToBrush();

private:
    void syncSpinboxes();

    QSpinBox *spinW_;
    QSpinBox *spinH_;
    QSpinBox *spinOX_;
    QSpinBox *spinOY_;

    static GridLockTool instance;
};

#endif // GRIDLOCKTOOL_H
