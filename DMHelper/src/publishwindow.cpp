#include "publishwindow.h"
#include "publishglframe.h"
#include "publishglrenderer.h"
#include <QKeyEvent>
#include <QDebug>

PublishWindow::PublishWindow(const QString& title, QWidget *parent) :
    QMainWindow(parent),
    _publishFrame(nullptr),
    _globalColor(0, 0, 0, 255),
    _globalColorSet(false)
{
    setWindowTitle(title);
    // Not this: setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    //setAutoFillBackground(true);
    //setStyleSheet("background-color: rgba(0,0,0,255);");

    _publishFrame = new PublishGLFrame(this);
    setCentralWidget(_publishFrame);
    resize(800, 600);
    //_publishFrame->setAutoFillBackground(true);

    setBackgroundColor();

    connect(_publishFrame,SIGNAL(arrowVisibleChanged(bool)),this,SIGNAL(arrowVisibleChanged(bool)));
    connect(_publishFrame, SIGNAL(publishMouseDown(QPointF)),this,SIGNAL(publishMouseDown(QPointF)));
    connect(_publishFrame, SIGNAL(publishMouseMove(QPointF)),this,SIGNAL(publishMouseMove(QPointF)));
    connect(_publishFrame, SIGNAL(publishMouseRelease(QPointF)),this,SIGNAL(publishMouseRelease(QPointF)));
    connect(_publishFrame, SIGNAL(positionChanged(QPointF)),this,SIGNAL(positionChanged(QPointF)));
    connect(_publishFrame,SIGNAL(frameResized(QSize)),this,SIGNAL(frameResized(QSize)));
    connect(_publishFrame,SIGNAL(labelResized(QSize)),this,SIGNAL(labelResized(QSize)));
}

void PublishWindow::setImage(QImage img)
{
    _publishFrame->setImage(img);
}

void PublishWindow::setImage(QImage img, QColor color)
{
    setBackgroundColorStyle(color);
    setImage(img);
    _globalColorSet = false;
}

void PublishWindow::setImageNoScale(QImage img)
{
    setBackgroundColor();
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

void PublishWindow::setBackgroundColor()
{
    if(!_globalColorSet)
    {
        setBackgroundColorStyle(_globalColor);
        _globalColorSet = true;
    }
}

void PublishWindow::setBackgroundColor(QColor color)
{
    if(_globalColor != color)
    {
        _globalColor = color;
        _globalColorSet = false;
        setBackgroundColor();
    }
}

void PublishWindow::setPointerFile(const QString& filename)
{
    _publishFrame->setPointerFile(filename);
}

void PublishWindow::setRenderer(PublishGLRenderer* renderer)
{
    _publishFrame->setRenderer(renderer);
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

void PublishWindow::setBackgroundColorStyle(QColor color)
{
    if(!_publishFrame)
        return;

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
