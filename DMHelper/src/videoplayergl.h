#ifndef VIDEOPLAYERGL_H
#define VIDEOPLAYERGL_H

#include <QObject>
#include <QSurfaceFormat>
#include <QSize>

class VideoPlayerGL : public QObject
{
    Q_OBJECT
public:
    explicit VideoPlayerGL(QObject *parent = nullptr);

    virtual void registerNewFrame();
    virtual QSurfaceFormat getFormat() const;
    virtual QSize getSize() const;

public slots:
    virtual void videoResized();

signals:
    void contextReady(QOpenGLContext *renderContext);

protected:
    virtual bool initializeVLC();
    virtual bool startPlayer();
    virtual bool stopPlayer();

};

#endif // VIDEOPLAYERGL_H
