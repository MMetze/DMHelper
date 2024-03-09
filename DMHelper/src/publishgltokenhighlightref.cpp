#include "publishgltokenhighlightref.h"
#include "publishglimage.h"
#include <QOpenGLFunctions>

PublishGLTokenHighlightRef::PublishGLTokenHighlightRef(PublishGLImage& referenceImage, QObject *parent) :
    PublishGLTokenHighlight{parent},
    _referenceImage(referenceImage)
{
}

PublishGLTokenHighlightRef::~PublishGLTokenHighlightRef()
{
}

void PublishGLTokenHighlightRef::paintGL(QOpenGLFunctions *f, int shaderModelMatrix)
{
    if(!f)
        return;

    f->glUniformMatrix4fv(shaderModelMatrix, 1, GL_FALSE, getMatrixData());
    _referenceImage.paintGL(f, nullptr);
}

int PublishGLTokenHighlightRef::getWidth() const
{
    return _referenceImage.getImageSize().width();
}

int PublishGLTokenHighlightRef::getHeight() const
{
    return _referenceImage.getImageSize().height();
}

PublishGLImage& PublishGLTokenHighlightRef::getImage() const
{
    return _referenceImage;
}
