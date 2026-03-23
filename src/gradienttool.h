#ifndef GRADIENTTOOL_H
#define GRADIENTTOOL_H

#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include "tool.h"
#include "gradientmarkerbox.h"

class GradientTool : public Tool
{
    Q_OBJECT

public:
    static GradientTool instance;

    void setActiveRange(int index);
    void refreshPanel();

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
    void onRangeChanged();

private:
    GradientTool();

    GradientMarkerBox *markerBox_ = nullptr;
    QLabel *colorsLabel_ = nullptr;
    QSpinBox *spreadSpin_ = nullptr;
    QList<QPushButton *> rangeButtons_;
};

#endif // GRADIENTTOOL_H
