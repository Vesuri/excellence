#ifndef GRADIENTTOOL_H
#define GRADIENTTOOL_H

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>
#include "tool.h"
#include "gradientmarkerbox.h"
#include "gradientrange.h"

class GradientTool : public Tool
{
    Q_OBJECT

public:
    static GradientTool instance;

    void setActiveRange(int index);
    void refreshPanel();
    void toggle();

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
    void onCycleTick();
    void syncButtonState();

private:
    GradientTool();
    void updateTimer();

    GradientMarkerBox *markerBox_ = nullptr;
    QLabel *colorsLabel_ = nullptr;
    QSpinBox *spreadSpin_ = nullptr;
    QCheckBox *randomCheck_ = nullptr;
    QCheckBox *hardEdgesCheck_ = nullptr;
    QSlider *ditherSlider_ = nullptr;
    QPushButton *cycleButton_ = nullptr;
    QSlider *speedSlider_ = nullptr;
    QTimer *cycleTimer_ = nullptr;
    double cycleAccumulators_[kGradientRangeCount] = {};
    bool cyclingEnabled_ = false;
    QList<QPushButton *> rangeButtons_;
};

#endif // GRADIENTTOOL_H
