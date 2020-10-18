#include <QtMath>
#include "buffer.h"
#include "propertiesdialog.h"
#include "ui_propertiesdialog.h"

PropertiesDialog::PropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertiesDialog),
    buffer(nullptr)
{
    ui->setupUi(this);

    connect(this, SIGNAL(accepted()), this, SLOT(setProperties()));
}

PropertiesDialog::~PropertiesDialog()
{
    delete ui;
}

void PropertiesDialog::setBuffer(Buffer *buffer)
{
    this->buffer = buffer;

    ui->spinBoxWidth->setValue(buffer->image().width());
    ui->spinBoxHeight->setValue(buffer->image().height());

    int colorsIndex = 0;
    for (int i = buffer->image().colorCount(); i > 2; i >>= 1) {
        colorsIndex++;
    }
    ui->comboBoxColors->setCurrentIndex(colorsIndex);
}

void PropertiesDialog::setProperties()
{
    int width = ui->spinBoxWidth->value();
    int height = ui->spinBoxHeight->value();
    int colors = qPow(2, ui->comboBoxColors->currentIndex() + 1);
    Buffer *newBuffer = new Buffer(width, height, colors, buffer->parent());
    emit bufferChanged(newBuffer);
}
