#ifndef CONNECTEDLINESTOOL_H
#define CONNECTEDLINESTOOL_H

#include <QPoint>
#include "tool.h"

class ConnectedLinesTool : public Tool
{
    Q_OBJECT

public:
    explicit ConnectedLinesTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    QRect hover(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;

private:
    QRect draw(const QPoint &point);
    QRect lineBoundingRect(const QPoint &from, const QPoint &to) const;

    bool active_;
    QPoint lastPoint_;

    static ConnectedLinesTool instance;
};

#endif // CONNECTEDLINESTOOL_H
