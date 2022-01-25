#include "publishgltextimagerenderer.h"
#include "publishglbattlebackground.h"
#include <QOpenGLFunctions>

PublishGLTextImageRenderer::PublishGLTextImageRenderer(EncounterText* encounter, QImage backgroundImage, QImage textImage, QObject *parent) :
    PublishGLTextRenderer(encounter, /*backgroundImage,*/ textImage, parent),
    _backgroundObject(nullptr),
    _backgroundImage(backgroundImage)
{
}

void PublishGLTextImageRenderer::cleanup()
{
    delete _backgroundObject;
    _backgroundObject = nullptr;

    PublishGLTextRenderer::cleanup();
}

void PublishGLTextImageRenderer::initializeBackground()
{
    if((_backgroundObject) || (!_encounter))
        return;

//    if(_backgroundImage.isNull())
//        _backgroundImage = _map->getBackgroundImage();

    _backgroundObject = new PublishGLBattleBackground(nullptr, _backgroundImage, GL_NEAREST);
    updateProjectionMatrix();
}

bool PublishGLTextImageRenderer::isBackgroundReady()
{
    return _backgroundObject != nullptr;
}

void PublishGLTextImageRenderer::resizeBackground(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

    if(_backgroundObject)
        updateProjectionMatrix();
}

void PublishGLTextImageRenderer::paintBackground(QOpenGLFunctions* functions)
{
    if((!_backgroundObject) || (!functions))
        return;

    functions->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _backgroundObject->getMatrixData());
    _backgroundObject->paintGL();
}

QSizeF PublishGLTextImageRenderer::getBackgroundSize()
{
    return _backgroundObject ? _backgroundObject->getSize() : QSizeF();
}
