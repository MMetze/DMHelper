#include "battleglobject.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>

BattleGLObject::BattleGLObject(BattleGLScene& scene) :
    QObject(),
    _scene(scene),
    _textureID(0),
    _modelMatrix()
{
}

BattleGLObject::~BattleGLObject()
{
    if(_textureID > 0)
    {
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        if(!f)
            return;

        f->glDeleteTextures(1, &_textureID);
    }
}

unsigned int BattleGLObject::getTextureID() const
{
    return _textureID;
}

const float * BattleGLObject::getMatrixData() const
{
    return _modelMatrix.constData();
}
