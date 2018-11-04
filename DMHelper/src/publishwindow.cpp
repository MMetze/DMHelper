#include "publishwindow.h"
#include "publishframe.h"
#include <QKeyEvent>

PublishWindow::PublishWindow(const QString& title, QWidget *parent) :
    QMainWindow(parent),
    _publishFrame(0)
{
    setWindowTitle(title);

    _publishFrame = new PublishFrame(this);
    setCentralWidget(_publishFrame);
    resize(600, 400);

    connect(_publishFrame,SIGNAL(visibleChanged(bool)),this,SIGNAL(visibleChanged(bool)));
    connect(_publishFrame, SIGNAL(positionChanged(QPointF)),this,SIGNAL(positionChanged(QPointF)));
    connect(_publishFrame,SIGNAL(frameResized(QSize)),this,SIGNAL(frameResized(QSize)));
}

void PublishWindow::setImage(QImage img)
{
    _publishFrame->setImage(img);
}

void PublishWindow::setImageNoScale(QImage img)
{
    _publishFrame->setImageNoScale(img);
}

void PublishWindow::setArrowVisible(bool visible)
{
    _publishFrame->setArrowVisible(visible);
}

void PublishWindow::setArrowPosition(const QPointF& position)
{
    _publishFrame->setArrowPosition(position);
}

void PublishWindow::keyPressEvent(QKeyEvent * event)
{
    if( event->key() == Qt::Key_Escape )
    {
        hide();
    }
    else
    {
        QMainWindow::keyPressEvent(event);
    }
}

