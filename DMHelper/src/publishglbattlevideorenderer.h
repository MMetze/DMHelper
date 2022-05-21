#ifndef PUBLISHGLBATTLEVIDEORENDERER_H
#define PUBLISHGLBATTLEVIDEORENDERER_H

#include "publishglbattlerenderer.h"

//#define BATTLEVIDEO_USE_SCREENSHOT_ONLY

class VideoPlayerGLPlayer;

class PublishGLBattleVideoRenderer : public PublishGLBattleRenderer
{
    Q_OBJECT
public:
    PublishGLBattleVideoRenderer(BattleDialogModel* model, QObject *parent = nullptr);
    virtual ~PublishGLBattleVideoRenderer() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;
    virtual QSizeF getBackgroundSize() override;

    QImage getLastScreenshot();

#ifdef BATTLEVIDEO_USE_SCREENSHOT_ONLY
protected slots:
    void handleScreenshotReady(const QImage& image);
#endif

protected:
    // Background overrides
    virtual void initializeBackground() override;
    virtual bool isBackgroundReady() override;
    virtual void resizeBackground(int w, int h) override;
    virtual void paintBackground(QOpenGLFunctions* functions) override;
    virtual void updateBackground() override;

    VideoPlayerGLPlayer* _videoPlayer;

#ifdef BATTLEVIDEO_USE_SCREENSHOT_ONLY
    PublishGLBattleBackground* _backgroundObject;
    QImage _backgroundImage;
#endif
};

#endif // PUBLISHGLBATTLEVIDEORENDERER_H
