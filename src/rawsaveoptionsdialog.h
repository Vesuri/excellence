#ifndef RAWSAVEOPTIONSDIALOG_H
#define RAWSAVEOPTIONSDIALOG_H

#include <QDialog>
#include "plugins/raw/rawplugin.h"

namespace Ui {
class RawSaveOptionsDialog;
}

struct RawSaveOptions {
    bool                    interleave       = true;
    bool                    wordAlign        = true;
    int                     paletteDepth     = 24;
    RawOption::Placement    palettePlacement = RawOption::PlacementEnd;
};

class RawSaveOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RawSaveOptionsDialog(QWidget *parent = nullptr);
    ~RawSaveOptionsDialog();

    RawSaveOptions options() const;

private:
    Ui::RawSaveOptionsDialog *ui;
};

#endif // RAWSAVEOPTIONSDIALOG_H
