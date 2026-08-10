#ifndef BUFFERTHUMBNAILBUTTON_H
#define BUFFERTHUMBNAILBUTTON_H

#include <QAbstractButton>

class Buffer;

class BufferThumbnailButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit BufferThumbnailButton(Buffer *buffer, QWidget *parent = nullptr);

    void setActive(bool active);
    void setSpare(bool spare);
    void setWork(bool work);

protected:
    void paintEvent(QPaintEvent *e) override;

private slots:
    void updateLabel();

private:
    Buffer *buffer_;
    bool active_ = false;
    bool spare_ = false;
    bool work_ = false;
};

#endif // BUFFERTHUMBNAILBUTTON_H
