#ifndef MIRRORTOOL_H
#define MIRRORTOOL_H

#include "tool.h"

namespace Ui { class MirrorToolOptions; }

class MirrorTool : public Tool
{
    Q_OBJECT

public:
    explicit MirrorTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &, const Qt::KeyboardModifiers &) override { return QRect(); }
    QRect move(const QPoint &) override { return QRect(); }
    QRect release(const QPoint &) override { return QRect(); }
    void addButtonToGridLayout(QGridLayout *layout) override;

    void toggle();

    static MirrorTool instance;

protected:
    void registerTool() override;
    void activate() override;
    QWidget *createOptionsWidget() override;

private slots:
    void syncButtonState();
    void syncWidgets();

private:
    Ui::MirrorToolOptions *ui_ = nullptr;
};

#endif // MIRRORTOOL_H
