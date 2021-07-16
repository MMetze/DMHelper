#include "battleglobject.h"

BattleGLObject::BattleGLObject() :
    _textureID(0),
    _modelMatrix()
{
}

unsigned int BattleGLObject::getTextureID() const
{
    return _textureID;
}

const float * BattleGLObject::getMatrixData() const
{
    return _modelMatrix.constData();
}
