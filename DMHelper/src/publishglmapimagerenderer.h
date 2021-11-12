#ifndef PUBLISHGLMAPIMAGERENDERER_H
#define PUBLISHGLMAPIMAGERENDERER_H

#include "publishglrenderer.h"
#include <QColor>
#include <QImage>

class Map;
class BattleGLBackground;
class PublishGLImage;

class PublishGLMapImageRenderer : public PublishGLRenderer
{
    Q_OBJECT
public:
    PublishGLMapImageRenderer(Map* map, QObject *parent = nullptr);
    virtual ~PublishGLMapImageRenderer() override;

    virtual CampaignObjectBase* getObject() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;
    virtual bool deleteOnDeactivation() override;
    virtual void setBackgroundColor(const QColor& color) override;

    // Standard OpenGL calls
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

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
    QSize _targetSize;
    QColor _color;
    bool _initialized;
    unsigned int _shaderProgram;
    BattleGLBackground* _backgroundObject;
    PublishGLImage* _partyToken;
};

#endif // PUBLISHGLMAPIMAGERENDERER_H
