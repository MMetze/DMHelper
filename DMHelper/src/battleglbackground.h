#ifndef BATTLEGLBACKGROUND_H
#define BATTLEGLBACKGROUND_H

#include "battleglobject.h"

class BattleGLBackground : public BattleGLObject
{
public:
    BattleGLBackground(const QImage& image, int textureParam);
    virtual ~BattleGLBackground() override;

    virtual void paintGL() override;

protected:
    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;
};

#endif // BATTLEGLBACKGROUND_H