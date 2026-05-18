#include "plugins/raw/rawplugin.h"
#include "rawsaveoptionsdialog.h"
#include "ui_rawsaveoptionsdialog.h"

RawSaveOptionsDialog::RawSaveOptionsDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::RawSaveOptionsDialog)
{
    ui->setupUi(this);
}

RawSaveOptionsDialog::~RawSaveOptionsDialog()
{
    delete ui;
}

RawSaveOptions RawSaveOptionsDialog::options() const
{
    RawSaveOptions opts;
    opts.interleave       = ui->checkBoxInterleave->isChecked();
    opts.wordAlign        = ui->checkBoxWordAlign->isChecked();
    opts.paletteDepth     = ui->radioButton24bit->isChecked() ? 24 : 12;
    opts.palettePlacement = ui->comboBoxPlacement->currentIndex();
    return opts;
}
