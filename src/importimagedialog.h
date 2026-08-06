#ifndef IMPORTIMAGEDIALOG_H
#define IMPORTIMAGEDIALOG_H

#include <QDialog>
#include "dithermode.h"
#include "palettesortmode.h"

namespace Ui {
class ImportImageDialog;
}

class ImportImageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportImageDialog(QWidget *parent = nullptr);
    ~ImportImageDialog();

    // Default width/height, shown pre-filled with the source image's own size.
    void setImageSize(int width, int height);

    // Colors/out-of values of the currently open image, used to populate
    // and grey out the fields when Palette is set to "Current".
    void setCurrentColorCount(int colors);

    int width() const;
    int height() const;
    bool useOptimalPalette() const;
    int colors() const;
    int outOf() const;
    DitherMode ditherMode() const;
    PaletteSortMode sortMode() const;

private slots:
    void setPaletteMode(int index);

private:
    Ui::ImportImageDialog *ui;
    int currentColorsIndex_ = 0;
};

#endif // IMPORTIMAGEDIALOG_H
