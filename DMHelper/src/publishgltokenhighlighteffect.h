#ifndef PUBLISHGLTOKENHIGHLIGHTEFFECT_H
#define PUBLISHGLTOKENHIGHLIGHTEFFECT_H

#include "publishgltokenhighlight.h"

class PublishGLImage;
class BattleDialogModelEffect;

class PublishGLTokenHighlightEffect : public PublishGLTokenHighlight
{
    Q_OBJECT
public:
    explicit PublishGLTokenHighlightEffect(PublishGLImage* effectImage, BattleDialogModelEffect* effect, QObject *parent = nullptr);
    virtual ~PublishGLTokenHighlightEffect() override;

    virtual void paintGL(QOpenGLFunctions *f, int shaderModelMatrix) override;

    virtual int getWidth() const override;
    virtual int getHeight() const override;

    virtual void setPositionScale(const QVector3D& pos, float sizeFactor) override;

    PublishGLImage* getImage() const;
    BattleDialogModelEffect* getEffect() const;

protected:
    void createEffectHighlightGL();

    PublishGLImage* _effectImage;
    BattleDialogModelEffect* _effect;
    QVector3D _position;
    float _sizeFactor;
};

#endif // PUBLISHGLTOKENHIGHLIGHTEFFECT_H
