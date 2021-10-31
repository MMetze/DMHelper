#ifndef PUBLISHGLMAPRENDERER_H
#define PUBLISHGLMAPRENDERER_H

#include "publishglrenderer.h"
#include <QColor>
#include <QImage>

class Map;
class VideoPlayerGLPlayer;
class BattleGLBackground;
class PublishGLImage;

class PublishGLMapRenderer : public PublishGLRenderer
{
    Q_OBJECT
public:
    PublishGLMapRenderer(Map* map, QObject *parent = nullptr);
    virtual ~PublishGLMapRenderer() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;
    virtual bool deleteOnDeactivation() override;
    virtual void setBackgroundColor(const QColor& color) override;

    // Standard OpenGL calls
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

    QImage getLastScreenshot();
    const QImage& getImage() const;
    QColor getColor() const;

public slots:
    void setImage(const QImage& image);
//    void setColor(QColor color);

protected:
    void setOrthoProjection();

private:
    Map* _map;
    QImage _image;
    VideoPlayerGLPlayer* _videoPlayer;
    QSize _targetSize;
    QColor _color;
    bool _initialized;
    unsigned int _shaderProgram;
    BattleGLBackground* _backgroundObject;
    PublishGLImage* _partyToken;
};

#endif // PUBLISHGLMAPRENDERER_H
