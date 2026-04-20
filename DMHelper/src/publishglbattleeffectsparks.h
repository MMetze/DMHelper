#ifndef PUBLISHGLBATTLEEFFECTSPARKS_H
#define PUBLISHGLBATTLEEFFECTSPARKS_H

#include "publishglbattleeffectanimated.h"

class BattleDialogModelEffectSparks;

class PublishGLBattleEffectSparks : public PublishGLBattleEffectAnimated
{
    Q_OBJECT

public:
    PublishGLBattleEffectSparks(PublishGLScene* scene, BattleDialogModelEffectSparks* effect);
    virtual ~PublishGLBattleEffectSparks() override;

    virtual void cleanup() override;
    virtual void prepareObjectsGL() override;
    virtual void paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix) override;

protected:
    virtual const char* getVertexShaderSource() const override;
    virtual const char* getFragmentShaderSource() const override;
    virtual void getEffectUniformLocations(QOpenGLFunctions* f) override;
    virtual void setEffectUniforms(QOpenGLFunctions* f) override;

    void createParticleGeometry();

    int _shaderGlowRadius;
    int _shaderGlowOpacity;
    int _shaderArcFalloff;
    int _shaderFadeDistance;
    int _shaderSparkSpeed;
    int _shaderWindVec;

    int _particleCount;
    bool _objectsDirty;
};

#endif // PUBLISHGLBATTLEEFFECTSPARKS_H
