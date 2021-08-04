#ifndef BATTLEGLBACKGROUND_H
#define BATTLEGLBACKGROUND_H

#include "battleglobject.h"

class BattleGLBackground : public BattleGLObject
{
    Q_OBJECT

public:
    BattleGLBackground(BattleGLScene* scene, const QImage& image, int textureParam);
    virtual ~BattleGLBackground() override;

    virtual void cleanup() override;
    virtual void paintGL() override;

    void setImage(const QImage& image);

protected:
    void createImageObjects(const QImage& image);

    int _textureParam;
    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;
};

#endif // BATTLEGLBACKGROUND_H
