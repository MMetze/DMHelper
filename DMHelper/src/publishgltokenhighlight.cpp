#include "publishgltokenhighlight.h"
#include "publishglimage.h"

PublishGLTokenHighlight::PublishGLTokenHighlight(QObject *parent) :
    QObject(parent),
    _modelMatrix()
{
}

PublishGLTokenHighlight::~PublishGLTokenHighlight()
{
}

void PublishGLTokenHighlight::setPositionScale(const QVector3D& pos, float sizeFactor)
{
    int maxDim = qMax(getWidth(), getHeight());

    if(maxDim <= 0)
        return;

    qreal scaleFactor = sizeFactor / maxDim;

    _modelMatrix.setToIdentity();
    _modelMatrix.translate(pos);
    _modelMatrix.scale(scaleFactor, scaleFactor);
}

QMatrix4x4 PublishGLTokenHighlight::getMatrix() const
{
    return _modelMatrix;
}

const float* PublishGLTokenHighlight::getMatrixData() const
{
    return _modelMatrix.constData();
}
