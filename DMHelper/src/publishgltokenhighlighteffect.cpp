#include "publishgltokenhighlighteffect.h"
#include "publishglimage.h"
#include "battledialogmodeleffect.h"
#include "scaledpixmap.h"
#include "dmh_opengl.h"
#include <QPainter>

PublishGLTokenHighlightEffect::PublishGLTokenHighlightEffect(PublishGLImage* effectImage, BattleDialogModelEffect* effect, QObject *parent) :
    PublishGLTokenHighlight{parent},
    _effectImage(effectImage),
    _effect(effect),
    _position(),
    _sizeFactor()
{
}

PublishGLTokenHighlightEffect::~PublishGLTokenHighlightEffect()
{
    delete _effectImage;
}

void PublishGLTokenHighlightEffect::paintGL(QOpenGLFunctions *f, int shaderModelMatrix)
{
    if(!f)
        return;

    if(!_effectImage)
    {
        createEffectHighlightGL();
        if(!_effectImage)
            return;
    }

    DMH_DEBUG_OPENGL_glUniformMatrix4fv(shaderModelMatrix, 1, GL_FALSE, getMatrixData(), getMatrix());
    f->glUniformMatrix4fv(shaderModelMatrix, 1, GL_FALSE, getMatrixData());
    _effectImage->paintGL(f, nullptr);
}


int PublishGLTokenHighlightEffect::getWidth() const
{
    return _effectImage ? _effectImage->getImageSize().width() : 0;
}

int PublishGLTokenHighlightEffect::getHeight() const
{
    return _effectImage ? _effectImage->getImageSize().height() : 0;
}

void PublishGLTokenHighlightEffect::setPositionScale(const QVector3D& pos, float sizeFactor)
{
    _position = pos;
    _sizeFactor = sizeFactor;

    if(_effectImage)
        PublishGLTokenHighlight::setPositionScale(_position, _sizeFactor);
}

PublishGLImage* PublishGLTokenHighlightEffect::getImage() const
{
    return _effectImage;
}

BattleDialogModelEffect* PublishGLTokenHighlightEffect::getEffect() const
{
    return _effect;
}

void PublishGLTokenHighlightEffect::createEffectHighlightGL()
{
    if((!_effect) || (_effectImage))
        return;

    QColor ellipseColor = _effect->getColor();
    if(!ellipseColor.isValid())
        return;

    QImage ellipseImage(DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0], DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][1], QImage::Format_RGBA8888);
    ellipseImage.fill(Qt::transparent);
    QPainter p;
    p.begin(&ellipseImage);
        p.setPen(QPen(ellipseColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        ellipseColor.setAlpha(128);
        p.setBrush(QBrush(ellipseColor));
        p.drawEllipse(0, 0, ellipseImage.width(), ellipseImage.height());
    p.end();

    _effectImage = new PublishGLImage(ellipseImage);
    PublishGLTokenHighlight::setPositionScale(_position, _sizeFactor);
}
