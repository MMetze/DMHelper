#ifndef PUBLISHGLBATTLEEFFECTANIMATED_H
#define PUBLISHGLBATTLEEFFECTANIMATED_H

#include "publishglbattleeffect.h"
#include <QBasicTimer>

class LayerTokens;

class PublishGLBattleEffectAnimated : public PublishGLBattleEffect
{
    Q_OBJECT

public:
    PublishGLBattleEffectAnimated(PublishGLScene* scene, BattleDialogModelEffect* effect);
    virtual ~PublishGLBattleEffectAnimated() override;

    virtual void cleanup() override;
    virtual void prepareObjectsGL() override;
    virtual void paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix) override;
    virtual bool hasCustomShaders() const override;

signals:
    void updateWidget();

protected:
    virtual void timerEvent(QTimerEvent *event) override;

    void createShadersGL();
    void cleanupShadersGL();

    void createQuadGL();
    void cleanupQuadGL();

    virtual const char* getVertexShaderSource() const = 0;
    virtual const char* getFragmentShaderSource() const = 0;
    virtual void getEffectUniformLocations(QOpenGLFunctions* f);
    virtual void setEffectUniforms(QOpenGLFunctions* f) = 0;

    unsigned int _shaderProgram;
    int _shaderModelMatrix;
    int _shaderProjectionMatrix;
    int _shaderAlpha;
    int _shaderTime;
    int _shaderColor;
    int _shaderSize;

    QBasicTimer _animationTimer;
    qint64 _milliseconds;
};

#endif // PUBLISHGLBATTLEEFFECTANIMATED_H
