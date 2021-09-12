#ifndef TEXTPUBLISHDIALOG_H
#define TEXTPUBLISHDIALOG_H

#include <QDialog>

namespace Ui {
class TextPublishDialog;
}

class TextPublishDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextPublishDialog(QWidget *parent = nullptr);
    ~TextPublishDialog();

public slots:
    void publishTextImage();
    void setPreviewImage(QImage img, const QColor& color);

signals:
    void publishImage(QImage img, const QColor& color);
    void openPreview();

protected:
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);
    virtual void hideEvent(QHideEvent * event);

private:
    Ui::TextPublishDialog *ui;
    bool mouseDown;
    QPoint mouseDownPos;
};

#endif // TEXTPUBLISHDIALOG_H
