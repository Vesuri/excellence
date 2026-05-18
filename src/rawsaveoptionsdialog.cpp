#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include "plugins/raw/rawplugin.h"
#include "rawsaveoptionsdialog.h"

RawSaveOptionsDialog::RawSaveOptionsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Raw Save Options"));

    // Bitplane options
    interleaveCheck_ = new QCheckBox(tr("Interleave bitplanes"));
    interleaveCheck_->setChecked(true);
    wordAlignCheck_ = new QCheckBox(tr("Word-align rows (pad to 16-bit boundary)"));
    wordAlignCheck_->setChecked(true);

    QGroupBox *bitplanesGroup = new QGroupBox(tr("Bitplanes"));
    QVBoxLayout *bitplanesLayout = new QVBoxLayout(bitplanesGroup);
    bitplanesLayout->setContentsMargins(6, 4, 6, 6);
    bitplanesLayout->setSpacing(6);
    bitplanesLayout->addWidget(interleaveCheck_);
    bitplanesLayout->addWidget(wordAlignCheck_);

    // Palette depth
    depth12_ = new QRadioButton(tr("12-bit (4 bits per channel)"));
    depth24_ = new QRadioButton(tr("24-bit (8 bits per channel)"));
    depth24_->setChecked(true);

    QGroupBox *depthGroup = new QGroupBox(tr("Palette depth"));
    QVBoxLayout *depthLayout = new QVBoxLayout(depthGroup);
    depthLayout->setContentsMargins(6, 4, 6, 6);
    depthLayout->setSpacing(6);
    depthLayout->addWidget(depth12_);
    depthLayout->addWidget(depth24_);

    // Palette placement
    placementCombo_ = new QComboBox;
    placementCombo_->addItem(tr("None (omit palette)"),  RawOption::PlacementNone);
    placementCombo_->addItem(tr("Start (before data)"),  RawOption::PlacementStart);
    placementCombo_->addItem(tr("End (after data)"),     RawOption::PlacementEnd);
    placementCombo_->setCurrentIndex(2);

    QGroupBox *placementGroup = new QGroupBox(tr("Palette placement"));
    QFormLayout *placementLayout = new QFormLayout(placementGroup);
    placementLayout->setContentsMargins(6, 4, 6, 6);
    placementLayout->setSpacing(4);
    placementLayout->addRow(tr("Position:"), placementCombo_);

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setSpacing(8);
    main->setContentsMargins(8, 8, 8, 8);
    main->addWidget(bitplanesGroup);
    main->addWidget(depthGroup);
    main->addWidget(placementGroup);
    main->addWidget(buttons);
}

RawSaveOptions RawSaveOptionsDialog::options() const
{
    RawSaveOptions opts;
    opts.interleave       = interleaveCheck_->isChecked();
    opts.wordAlign        = wordAlignCheck_->isChecked();
    opts.paletteDepth     = depth24_->isChecked() ? 24 : 12;
    opts.palettePlacement = placementCombo_->currentData().toInt();
    return opts;
}
