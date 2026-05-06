#ifndef PUBLISHGLBATTLEEFFECTLIGHT_H
#define PUBLISHGLBATTLEEFFECTLIGHT_H

#include "publishglbattleeffectanimated.h"

class BattleDialogModelEffectLight;

class PublishGLBattleEffectLight : public PublishGLBattleEffectAnimated
{
    Q_OBJECT

public:
    PublishGLBattleEffectLight(PublishGLScene* scene, BattleDialogModelEffectLight* effect);
    virtual ~PublishGLBattleEffectLight() override;

protected:
    virtual const char* getVertexShaderSource() const override;
    virtual const char* getFragmentShaderSource() const override;
    virtual void getEffectUniformLocations(QOpenGLFunctions* f) override;
    virtual void setEffectUniforms(QOpenGLFunctions* f) override;

    int _shaderGradientType;
    int _shaderFlickerEnabled;
    int _shaderFlickerFrequency;
    int _shaderDimAmplitude;
};

#endif // PUBLISHGLBATTLEEFFECTLIGHT_H
