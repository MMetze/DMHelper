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

    PublishGLImage* getImage() const;
    BattleDialogModelEffect* getEffect() const;

protected:
    PublishGLImage* _effectImage;
    BattleDialogModelEffect* _effect;
};

#endif // PUBLISHGLTOKENHIGHLIGHTEFFECT_H
