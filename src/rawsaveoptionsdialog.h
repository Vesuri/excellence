#ifndef RAWSAVEOPTIONSDIALOG_H
#define RAWSAVEOPTIONSDIALOG_H

#include <QDialog>

class QCheckBox;
class QRadioButton;
class QComboBox;

struct RawSaveOptions {
    bool interleave       = true;
    bool wordAlign        = true;
    int  paletteDepth     = 24;
    int  palettePlacement = 2; // RawOption::PlacementEnd
};

class RawSaveOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RawSaveOptionsDialog(QWidget *parent = nullptr);

    RawSaveOptions options() const;

private:
    QCheckBox    *interleaveCheck_;
    QCheckBox    *wordAlignCheck_;
    QRadioButton *depth12_;
    QRadioButton *depth24_;
    QComboBox    *placementCombo_;
};

#endif // RAWSAVEOPTIONSDIALOG_H
