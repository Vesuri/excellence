#ifndef AIRBRUSHTOOL_H
#define AIRBRUSHTOOL_H

#include <QPoint>
#include <QTimer>
#include "tool.h"

class Brush;
class PenTip;
namespace Ui { class AirbrushToolOptions; }

class AirbrushTool : public Tool
{
    Q_OBJECT

public:
    enum SprayMode { FineSpray, Splatter, ShapeAirbrush };

    explicit AirbrushTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QString name() const override;
    bool restrictToColorAndRandom() const override { return sprayMode_ != Splatter; }
    bool allowsBrushModeButton() const override { return sprayMode_ == ShapeAirbrush; }
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;
    QWidget* createOptionsWidget() override;

private slots:
    void sprayTick();

private:
    void setMode(SprayMode mode);
    QRect sprayDots();
    QPoint randomNozzlePoint() const;
    QRect paintDot(const QPoint &point);

    SprayMode sprayMode_;
    bool erasing_;
    QPoint center_;
    QTimer *timer_;
    Brush *sprayBrush_ = nullptr;
    PenTip *sprayTip_ = nullptr;

    int nozzleRadius_;  // pixels
    int flow_;          // 0–100
    int focus_;         // 0–100 (100 = all at center)

    Ui::AirbrushToolOptions *ui_ = nullptr;

    static AirbrushTool instance;
    static const char *icons[];
};

#endif // AIRBRUSHTOOL_H
