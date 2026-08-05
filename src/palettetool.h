#ifndef PALETTETOOL_H
#define PALETTETOOL_H

#include "tool.h"

class QColorDialog;

class PaletteTool : public Tool
{
    Q_OBJECT

public:
    explicit PaletteTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;

private slots:
    void toggleColorDialogVisibility();
    void setPaintColor(unsigned colorIndex, const QColor &color);
    void editPaintColor(const QColor &color);

private:
    static PaletteTool instance;
    QColorDialog *colorDialog;
};

#endif // PALETTETOOL_H
