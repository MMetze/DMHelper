#include "publishglbattleimagerenderer.h"
#include "battleglbackground.h"
#include "battledialogmodel.h"
#include <QOpenGLFunctions>

PublishGLBattleImageRenderer::PublishGLBattleImageRenderer(BattleDialogModel* model, QObject *parent) :
    PublishGLBattleRenderer(model, parent),
    _backgroundObject(nullptr),
    _backgroundImage()
{
}

void PublishGLBattleImageRenderer::cleanup()
{
    delete _backgroundObject;
    _backgroundObject = nullptr;

    PublishGLBattleRenderer::cleanup();
}

void PublishGLBattleImageRenderer::initializeBackground()
{
    if((_backgroundObject) || (!_model))
        return;

    if(_backgroundImage.isNull())
        _backgroundImage = _model->getBackgroundImage();

    _scene.deriveSceneRectFromSize(_model->getBackgroundImage().size());
    _backgroundObject = new BattleGLBackground(&_scene, _backgroundImage, GL_NEAREST);
    updateProjectionMatrix();
}

void PublishGLBattleImageRenderer::resizeBackground(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

    if(_backgroundObject)
        updateProjectionMatrix();
}

void PublishGLBattleImageRenderer::paintBackground(QOpenGLFunctions* functions)
{
    if((!_backgroundObject) || (!functions))
        return;

    functions->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _backgroundObject->getMatrixData());
    _backgroundObject->paintGL();
}

QSizeF PublishGLBattleImageRenderer::getBackgroundSize()
{
    return _backgroundObject ? _backgroundObject->getSize() : QSizeF();
}
