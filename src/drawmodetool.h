#ifndef DRAWMODETOOL_H
#define DRAWMODETOOL_H

#include <QWidget>
#include "buffer.h"
#include "tool.h"

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

private:
    void applyMode();

    Buffer::PaintMode selectedMode_;
    QWidget *fillGroupWidget_ = nullptr;
    static DrawModeTool instance;
};

#endif // DRAWMODETOOL_H
