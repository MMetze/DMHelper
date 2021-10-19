#ifndef PUBLISHGLIMAGERENDERER_H
#define PUBLISHGLIMAGERENDERER_H

#include "publishglrenderer.h"
#include "battleglscene.h"
#include <QColor>
#include <QImage>

class BattleGLBackground;

class PublishGLImageRenderer : public PublishGLRenderer
{
    Q_OBJECT
public:
    PublishGLImageRenderer(const QImage& image, QColor color, QObject *parent = nullptr);
    virtual ~PublishGLImageRenderer() override;

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
    //void setColor(QColor color);

protected:
    void setOrthoProjection();

private:
    QImage _image;
    QColor _color;
    BattleGLScene _scene;
    bool _initialized;
    unsigned int _shaderProgram;
    BattleGLBackground* _backgroundObject;
};

#endif // PUBLISHGLIMAGERENDERER_H
