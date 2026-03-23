#ifndef FILLTOOL_H
#define FILLTOOL_H

#include <QByteArray>
#include "tool.h"

class FillTool : public Tool
{
    Q_OBJECT

public:
    explicit FillTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    bool hasFill() const override { return true; }
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;

private:
    QRect flatFill(const QPoint &seed, int fillColor);
    QRect collectRegion(const QPoint &seed);
    QRect applyGradientFill(const QPoint &endPoint);

    QPoint startPoint_;
    QByteArray visited_;
    int visitedW_ = 0;
    int visitedH_ = 0;
    QRect visitedRect_;
    bool gradientPending_ = false;

    static FillTool instance;
};

#endif // FILLTOOL_H
