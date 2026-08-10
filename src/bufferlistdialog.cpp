#include <QAbstractButton>
#include <QGridLayout>
#include <QPushButton>
#include "bufferlistdialog.h"
#include "bufferthumbnailbutton.h"
#include "mainwindow.h"
#include "ui_bufferlistdialog.h"

BufferListDialog::BufferListDialog(MainWindow *mainWindow, QWidget *parent) : QDialog(parent),
    ui(new Ui::BufferListDialog),
    mainWindow_(mainWindow)
{
    ui->setupUi(this);
    connect(mainWindow_, &MainWindow::bufferListChanged, this, &BufferListDialog::refresh);
    connect(ui->buttonAdd, &QPushButton::clicked, mainWindow_, &MainWindow::addBuffer);
    connect(ui->buttonDelete, &QPushButton::clicked, this, [this]() {
        mainWindow_->deleteBufferAt(mainWindow_->activeBufferIndex());
    });
}

BufferListDialog::~BufferListDialog()
{
    delete ui;
}

void BufferListDialog::refresh()
{
    while (ui->thumbnailsLayout->count() > 0) {
        QLayoutItem *item = ui->thumbnailsLayout->takeAt(0);
        delete item->widget();
        delete item;
    }

    int count = mainWindow_->bufferCount();
    int active = mainWindow_->activeBufferIndex();
    int spare = mainWindow_->spareBufferIndex();
    int work = mainWindow_->workBufferIndex();
    static const int perRow = 4;
    for (int i = 0, row = 0, col = 0; i < count; i++) {
        BufferThumbnailButton *thumb = new BufferThumbnailButton(mainWindow_->bufferAt(i));
        thumb->setActive(i == active);
        thumb->setSpare(i == spare);
        thumb->setWork(i == work);
        connect(thumb, &QAbstractButton::clicked, this, [this, i]() { mainWindow_->setActiveBufferIndex(i); });
        ui->thumbnailsLayout->addWidget(thumb, row, col);
        if (++col >= perRow) { col = 0; row++; }
    }
    ui->buttonDelete->setEnabled(count > 1);
}
