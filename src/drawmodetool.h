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

class DrawModeTool : public Tool
{
    Q_OBJECT

public:
    explicit DrawModeTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &, const Qt::KeyboardModifiers &) override { return {}; }
    QRect move(const QPoint &) override { return {}; }
    QRect release(const QPoint &) override { return {}; }
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;
    QWidget *createOptionsWidget() override;

signals:
    void selectedModeChanged(Buffer::PaintMode mode);

private slots:
    void onToolChanged(Tool *tool);
    void onPaintModeChanged(Buffer::PaintMode mode);
    void onPenChanged(Pen *pen);

private:
    void applyMode();
    void updateAvailability();
    bool isModeAvailable(Buffer::PaintMode mode) const;

    Buffer::PaintMode selectedMode_;
    bool fillModeSelected_ = false;
    QWidget *fillGroupWidget_ = nullptr;
    QRadioButton *brushModeBtn_ = nullptr;
    QCheckBox *replaceModeBtn_ = nullptr;
    QRadioButton *randomModeBtn_ = nullptr;
    QList<QRadioButton *> generalModeBtns_;    // disabled when restrictToColorAndRandom
    QList<QRadioButton *> fillSensitiveBtns_;  // disabled when restrictToColorAndRandom or hasFill
    QList<QPair<QRadioButton *, GradientFillMode>> fillModeBtns_;
    static DrawModeTool instance;
};

#endif // DRAWMODETOOL_H
