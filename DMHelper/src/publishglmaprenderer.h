#ifndef PUBLISHGLMAPRENDERER_H
#define PUBLISHGLMAPRENDERER_H

#include "publishglrenderer.h"
#include <QColor>
#include <QImage>

class Map;
class VideoPlayerGL;

class PublishGLMapRenderer : public PublishGLRenderer
{
    Q_OBJECT
public:
    PublishGLMapRenderer(Map* map, QColor color, QObject *parent = nullptr);
    virtual ~PublishGLMapRenderer() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;
    virtual bool deleteOnDeactivation() override;

    // Standard OpenGL calls
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

    const QImage& getImage() const;
    QColor getColor() const;

public slots:
    void setImage(const QImage& image);
    void setColor(QColor color);

protected:
    void setOrthoProjection();

private:
    Map* _map;
    QImage _image;
    VideoPlayerGL* _videoPlayer;
    QSize _targetSize;
    QColor _color;
    bool _initialized;
    unsigned int _shaderProgram;
};

#endif // PUBLISHGLMAPRENDERER_H
