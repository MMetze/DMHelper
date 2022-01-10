#ifndef BATTLEGLEFFECT_H
#define BATTLEGLEFFECT_H

#include "battleglobject.h"

class BattleDialogModelEffect;
class BattleDialogModelEffectObject;
class QPainter;

class BattleGLEffect : public BattleGLObject
{
    Q_OBJECT
public:
    BattleGLEffect(BattleGLScene* scene, BattleDialogModelEffect* effect);
    virtual ~BattleGLEffect() override;

    virtual void cleanup() override;
    virtual void paintGL() override;

public slots:
    void effectMoved();

protected:
    virtual void drawShape(QPainter& painter, BattleDialogModelEffect* effect, int effectSize, int effectWidth);
    virtual void drawObject(QPainter& painter, BattleDialogModelEffectObject* effectObject, int effectSize, int effectWidth);

    BattleDialogModelEffect* _effect;
    BattleDialogModelEffectObject* _childEffect;
    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;
    QSizeF _textureSize;
    qreal _imageScaleFactor;
};

#endif // BATTLEGLEFFECT_H
