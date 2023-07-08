#include "publishgltokenhighlighteffect.h"
#include "publishglimage.h"
#include <QOpenGLFunctions>

PublishGLTokenHighlightEffect::PublishGLTokenHighlightEffect(PublishGLImage* effectImage, BattleDialogModelEffect* effect, QObject *parent) :
    PublishGLTokenHighlight{parent},
    _effectImage(effectImage),
    _effect(effect)
{
}

PublishGLTokenHighlightEffect::~PublishGLTokenHighlightEffect()
{
    delete _effectImage;
}

void PublishGLTokenHighlightEffect::paintGL(QOpenGLFunctions *f, int shaderModelMatrix)
{
    if((!f) || (!_effectImage))
        return;

    f->glUniformMatrix4fv(shaderModelMatrix, 1, GL_FALSE, getMatrixData());
    _effectImage->paintGL();
}


int PublishGLTokenHighlightEffect::getWidth() const
{
    return _effectImage ? _effectImage->getImageSize().width() : 0;
}

int PublishGLTokenHighlightEffect::getHeight() const
{
    return _effectImage ? _effectImage->getImageSize().height() : 0;
}

PublishGLImage* PublishGLTokenHighlightEffect::getImage() const
{
    return _effectImage;
}

BattleDialogModelEffect* PublishGLTokenHighlightEffect::getEffect() const
{
    return _effect;
}
