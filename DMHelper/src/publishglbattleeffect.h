#ifndef PUBLISHGLBATTLEEFFECT_H
#define PUBLISHGLBATTLEEFFECT_H

#include "publishglbattleobject.h"

class BattleDialogModelEffect;
class BattleDialogModelEffectObject;
class QPainter;

class PublishGLBattleEffect : public PublishGLBattleObject
{
    Q_OBJECT

public:
    PublishGLBattleEffect(PublishGLBattleScene* scene, BattleDialogModelEffect* effect);
    virtual ~PublishGLBattleEffect() override;

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

#endif // PUBLISHGLBATTLEEFFECT_H