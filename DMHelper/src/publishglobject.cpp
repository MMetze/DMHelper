#include "publishglobject.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>

PublishGLObject::PublishGLObject(QObject *parent) :
    QObject(parent),
    _textureID(0),
    _modelMatrix()
{
}

PublishGLObject::~PublishGLObject()
{
    PublishGLObject::cleanup();
}

void PublishGLObject::cleanup()
{
    if(_textureID > 0)
    {
        if(QOpenGLContext::currentContext())
        {
            QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
            if(f)
                f->glDeleteTextures(1, &_textureID);
        }
        _textureID = 0;
    }
}

unsigned int PublishGLObject::getTextureID() const
{
    return _textureID;
}

QMatrix4x4 PublishGLObject::getMatrix() const
{
    return _modelMatrix;
}

const float* PublishGLObject::getMatrixData() const
{
    return _modelMatrix.constData();
}