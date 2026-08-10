#include <QFileInfo>
#include <QPainter>
#include <QPaintEvent>
#include "buffer.h"
#include "bufferthumbnailbutton.h"

BufferThumbnailButton::BufferThumbnailButton(Buffer *buffer, QWidget *parent) : QAbstractButton(parent),
    buffer_(buffer)
{
    setMinimumSize(96, 96);
    setCheckable(false);
    connect(buffer_, &Buffer::modified, this, [this](QRect) { update(); });
    connect(buffer_, &Buffer::dirtyChanged, this, [this](bool) { updateLabel(); });
    connect(buffer_, &Buffer::pathChanged, this, [this](QString) { updateLabel(); });
}

void BufferThumbnailButton::setActive(bool active)
{
    if (active_ == active) return;
    active_ = active;
    update();
}

void BufferThumbnailButton::setSpare(bool spare)
{
    if (spare_ == spare) return;
    spare_ = spare;
    update();
}

void BufferThumbnailButton::setWork(bool work)
{
    if (work_ == work) return;
    work_ = work;
    update();
}

void BufferThumbnailButton::updateLabel()
{
    update();
}

void BufferThumbnailButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    const QRect r = rect();

    painter.fillRect(r, Qt::black);

    QRect imageRect = r.adjusted(4, 4, -4, -20);
    if (imageRect.isValid()) {
        QPixmap pm = QPixmap::fromImage(buffer_->image()).scaled(
            imageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect target(0, 0, pm.width(), pm.height());
        target.moveCenter(imageRect.center());
        painter.drawPixmap(target, pm);
    }

    QString name = buffer_->path().isEmpty() ? tr("Untitled") : QFileInfo(buffer_->path()).fileName();
    if (buffer_->isDirty())
        name += "*";

    painter.setPen(Qt::white);
    QFont f = painter.font();
    f.setPixelSize(10);
    painter.setFont(f);
    painter.drawText(r.adjusted(2, 0, -2, -4), Qt::AlignBottom | Qt::AlignHCenter, name);

    if (spare_) {
        painter.setPen(Qt::yellow);
        QFont bf = painter.font();
        bf.setPixelSize(9);
        bf.setBold(true);
        painter.setFont(bf);
        painter.drawText(r.adjusted(4, 4, -4, -4), Qt::AlignTop | Qt::AlignLeft, tr("SPARE"));
    }

    if (work_) {
        painter.setPen(Qt::cyan);
        QFont bf = painter.font();
        bf.setPixelSize(9);
        bf.setBold(true);
        painter.setFont(bf);
        painter.drawText(r.adjusted(4, 4, -4, -4), Qt::AlignTop | Qt::AlignRight, tr("WORK"));
    }

    if (active_) {
        painter.setPen(QPen(QColor(0x33, 0x75, 0xe6), 2));
        painter.drawRect(r.adjusted(1, 1, -2, -2));
    }
}
