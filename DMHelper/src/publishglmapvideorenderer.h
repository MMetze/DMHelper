#ifndef PUBLISHGLMAPVIDEORENDERER_H
#define PUBLISHGLMAPVIDEORENDERER_H

#include "publishglmaprenderer.h"

//#define MAPVIDEO_USE_SCREENSHOT_ONLY

class VideoPlayerGLPlayer;

class PublishGLMapVideoRenderer : public PublishGLMapRenderer
{
    Q_OBJECT
public:
    PublishGLMapVideoRenderer(Map* map, QObject *parent = nullptr);

    // DMH OpenGL renderer calls
    virtual void cleanup() override;

    QImage getLastScreenshot();

#ifdef MAPVIDEO_USE_SCREENSHOT_ONLY
protected slots:
    void handleScreenshotReady(const QImage& image);
#endif

protected:
    // Background overrides
    virtual void initializeBackground() override;
    virtual bool isBackgroundReady() override;
    virtual void resizeBackground(int w, int h) override;
    virtual void paintBackground(QOpenGLFunctions* functions) override;
    virtual QSizeF getBackgroundSize() override;
    virtual void updateBackground() override;

    VideoPlayerGLPlayer* _videoPlayer;

#ifdef MAPVIDEO_USE_SCREENSHOT_ONLY
    PublishGLBattleBackground* _backgroundObject;
    QImage _backgroundImage;
#endif
};

#endif // PUBLISHGLMAPVIDEORENDERER_H
