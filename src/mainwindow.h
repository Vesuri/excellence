#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private:
    enum PaletteMode { Pick, ImageCopy, ImageSwap, PaletteCopy, PaletteSwap, PaletteSwapAndRemap };

    void updateWindowTitle(int paletteIndex = -1);
    QImage convertToIndexed(const QImage &source) const;

    Ui::MainWindow *ui;
    QFileDialog *openDialog;
    QFileDialog *loadPaletteDialog;
    PropertiesDialog *propertiesDialog;
    QVector<BufferView *> bufferViews;
    BufferView *activeBufferView;
    Buffer *buffer;
    PenTip *penTip;
    PenTip *toolPenTip;
    PaletteMode paletteMode;
};

#endif // MAINWINDOW_H
