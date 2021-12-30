#ifndef PUBLISHGLMAPVIDEORENDERER_H
#define PUBLISHGLMAPVIDEORENDERER_H

#include "publishglmaprenderer.h"

#define USE_SCREENSHOT_ONLY

class VideoPlayerGLPlayer;

class PublishGLMapVideoRenderer : public PublishGLMapRenderer
{
    Q_OBJECT
public:
    PublishGLMapVideoRenderer(Map* map, QObject *parent = nullptr);
    virtual ~PublishGLMapVideoRenderer() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;

    QImage getLastScreenshot();

#ifdef USE_SCREENSHOT_ONLY
protected slots:
    void handleScreenshotReady(const QImage& image);
#endif

protected:
    // Background overrides
    virtual void initializeBackground() override;
    virtual void resizeBackground(int w, int h) override;
    virtual void paintBackground(QOpenGLFunctions* functions) override;
    virtual QSizeF getBackgroundSize() override;

    VideoPlayerGLPlayer* _videoPlayer;


#ifdef USE_SCREENSHOT_ONLY
    BattleGLBackground* _backgroundObject;
    QImage _backgroundImage;
#endif

};

#endif // PUBLISHGLMAPVIDEORENDERER_H
