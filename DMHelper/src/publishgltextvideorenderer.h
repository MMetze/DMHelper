#ifndef PUBLISHGLTEXTVIDEORENDERER_H
#define PUBLISHGLTEXTVIDEORENDERER_H

#include "publishgltextrenderer.h"

//#define TEXTVIDEO_USE_SCREENSHOT_ONLY

class VideoPlayerGLPlayer;

class PublishGLTextVideoRenderer : public PublishGLTextRenderer
{
    Q_OBJECT
public:
    PublishGLTextVideoRenderer(EncounterText* encounter, QImage textImage, QObject *parent = nullptr);
    virtual ~PublishGLTextVideoRenderer() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;
    virtual QSizeF getBackgroundSize() override;

    QImage getLastScreenshot();

#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
protected slots:
    void handleScreenshotReady(const QImage& image);
#endif

public slots:
    virtual void play() override;
    virtual void stop() override;

protected:
    // Background overrides
    virtual void initializeBackground() override;
    virtual bool isBackgroundReady() override;
    virtual void resizeBackground(int w, int h) override;
    virtual void paintBackground(QOpenGLFunctions* functions) override;
    virtual void updateBackground() override;

    void cleanupBackground();

    VideoPlayerGLPlayer* _videoPlayer;

#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
    PublishGLBattleBackground* _backgroundObject;
    QImage _backgroundImage;
#endif
};

#endif // PUBLISHGLTEXTVIDEORENDERER_H
