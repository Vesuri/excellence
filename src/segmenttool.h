#ifndef SEGMENTTOOL_H
#define SEGMENTTOOL_H

#include "tool.h"

namespace Ui { class SegmentToolOptions; }

class SegmentTool : public Tool
{
    Q_OBJECT

public:
    explicit SegmentTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &, const Qt::KeyboardModifiers &) override { return QRect(); }
    QRect move(const QPoint &) override { return QRect(); }
    QRect release(const QPoint &) override { return QRect(); }
    void addButtonToGridLayout(QGridLayout *layout) override;

    static SegmentTool instance;

protected:
    void registerTool() override;
    void activate() override;
    QWidget *createOptionsWidget() override;

private slots:
    void syncButtonState();
    void syncWidgets();

private:
    Ui::SegmentToolOptions *ui_ = nullptr;
};

#endif // SEGMENTTOOL_H
