#ifndef TEXTTOOL_H
#define TEXTTOOL_H

#include <QDialog>
#include <QFont>
#include "tool.h"

class QLineEdit;
class QLabel;

class TextDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextDialog(QWidget *parent = nullptr);

    QString text() const;
    QFont font() const;

private slots:
    void chooseFont();
    void updatePreview();

private:
    QLineEdit *textEdit_;
    QLabel *fontLabel_;
    QLabel *preview_;
    QFont font_;
};

class TextTool : public Tool
{
    Q_OBJECT

public:
    explicit TextTool(QObject *parent = nullptr);

    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;

private:
    static TextTool instance;
};

#endif // TEXTTOOL_H
