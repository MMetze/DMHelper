#include "publishframe.h"
#include "dmconstants.h"
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTime>
#include <QDebug>

PublishFrame::PublishFrame(QWidget *parent) :
    QFrame(parent),
    _scrollArea(nullptr),
    _label(nullptr),
    _arrow(nullptr),
    _publishImg(),
    _isScaled(false),
    _arrowVisible(false),
    _arrowPosition(0, 0)
{
    _scrollArea = new QScrollArea(this);
    _scrollArea->setFrameShape(QFrame::NoFrame);
    _label = new QLabel(_scrollArea);

    // NOTE: New approach does not draw the arrow on the publish frame - this can be removed if the MapFrame is removed
    _arrow = new QLabel(_scrollArea);
    _arrow->setStyleSheet("background-image: url(); background-color: rgba(0,0,0,0);");
    setPointerFile(QString());
    //QImage arrowImg;
    //arrowImg.load(":/img/data/arrow.png");
    //QImage arrowScaled = arrowImg.scaled(DMHelper::CURSOR_SIZE,DMHelper::CURSOR_SIZE,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    //_arrow->resize(arrowScaled.size());
    //_arrow->setPixmap(QPixmap::fromImage(arrowScaled));
    _arrow->hide();

    _scrollArea->setAttribute(Qt::WA_TransparentForMouseEvents);
    _scrollArea->resize(size());

    _label->setStyleSheet("background-color: rgba(255,0,0,0);");

    setMouseTracking(true);
}

PublishFrame::~PublishFrame()
{
}

QSize PublishFrame::sizeHint() const
{
    return QSize(800,600);
}

void PublishFrame::setImage(QImage img)
{
    _publishImg = img;
    _isScaled = true;
    setScaledImg();
}

void PublishFrame::setImageNoScale(QImage img)
{
    _isScaled = false;
    _label->setPixmap(QPixmap::fromImage(img));
    _label->resize(img.size());
    _label->move(width()/2 - img.width()/2,height()/2 - img.height()/2);
}

void PublishFrame::setArrowVisible(bool visible)
{
    if(visible)
        _arrow->show();
    else
        _arrow->hide();

    update();
}

void PublishFrame::setArrowPosition(const QPointF& position)
{
    QPointF newPosition;

    newPosition.setX( ( position.x() * _label->width() ) + _label->x() );
    newPosition.setY( ( position.y() * _label->height() ) + _label->y() );

    _arrow->move(newPosition.toPoint());

    update();
}

void PublishFrame::setPointerFile(const QString& filename)
{
    QPixmap pointerImage;
    if((filename.isEmpty()) ||
       (!pointerImage.load(filename)))
    {
        pointerImage = QPixmap(":/img/data/arrow.png");
    }

    QPixmap scaledPointer = pointerImage.scaled(DMHelper::CURSOR_SIZE,DMHelper::CURSOR_SIZE,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    _arrow->resize(scaledPointer.size());
    _arrow->setPixmap(scaledPointer);
}

void PublishFrame::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event);
    _scrollArea->resize(size());
    if(_isScaled)
        setScaledImg();

    emit frameResized(size());
}

void PublishFrame::mouseMoveEvent(QMouseEvent * event)
{
    _arrowPosition.setX( ( event->localPos().x() - _label->x() ) / _label->width() );
    _arrowPosition.setY( ( event->localPos().y() - _label->y() ) / _label->height() );

    if( _arrowVisible )
        emit positionChanged(_arrowPosition);

    QWidget::mouseMoveEvent(event);
}

void PublishFrame::keyPressEvent(QKeyEvent * event)
{
    if( event->key() == Qt::Key_Space )
    {
        _arrowVisible = !_arrowVisible;
        emit arrowVisibleChanged(_arrowVisible);
        if(_arrowVisible)
            emit positionChanged(_arrowPosition);
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void PublishFrame::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    QWidget::focusInEvent(event);
}

void PublishFrame::setScaledImg()
{
    if((!_publishImg.isNull())&&(_label))
    {
        QImage scaledImg = _publishImg.scaled(size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
        _label->setPixmap(QPixmap::fromImage(scaledImg));
        _label->resize(scaledImg.size());
        _label->move(width()/2 - scaledImg.width()/2,height()/2 - scaledImg.height()/2);
    }
}
