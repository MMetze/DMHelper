#ifndef BATTLEGLOBJECT_H
#define BATTLEGLOBJECT_H

#include <QMatrix4x4>

class BattleGLObject
{
public:
    BattleGLObject();
    virtual ~BattleGLObject();

    virtual void paintGL() = 0;

    unsigned int getTextureID() const;
    const float * getMatrixData() const;

protected:
    unsigned int _textureID;
    QMatrix4x4 _modelMatrix;

};

#endif // BATTLEGLOBJECT_H
