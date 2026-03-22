#ifndef BRUSHTOOL_H
#define BRUSHTOOL_H

#include <QAbstractButton>
#include <QImage>
#include <QPixmap>
#include <QPoint>
#include "tool.h"

class UndoBuffer;

class BrushWellButton : public QAbstractButton
{
    Q_OBJECT
public:
    explicit BrushWellButton(QWidget *parent = nullptr);
    void store(const QImage &image);
    void clear();
    bool isEmpty() const;
    const QImage &brushImage() const;
    void paintEvent(QPaintEvent *event) override;
signals:
    void ctrlClicked();
protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    bool empty_;
    QImage brushImage_;
    QPixmap thumbnail_;
};

class BrushTool : public Tool
{
    Q_OBJECT

public:
    explicit BrushTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    QWidget *createOptionsWidget() override;

private slots:
    void wellClicked(int index);
    void wellCtrlClicked(int index);

private:
    QRect changes(const QPoint &point);
    QRect draw(const QPoint &point);
    void storeToWell(int index);

    QPoint startPoint;
    UndoBuffer *undoBuffer;

    static const int WellCount = 8;
    QImage wells_[WellCount];
    BrushWellButton *wellButtons_[WellCount];

    static BrushTool instance;
};

#endif // BRUSHTOOL_H
