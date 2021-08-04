#include "battleglobject.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>

BattleGLObject::BattleGLObject(BattleGLScene* scene) :
    QObject(),
    _scene(scene),
    _textureID(0),
    _modelMatrix()
{
}

BattleGLObject::~BattleGLObject()
{
    BattleGLObject::cleanup();
}

void BattleGLObject::cleanup()
{
    if(_textureID > 0)
    {
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        if(!f)
            return;

        f->glDeleteTextures(1, &_textureID);
        _textureID = 0;
    }
}

unsigned int BattleGLObject::getTextureID() const
{
    return _textureID;
}

QMatrix4x4 BattleGLObject::getMatrix() const
{
    return _modelMatrix;
}

const float * BattleGLObject::getMatrixData() const
{
    return _modelMatrix.constData();
}
