#ifndef FIXBACKGROUNDTOOL_H
#define FIXBACKGROUNDTOOL_H

#include "tool.h"

class FixBackgroundTool : public Tool
{
    Q_OBJECT

public:
    explicit FixBackgroundTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &, const Qt::KeyboardModifiers &) override { return QRect(); }
    QRect move(const QPoint &) override { return QRect(); }
    QRect release(const QPoint &) override { return QRect(); }
    void addButtonToGridLayout(QGridLayout *layout) override;

    static FixBackgroundTool instance;

protected:
    void registerTool() override;
    void activate() override;

private slots:
    void syncButtonState();
};

#endif // FIXBACKGROUNDTOOL_H
