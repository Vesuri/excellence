#ifndef PENTIPTOOL_H
#define PENTIPTOOL_H

#include "pen.h"
#include "tool.h"

class UndoBuffer;

namespace Ui { class PenTipToolOptions; }

class PenTipTool : public Tool
{
    Q_OBJECT

public:
    enum SizingMode { SizingNone, SizingCircle, SizingRect };

    explicit PenTipTool(QObject *parent = nullptr);

    QRect press(const QPoint &point, const Qt::KeyboardModifiers &) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    QRect hover(const QPoint &point) override;
    void cancel() override;
    void setBuffer(Buffer *buffer) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;
    QWidget *createOptionsWidget() override;

private slots:
    void onPenChanged(Pen *pen);

private:
    void updateButtonIcon();
    void activateSizing(SizingMode mode);
    QRect drawCirclePreview(const QPoint &center, int size);
    QRect drawRectPreview(const QPoint &center, int size);
    static QRect centeredPreviewRect(const QPoint &center, int size);

    SizingMode sizingMode_;
    QPoint startPoint_;
    int lastCircleSize_;
    int lastRectSize_;
    UndoBuffer *undoBuffer_;

    Ui::PenTipToolOptions *ui_ = nullptr;

    static PenTipTool instance;
};

#endif // PENTIPTOOL_H
