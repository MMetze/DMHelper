#include "publishwindow.h"
#include "publishframe.h"
#include <QKeyEvent>
#include <QDebug>

PublishWindow::PublishWindow(const QString& title, QWidget *parent) :
    QMainWindow(parent),
    _publishFrame(nullptr)
{
    setWindowTitle(title);
    // Not this: setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    //setAutoFillBackground(true);
    setStyleSheet("background-color: rgba(0,0,0,255);");

    _publishFrame = new PublishFrame(this);
    setCentralWidget(_publishFrame);
    resize(600, 400);
    //_publishFrame->setAutoFillBackground(true);

    connect(_publishFrame,SIGNAL(arrowVisibleChanged(bool)),this,SIGNAL(arrowVisibleChanged(bool)));
    connect(_publishFrame, SIGNAL(positionChanged(QPointF)),this,SIGNAL(positionChanged(QPointF)));
    connect(_publishFrame,SIGNAL(frameResized(QSize)),this,SIGNAL(frameResized(QSize)));
}

void PublishWindow::setImage(QImage img, QColor color)
{
    setBackgroundColor(color);
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

void PublishWindow::setBackgroundColor(QColor color)
{
    QString styleString("background-color: rgba(");
    styleString += QString::number(color.red());
    styleString += QString(",");
    styleString += QString::number(color.green());
    styleString += QString(",");
    styleString += QString::number(color.blue());
    styleString += QString(",255);");

    qDebug() << "[PublishWindow] changing background color to: " << color << ", string: " << styleString;

    setStyleSheet(styleString);
    _publishFrame->setStyleSheet(styleString);
}

void PublishWindow::setPointerFile(const QString& filename)
{
    _publishFrame->setPointerFile(filename);
}

void PublishWindow::keyPressEvent(QKeyEvent * event)
{
    /*
    if(event->key() == Qt::Key_Escape)
    {
        hide();
    }
    */
    if(event->key() == Qt::Key_Escape)
    {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    }
    else if(event->key() == Qt::Key_F)
    {
        setWindowState(windowState() ^ Qt::WindowFullScreen);
    }
    else
    {
        QMainWindow::keyPressEvent(event);
    }
}

void PublishWindow::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    emit windowVisible(true);
}

void PublishWindow::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    emit windowVisible(false);
}

