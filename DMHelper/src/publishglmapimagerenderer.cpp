#include "publishglmapimagerenderer.h"
#include "battleglbackground.h"
#include "map.h"
#include <QOpenGLFunctions>

PublishGLMapImageRenderer::PublishGLMapImageRenderer(Map* map, QObject *parent) :
    PublishGLMapRenderer(map, parent),
    _backgroundObject(nullptr),
    _backgroundImage()
{
}

PublishGLMapImageRenderer::~PublishGLMapImageRenderer()
{
}

void PublishGLMapImageRenderer::cleanup()
{
    delete _backgroundObject;
    _backgroundObject = nullptr;

    PublishGLMapRenderer::cleanup();
}

void PublishGLMapImageRenderer::initializeBackground()
{
    if(_backgroundObject)
        return;

    if(_backgroundImage.isNull())
        _backgroundImage = _map->getBackgroundImage();

    _backgroundObject = new BattleGLBackground(nullptr, _backgroundImage, GL_NEAREST);
    updateProjectionMatrix();
}

void PublishGLMapImageRenderer::resizeBackground(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

    if(_backgroundObject)
        updateProjectionMatrix();
}

void PublishGLMapImageRenderer::paintBackground(QOpenGLFunctions* functions)
{
    if((!_backgroundObject) || (!functions))
        return;

    functions->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _backgroundObject->getMatrixData());
    _backgroundObject->paintGL();
}

QSizeF PublishGLMapImageRenderer::getBackgroundSize()
{
    return _backgroundObject ? _backgroundObject->getSize() : QSizeF();
}