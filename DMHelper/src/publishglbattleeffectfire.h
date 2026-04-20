#ifndef PUBLISHGLBATTLEEFFECTFIRE_H
#define PUBLISHGLBATTLEEFFECTFIRE_H

#include "publishglbattleeffectanimated.h"

class BattleDialogModelEffectFire;

class PublishGLBattleEffectFire : public PublishGLBattleEffectAnimated
{
    Q_OBJECT

public:
    PublishGLBattleEffectFire(PublishGLScene* scene, BattleDialogModelEffectFire* effect);
    virtual ~PublishGLBattleEffectFire() override;

protected:
    virtual const char* getVertexShaderSource() const override;
    virtual const char* getFragmentShaderSource() const override;
    virtual void getEffectUniformLocations(QOpenGLFunctions* f) override;
    virtual void setEffectUniforms(QOpenGLFunctions* f) override;

    int _shaderIntensity;
    int _shaderDarkColor;
    int _shaderLightColor;
    int _shaderFlickerSpeed;
};

#endif // PUBLISHGLBATTLEEFFECTFIRE_H
