#include "publishglframe.h"
#include "publishglrenderer.h"
#include "publishglimagerenderer.h"
#include <QOpenGLFunctions>
#include <QTimer>
#include <QDebug>

PublishGLFrame::PublishGLFrame(QWidget *parent) :
    QOpenGLWidget(parent),
    _initialized(false),
    _targetSize(),
    _renderer(nullptr)
{
}

PublishGLFrame::~PublishGLFrame()
{
    cleanup();
    setRenderer(nullptr);
}

bool PublishGLFrame::isInitialized() const
{
    return _initialized;
}

PublishGLRenderer* PublishGLFrame::getRenderer() const
{
    return _renderer;
}

void PublishGLFrame::cleanup()
{
    qDebug() << "[PublishGLFrame] Cleaning up the publish frame...";
    _initialized = false;

    if(_renderer)
        _renderer->cleanup();
}

void PublishGLFrame::updateWidget()
{
    update();
}

void PublishGLFrame::setRenderer(PublishGLRenderer* renderer)
{
    if(renderer == _renderer)
    {
        updateWidget();
        return;
    }

    if(_renderer)
    {
        if(!renderer)
            renderer = new PublishGLImageRenderer(nullptr, grabFramebuffer(), _renderer->getBackgroundColor());

        _renderer->rendererDeactivated();
        disconnect(_renderer, &PublishGLRenderer::updateWidget, this, &PublishGLFrame::updateWidget);
        disconnect(_renderer, &PublishGLRenderer::destroyed, this, &PublishGLFrame::clearRenderer);
        if(_renderer->deleteOnDeactivation())
            _renderer->deleteLater();
    }

    _renderer = renderer;
    if(_renderer)
    {
        connect(_renderer, &PublishGLRenderer::updateWidget, this, &PublishGLFrame::updateWidget);
        connect(_renderer, &PublishGLRenderer::destroyed, this, &PublishGLFrame::clearRenderer);
        _renderer->rendererActivated(this);
        if(isInitialized())
        {
            makeCurrent();
            _renderer->initializeGL();
            _renderer->resizeGL(_targetSize.width(), _targetSize.height());
            doneCurrent();
            updateWidget();
        }
    }
}

void PublishGLFrame::clearRenderer()
{
    _renderer = nullptr;
}

void PublishGLFrame::setBackgroundColor(const QColor& color)
{
    if(_renderer)
        _renderer->setBackgroundColor(color);
}

void PublishGLFrame::setArrowVisible(bool visible)
{

}

void PublishGLFrame::setArrowPosition(const QPointF& position)
{

}

void PublishGLFrame::setPointerFile(const QString& filename)
{

}

void PublishGLFrame::initializeGL()
{
    if(!QOpenGLContext::currentContext())
        return;

    connect(QOpenGLContext::currentContext(), &QOpenGLContext::aboutToBeDestroyed, this, &PublishGLFrame::cleanup);

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(f)
    {
        f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
        f->glEnable(GL_TEXTURE_2D); // Enable texturing
        f->glEnable(GL_BLEND);// you enable blending function
        f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    QTimer::singleShot(0, this, &PublishGLFrame::initializeRenderer);

    _initialized = true;

}

void PublishGLFrame::resizeGL(int w, int h)
{
    if(_targetSize == QSize(w, h))
        return;

    qDebug() << "[PublishGLFrame] Resize w: " << w << ", h: " << h;
    _targetSize = QSize(w, h);

    if(_renderer)
        _renderer->resizeGL(w, h);

    emit frameResized(_targetSize);
}

void PublishGLFrame::paintGL()
{
    if(_renderer)
        _renderer->paintGL();
}

void PublishGLFrame::initializeRenderer()
{
    if(_renderer)
    {
        makeCurrent();
        _renderer->initializeGL();
        _renderer->resizeGL(_targetSize.width(), _targetSize.height());
        doneCurrent();
        updateWidget();
    }
}
