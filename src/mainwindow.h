#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QRgb>

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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void initialize();
    void updatePalette();
    void runPaletteActionForPaintColor(unsigned paletteIndex);
    void runPaletteActionForEraseColor(unsigned paletteIndex);
    void openFile(const QString &path = QString());
    void saveFile(const QString &path = QString());
    void saveAs();
    void newWindow();
    void closeWindow();
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
    void brushDouble();
    void brushHalve();
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
    void openMagnifiedView(int zoomLevel);
    void openMagnifiedViewAt(int zoomLevel, QPoint point);

private:
    enum PaletteMode { Pick, ImageCopy, ImageSwap, PaletteCopy, PaletteSwap, PaletteSwapAndRemap, PaletteSpread };

    void updateWindowTitle(int paletteIndex = -1);
    QImage convertToIndexed(const QImage &source) const;

    Ui::MainWindow *ui;
    QFileDialog *openDialog;
    QFileDialog *loadPaletteDialog;
    QFileDialog *savePaletteDialog;
    PropertiesDialog *propertiesDialog;
    QVector<BufferView *> bufferViews;
    BufferView *activeBufferView;
    Buffer *buffer;
    PenTip *penTip;
    PenTip *toolPenTip;
    PaletteMode paletteMode;
    QVector<QRgb> paletteUndoSnapshot_;
    QVector<QRgb> paletteRestorePoint_;
    bool saveWithTransparency_ = false;
};

#endif // MAINWINDOW_H
