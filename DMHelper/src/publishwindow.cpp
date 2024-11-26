#include "publishwindow.h"
#include "publishglframe.h"
#include "publishglrenderer.h"
#include "publishglimagerenderer.h"
#include <QKeyEvent>
#include <QUuid>
#include <QDebug>

PublishWindow::PublishWindow(const QString& title, QWidget *parent) :
    QMainWindow(parent),
    _publishFrame(nullptr),
    _globalColor(0, 0, 0, 255),
    _globalColorSet(false)
{
    setWindowTitle(title);
    // Not this: setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    _publishFrame = new PublishGLFrame();
    setCentralWidget(_publishFrame);

    setBackgroundColor();

    connect(_publishFrame, SIGNAL(publishMouseDown(QPointF)), this, SIGNAL(publishMouseDown(QPointF)));
    connect(_publishFrame, SIGNAL(publishMouseMove(QPointF)), this, SIGNAL(publishMouseMove(QPointF)));
    connect(_publishFrame, SIGNAL(publishMouseRelease(QPointF)), this, SIGNAL(publishMouseRelease(QPointF)));
    connect(_publishFrame, SIGNAL(frameResized(QSize)), this, SIGNAL(frameResized(QSize)));
    connect(_publishFrame, SIGNAL(labelResized(QSize)), this, SIGNAL(labelResized(QSize)));
}

QUuid PublishWindow::getObjectId() const
{
    return ((_publishFrame) && (_publishFrame->getRenderer())) ? _publishFrame->getRenderer()->getObjectId() : QUuid();
}

PublishGLRenderer* PublishWindow::getRenderer() const
{
    return _publishFrame ? _publishFrame->getRenderer() : nullptr;
}

void PublishWindow::setImage(QImage img)
{
    PublishGLImageRenderer* newRenderer = new PublishGLImageRenderer(nullptr, img, _globalColor);
    _publishFrame->setRenderer(newRenderer);
}

void PublishWindow::setImage(QImage img, const QColor& color)
{
    setBackgroundColorStyle(color);

    PublishGLImageRenderer* newRenderer = new PublishGLImageRenderer(nullptr, img, color);
    _publishFrame->setRenderer(newRenderer);

    _globalColorSet = false;
}

void PublishWindow::setBackgroundColor()
{
    if(!_globalColorSet)
    {
        setBackgroundColorStyle(_globalColor);
        _globalColorSet = true;
    }
}

void PublishWindow::setBackgroundColor(const QColor& color)
{
    if(_globalColor != color)
    {
        _globalColor = color;
        _globalColorSet = false;
        setBackgroundColor();
    }
}

void PublishWindow::setRenderer(PublishGLRenderer* renderer)
{
    if(renderer)
        renderer->setBackgroundColor(_globalColor);

    _publishFrame->setRenderer(renderer);
}

void PublishWindow::keyPressEvent(QKeyEvent * event)
{
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

void PublishWindow::setBackgroundColorStyle(const QColor& color)
{
    // Make sure the OpenGL background is also set properly
    _publishFrame->setBackgroundColor(color);

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
