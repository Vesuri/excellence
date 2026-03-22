#ifndef BRUSHTOOL_H
#define BRUSHTOOL_H

#include <QAbstractButton>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPoint>
#include <QPolygon>
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

class BrushTool;

class BrushHandleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BrushHandleWidget(BrushTool *tool, QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
private:
    BrushTool *tool_;
};

class BrushTool : public Tool
{
    Q_OBJECT

public:
    enum Mode { Rectangle, Freehand };

    explicit BrushTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;
    QWidget *createOptionsWidget() override;

public:
    QImage currentBrushImage() const;
    QPoint currentHandleOffset() const;

public slots:
    void setHandle(const QPoint &offset);

private slots:
    void wellClicked(int index);
    void wellCtrlClicked(int index);
    void setHandleTopLeft();
    void setHandleTopRight();
    void setHandleCenter();
    void setHandleBottomLeft();
    void setHandleBottomRight();
    void brushFlipH();
    void brushFlipV();
    void brushRotate90CW();
    void brushRotate90CCW();
    void brushDouble();
    void brushHalve();
    void brushShearXPlus();
    void brushShearXMinus();
    void brushShearYPlus();
    void brushShearYMinus();
    void brushBendXPlus();
    void brushBendXMinus();
    void brushBendYPlus();
    void brushBendYMinus();
    void brushOutline();
    void brushTrim();
    void brushRestore();
    void brushTileCut();

private:
    void updateButton();
    QRect changes(const QPoint &point);
    QRect draw(const QPoint &point);
    void storeToWell(int index);

    Mode mode_;
    QPoint startPoint_;
    UndoBuffer *undoBuffer_;

    // Freehand (carve) mode state
    QPolygon polygon_;
    QPoint prevPoint_;

    static const int WellCount = 8;
    QImage wells_[WellCount];
    BrushWellButton *wellButtons_[WellCount];
    BrushHandleWidget *handleWidget_;
    QLabel *dimensionsLabel_;

    static BrushTool instance;
};

#endif // BRUSHTOOL_H
