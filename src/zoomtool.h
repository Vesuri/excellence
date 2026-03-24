#ifndef ZOOMTOOL_H
#define ZOOMTOOL_H

#include "tool.h"

class ZoomTool : public Tool
{
    Q_OBJECT

public:
    explicit ZoomTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;
    Type type() const override;

protected:
    void registerTool() override;
    void activate() override;
    QWidget *createOptionsWidget() override;

private:
    bool placeMagnifierMode_;
    int magnifierZoom_;

    static ZoomTool instance;
};

#endif // ZOOMTOOL_H
