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
    PublishGLBattleEffect(PublishGLScene* scene, BattleDialogModelEffect* effect);
    virtual ~PublishGLBattleEffect() override;

    virtual void cleanup() override;
    virtual void prepareObjectsGL();
    virtual void paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix) override;

    virtual BattleDialogModelEffect* getEffect() const;
    virtual qreal getEffectAlpha() const;

public slots:
    virtual void effectMoved();
    virtual void effectChanged();

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
    bool _recreateEffect;
};

#endif // PUBLISHGLBATTLEEFFECT_H
