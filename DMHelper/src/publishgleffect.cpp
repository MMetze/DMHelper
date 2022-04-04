#include "publishgleffect.h"
#include "publishglimage.h"

PublishGLEffect::PublishGLEffect(PublishGLImage& effectImage, QObject *parent) :
    QObject(parent),
    _effectImage(effectImage),
    _modelMatrix()
{
}

PublishGLEffect::~PublishGLEffect()
{
}

void PublishGLEffect::setPositionScale(const QVector3D& pos, float sizeFactor)
{
    if(_effectImage.getImageSize().isNull())
        return;

    qreal scaleFactor = sizeFactor / qMax(_effectImage.getImageSize().width(), _effectImage.getImageSize().height());

    _modelMatrix.setToIdentity();
    _modelMatrix.translate(pos);
    _modelMatrix.scale(scaleFactor, scaleFactor);
}

PublishGLImage& PublishGLEffect::getImage() const
{
    return _effectImage;
}

QMatrix4x4 PublishGLEffect::getMatrix() const
{
    return _modelMatrix;
}

const float* PublishGLEffect::getMatrixData() const
{
    return _modelMatrix.constData();
}
