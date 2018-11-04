#include "selectzoom.h"
#include "ui_selectzoom.h"
#include <QMouseEvent>
#include <QPainter>

SelectZoom::SelectZoom(QImage img, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectZoom),
    _img(img),
    _scaledSize(),
    _rubberBand(0),
    _mouseDownPos()
{
    ui->setupUi(this);
    ui->scrollAreaWidgetContents->installEventFilter(this);
}

SelectZoom::~SelectZoom()
{
    delete ui;
}

QImage SelectZoom::getZoomImage()
{
    if(!_rubberBand)
        return _img;

    return _img.copy(getZoomRect());
}

QRect SelectZoom::getZoomRect()
{
    if(!_rubberBand)
        return _img.rect();

    QRect selection = ui->lblImage->geometry().intersected(_rubberBand->geometry());
    selection.translate(-ui->lblImage->x(),-ui->lblImage->y());
    QRect imgSelection(selection.x() * _img.width() / ui->lblImage->width(),
                       selection.y() * _img.height() / ui->lblImage->height(),
                       selection.width() * _img.width() / ui->lblImage->width(),
                       selection.height() * _img.height() / ui->lblImage->height() );

    int imgLeft = ((ui->lblImage->width() - _scaledSize.width()) / 2) + ui->lblImage->x();
    int imgTop = ((ui->lblImage->height() - _scaledSize.height()) / 2) + ui->lblImage->y();
    imgSelection.setLeft((_rubberBand->x() - imgLeft) * _img.width() / _scaledSize.width());
    imgSelection.setTop((_rubberBand->y() - imgTop) * _img.height() / _scaledSize.height());
    imgSelection.setWidth(_rubberBand->width() * _img.width() / _scaledSize.width());
    imgSelection.setHeight(_rubberBand->height() * _img.height() / _scaledSize.height());

    return imgSelection;
}

void SelectZoom::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event);
    setScaledImg();
}

void SelectZoom::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    setScaledImg();
}

bool SelectZoom::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        _mouseDownPos = mouseEvent->pos();
        if(!_rubberBand)
        {
            _rubberBand = new QRubberBand(QRubberBand::Rectangle, ui->scrollAreaWidgetContents);
        }
        _rubberBand->setGeometry(QRect(_mouseDownPos, QSize()));
        _rubberBand->show();
    }
    else if(event->type() == QEvent::MouseMove)
    {
        if(_rubberBand)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            _rubberBand->setGeometry(QRect(_mouseDownPos, mouseEvent->pos()).normalized());
        }
    }

    return QObject::eventFilter(obj,event);
}

void SelectZoom::setScaledImg()
{
    if(!_img.isNull())
    {
        QSize targetSize = ui->scrollArea->viewport()->size();
        QImage scaledImg = _img.scaled(targetSize,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        _scaledSize = scaledImg.size();
        ui->lblImage->setPixmap(QPixmap::fromImage(scaledImg));
    }
}
