#ifndef PUBLISHGLBATTLEEFFECTVIDEO_H
#define PUBLISHGLBATTLEEFFECTVIDEO_H

#include "publishglbattleeffect.h"

class BattleDialogModelEffectObjectVideo;
class VideoPlayer;
//class PublishGLBattleBackground;

class PublishGLBattleEffectVideo : public PublishGLBattleEffect
{
    Q_OBJECT
public:
    PublishGLBattleEffectVideo(PublishGLScene* scene, BattleDialogModelEffectObjectVideo* effect);
    virtual ~PublishGLBattleEffectVideo() override;

    virtual void cleanup() override;
    virtual void prepareObjectsGL() override;
    virtual void paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix) override;
    virtual bool hasCustomShaders() const override;

    virtual BattleDialogModelEffectObjectVideo* getEffectVideo() const;

signals:
    void updateWidget();

protected:
    void createShadersGL();
    void cleanupShadersGL();
    const char* getVertexShaderSource(BattleDialogModelEffectObjectVideo* effectVideo);
    const char* getFragmentShaderSource(BattleDialogModelEffectObjectVideo* effectVideo);

    VideoPlayer* _videoPlayer;
    QSize _playerSize;

    unsigned int _shaderProgramRGBA;
    int _shaderModelMatrixRGBA;
    int _shaderProjectionMatrixRGBA;
    int _shaderAlphaRGBA;
    int _shaderTransparentColor;
    int _shaderTransparentTolerance;
    int _shaderColorizeColor;
};

#endif // PUBLISHGLBATTLEEFFECTVIDEO_H
