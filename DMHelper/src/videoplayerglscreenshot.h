#ifndef VIDEOPLAYERGLSCREENSHOT_H
#define VIDEOPLAYERGLSCREENSHOT_H

#include "videoplayergl.h"
#include "dmh_vlc.h"

class VideoPlayerGLVideo;

class VideoPlayerGLScreenshot : public VideoPlayerGL
{
    Q_OBJECT
public:
    explicit VideoPlayerGLScreenshot(const QString& videoFile, QObject *parent = nullptr);
    virtual ~VideoPlayerGLScreenshot();

    void retrieveScreenshot();

    virtual void registerNewFrame() override;

signals:
    void screenshotReady(const QImage& image);

protected:
    virtual bool initializeVLC() override;
    virtual bool startPlayer() override;
    virtual bool stopPlayer() override;

    QImage extractImage();

    QString _videoFile;
    VideoPlayerGLVideo* _video;
    libvlc_media_player_t* _vlcPlayer;
    libvlc_media_t* _vlcMedia;

};

#endif // VIDEOPLAYERGLSCREENSHOT_H
