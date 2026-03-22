#ifndef AIRTOOL_H
#define AIRTOOL_H

#include <QPoint>
#include <QTimer>
#include "tool.h"

class AirTool : public Tool
{
    Q_OBJECT

public:
    enum SprayMode { FineSpray, Splatter, ShapeAirbrush };

    explicit AirTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
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
    void setSprayMode(SprayMode mode);

private:
    void setMode(SprayMode mode);
    QRect sprayDots();
    QPoint randomNozzlePoint() const;
    QRect paintDot(const QPoint &point);

    SprayMode sprayMode_;
    bool erasing_;
    QPoint center_;
    QTimer *timer_;

    int nozzleRadius_;  // pixels
    int flow_;          // 0–100
    int focus_;         // 0–100 (100 = all at center)

    static AirTool instance;
    static const char *icons[];
};

#endif // AIRTOOL_H
