#ifndef PUBLISHGLBATTLEEFFECTSMOKE_H
#define PUBLISHGLBATTLEEFFECTSMOKE_H

#include "publishglbattleeffectanimated.h"

class BattleDialogModelEffectSmoke;

class PublishGLBattleEffectSmoke : public PublishGLBattleEffectAnimated
{
    Q_OBJECT

public:
    PublishGLBattleEffectSmoke(PublishGLScene* scene, BattleDialogModelEffectSmoke* effect);
    virtual ~PublishGLBattleEffectSmoke() override;

protected:
    virtual const char* getVertexShaderSource() const override;
    virtual const char* getFragmentShaderSource() const override;
    virtual void getEffectUniformLocations(QOpenGLFunctions* f) override;
    virtual void setEffectUniforms(QOpenGLFunctions* f) override;

    int _shaderDensity;
    int _shaderCenterColor;
    int _shaderEdgeColor;
    int _shaderBillowFactor;
    int _shaderWindVec;
};

#endif // PUBLISHGLBATTLEEFFECTSMOKE_H
