#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "rawsaveoptionsdialog.h"
#include <QVector>
#include <QRgb>
#include <QPoint>

namespace Ui {
class MainWindow;
}

class QFileDialog;
class PropertiesDialog;
class QGraphicsScene;
class PaletteItem;
class BufferView;
class Buffer;
class PenTip;
class QFrame;
class QLabel;
class CurrentColorsButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *e) override;

private slots:
    void initialize();
    void updatePalette();
    void runPaletteActionForPaintColor(unsigned paletteIndex);
    void runPaletteActionForEraseColor(unsigned paletteIndex);
    void openFile(const QString &path = QString());
    void saveFile(const QString &path = QString(), const RawSaveOptions &rawOptions = RawSaveOptions());
    void saveAs();
    void newWindow();
    void closeWindow();
    void toggleFloatPanels(bool checked);
    void loadPalette(const QString &path = QString());
    void showProperties();
    void setBuffer(Buffer *buffer);
    void imageHistogram();
    void imageCopy();
    void imagePaste();
    void imageCopyColor();
    void imageSwapColors();
    void paletteCopyColor();
    void paletteSwapColors();
    void paletteSwapAndRemapColors();
    void brushLoad();
    void brushSave();
    void brushCopy();
    void brushPaste();
    void brushDelete();
    void brushRemap();
    void brushBgFgSwap();
    void brushAutoBackground();
    void brushFlipHorizontal();
    void brushFlipVertical();
    void brushRotate90CW();
    void brushRotate90CCW();
    void brushRotateNumeric();
    void brushDouble();
    void brushDoubleX();
    void brushDoubleY();
    void brushHalve();
    void brushHalveX();
    void brushHalveY();
    void brushScaleToSize();
    void brushShearX();
    void brushShearY();
    void brushBendX();
    void brushBendY();
    void brushOutline();
    void brushTrim();
    void brushTileCut();
    void brushRestore();
    void pickForegroundColor();
    void pickBackgroundColor();
    void paletteSave(const QString &path = QString());
    void paletteSpread();
    void paletteDefault();
    void paletteRestore();
    void paletteUndo();
    void paletteRemapPage();
    void about();
    void onDirtyChanged(bool dirty);
    void openMagnifiedViewAt(int zoomLevel, QPoint point);
    void updateStatusBarStatic();
    void updateCursorStatus(QPoint point, bool valid);
    void squashDialogs();
    void toggleAllDialogs();
    void toggleSingleWindowMode(bool checked);

private:
    enum PaletteMode { Pick, ImageCopy, ImageSwap, PaletteCopy, PaletteSwap, PaletteSwapAndRemap, PaletteSpread };

    void updateWindowTitle();
    QImage convertToIndexed(const QImage &source) const;
    class Brush *brushForTransform();
    QVector<QWidget *> collectAndHideToolDialogs();

    Ui::MainWindow *ui;
    QFileDialog *openDialog;
    QFileDialog *loadPaletteDialog;
    QFileDialog *savePaletteDialog;
    PropertiesDialog *propertiesDialog;
    QVector<BufferView *> bufferViews;
    BufferView *activeBufferView = nullptr;
    Buffer *buffer;
    PenTip *penTip;
    PenTip *toolPenTip;
    PaletteMode paletteMode;
    QVector<QRgb> paletteUndoSnapshot_;
    QVector<QRgb> paletteRestorePoint_;
    bool saveWithTransparency_ = false;

    CurrentColorsButton *statusColorsButton_;
    QLabel *statusToolLabel_;
    QLabel *statusModeLabel_;
    QLabel *statusBrushLabel_;
    QFrame *statusBrushSep_;
    QFrame *statusCoordsSep_;
    QLabel *statusCoordsLabel_;

    QPoint lastCursorPoint_;
    bool lastCursorValid_ = false;
    bool fullScreenActivatedSingleWindowMode_ = false;
    QVector<QWidget *> squashedDialogs_;
    QVector<QWidget *> hiddenDialogs_;
};

#endif // MAINWINDOW_H
