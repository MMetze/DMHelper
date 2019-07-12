#ifndef SELECTZOOM_H
#define SELECTZOOM_H

#include <QDialog>
#include <QImage>
#include <QRubberBand>

namespace Ui {
class SelectZoom;
}

class SelectZoom : public QDialog
{
    Q_OBJECT

public:
    explicit SelectZoom(QImage img, QWidget *parent = nullptr);
    ~SelectZoom();

    QImage getZoomImage();
    QRect getZoomRect();

protected:
    virtual void resizeEvent(QResizeEvent * event);
    virtual void showEvent(QShowEvent *event);
    virtual bool eventFilter(QObject *obj, QEvent *event);
    void setScaledImg();

private:
    Ui::SelectZoom *ui;
    QImage _img;
    QSize _scaledSize;
    QRubberBand* _rubberBand;
    QPoint _mouseDownPos;
};

#endif // SELECTZOOM_H
