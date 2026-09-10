#ifndef GRADIENTTOOL_H
#define GRADIENTTOOL_H

#include <QCheckBox>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QTabBar>
#include <QTimer>
#include "tool.h"
#include "gradientmarkerbox.h"
#include "gradientrange.h"

namespace Ui { class GradientToolOptions; }

class GradientTool : public Tool
{
    Q_OBJECT

public:
    static GradientTool instance;

    void setActiveRange(int index);
    void refreshPanel();
    void toggle();

    void setBuffer(Buffer *buffer) override;
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

    Ui::GradientToolOptions *ui_ = nullptr;
    QTimer *cycleTimer_ = nullptr;
    double cycleAccumulators_[kGradientRangeCount] = {};
    bool cyclingEnabled_ = false;
};

#endif // GRADIENTTOOL_H
