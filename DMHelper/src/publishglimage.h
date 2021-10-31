#ifndef PUBLISHGLIMAGE_H
#define PUBLISHGLIMAGE_H

#include "publishglobject.h"
#include <QImage>

class PublishGLImage : public PublishGLObject
{
    Q_OBJECT
public:
    PublishGLImage(const QImage& image, bool centered = true, QObject *parent = nullptr);
    PublishGLImage(const QImage& image, int textureParam, bool centered = true, QObject *parent = nullptr);
    virtual ~PublishGLImage() override;

    virtual void cleanup() override;
    virtual void paintGL() override;

    void setImage(const QImage& image);
    void setScale(float scaleFactor);
    void setPosition(float x, float y);

protected:
    void createImageObjects(const QImage& image);
    void updateMatrix();

    bool _centered;
    int _textureParam;
    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;

    float _scaleFactor;
    float _x;
    float _y;
};

#endif // PUBLISHGLIMAGE_H
