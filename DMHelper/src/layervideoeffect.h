#ifndef LAYERVIDEOEFFECT_H
#define LAYERVIDEOEFFECT_H

#include "layervideo.h"
#include <QObject>

class LayerVideoEffect : public LayerVideo
{
    Q_OBJECT
public:
    enum LayerVideoEffectType
    {
        LayerVideoEffectType_None = 0,
        LayerVideoEffectType_Red,
        LayerVideoEffectType_Green,
        LayerVideoEffectType_Blue,
        LayerVideoEffectType_TransparentColor
    };

    explicit LayerVideoEffect(const QString& name = QString(), const QString& filename = QString(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerVideoEffect() override;

    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;

public slots:
    // Player Window Generic Interface
    virtual void playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene) override;
    virtual void playerGLUninitialize() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix) override;
    virtual void playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA) override;

    // Layer Specific Interface
    virtual void editSettings() override;

protected:
    void createShadersGL();
    void cleanupShadersGL();
    const char* getVertexShaderSource();
    const char* getFragmentShaderSource();

    bool _recreateShaders;
    LayerVideoEffectType _effectType;
    bool _colorize;
    QColor _transparentColor;
    QColor _colorizeColor;
};

#endif // LAYERVIDEOEFFECT_H
