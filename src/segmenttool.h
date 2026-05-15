#ifndef SEGMENTTOOL_H
#define SEGMENTTOOL_H

#include "tool.h"

class QRadioButton;
class QSpinBox;

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
    QRadioButton *radioDistance_;
    QRadioButton *radioPoints_;
    QSpinBox *spinValue_;
};

#endif // SEGMENTTOOL_H
