#ifndef STENCILTOOL_H
#define STENCILTOOL_H

#include <QList>
#include "tool.h"

class QCheckBox;
class QGridLayout;
class PaletteButton;
class QPushButton;

class StencilTool : public Tool
{
    Q_OBJECT

public:
    explicit StencilTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void cancel() override;
    void addButtonToGridLayout(QGridLayout *layout) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    void toggle();

    static StencilTool instance;

protected:
    void registerTool() override;
    void activate() override;
    QWidget *createOptionsWidget() override;

private slots:
    void syncButtonState();
    void syncWidgets();
    void onToolChanged(Tool *tool);

private:
    enum PickMode { None, Eyedropper, Lasso };

    void startEyedropper();
    void startLasso();
    void stopPicking();
    void finishLasso();
    void rebuildColorGrid();

    PickMode mode_ = None;
    Tool *previousTool_ = nullptr;
    QList<QPoint> lassoPath_;

    // Options widget
    QCheckBox *showCheck_ = nullptr;
    QPushButton *selectBtn_ = nullptr;
    QGridLayout *colorGrid_ = nullptr;
    QList<PaletteButton *> colorSwatches_;
    int gridColorCount_ = -1;
};

#endif // STENCILTOOL_H
