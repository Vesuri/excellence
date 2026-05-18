#ifndef DRAWMODETOOL_H
#define DRAWMODETOOL_H

#include <QList>
#include <QPair>
#include <QWidget>
#include "buffer.h"
#include "gradientrange.h"
#include "tool.h"

class QCheckBox;
class QRadioButton;

namespace Ui { class DrawModeToolOptions; }

class DrawModeTool : public Tool
{
    Q_OBJECT

public:
    static DrawModeTool instance;

    explicit DrawModeTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &, const Qt::KeyboardModifiers &) override { return {}; }
    QRect move(const QPoint &) override { return {}; }
    QRect release(const QPoint &) override { return {}; }
    void addButtonToGridLayout(QGridLayout *layout) override;
    void activateModeByKey(Buffer::PaintMode mode);
    void toggleReplaceMode();

protected:
    void registerTool() override;
    void activate() override;
    QWidget *createOptionsWidget() override;

signals:
    void activeModeChanged(Buffer::PaintMode mode);

private slots:
    void onToolChanged(Tool *tool);
    void onPaintModeChanged(Buffer::PaintMode mode);
    void onPenChanged(Pen *pen);

private:
    void applyMode();
    void updateAvailability();
    bool isModeAvailable(Buffer::PaintMode mode) const;

    Buffer::PaintMode previousMode_;
    bool brushModeActive_ = false;
    bool fillModeSelected_ = false;
    void setFillModeSelected(bool v) { fillModeSelected_ = fillModeSelected = v; }
    Ui::DrawModeToolOptions *ui_ = nullptr;
    QList<QRadioButton *> generalModeBtns_;    // disabled when restrictToColorAndRandom
    QList<QRadioButton *> fillSensitiveBtns_;  // disabled when restrictToColorAndRandom or hasFill
    QList<QPair<QRadioButton *, GradientFillMode>> fillModeBtns_;
};

inline QString effectiveDrawModeName(Buffer::PaintMode paintMode)
{
    if (paintMode == Buffer::BrushMode)
        return paintModeName(paintMode);
    if (!drawModeActive)
        return paintModeName(Buffer::Color);
    if (fillModeSelected || paintMode == Buffer::Color)
        return gradientFillModeName(activeGradientFillMode);
    return paintModeName(paintMode);
}

#endif // DRAWMODETOOL_H
