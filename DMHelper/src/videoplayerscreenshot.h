#ifndef VIDEOPLAYERSCREENSHOT_H
#define VIDEOPLAYERSCREENSHOT_H

#include "videoplayer.h"

class VideoPlayerScreenshot : public VideoPlayer
{
    Q_OBJECT
public:
    explicit VideoPlayerScreenshot(const QString& videoFile, QObject *parent = nullptr);
    virtual ~VideoPlayerScreenshot();

    void retrieveScreenshot();

signals:
    void screenshotReady(const QImage& image);

protected slots:
    void handleScreenshot();

protected:
};

#endif // VIDEOPLAYERSCREENSHOT_H
