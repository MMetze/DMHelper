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
    virtual void paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix) override;

    void setImage(const QImage& image);
    void updateImage(const QImage& image);
    void setScale(float scaleFactor);
    void setScaleX(float scaleFactor);
    void setScaleY(float scaleFactor);
    void setX(float x);
    void setY(float y);
    void setPosition(float x, float y);
    void setPosition(const QPointF& pos);
    void setPositionScaleX(float x, float scaleFactor);
    void setPositionScaleY(float y, float scaleFactor);
    void setPositionScale(float x, float y, float scaleFactor);
    void setPositionScale(const QPointF& pos, float scaleFactor);

    float getX() const;
    float getY() const;
    QPointF getPosition() const;
    QSize getSize() const;
    QSize getImageSize() const;
    float getScale() const;
    float getScaleX() const;
    float getScaleY() const;

protected:
    void createImageObjects(const QImage& image);
    void updateMatrix();

    bool _centered;
    int _textureParam;
    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;

    float _scaleFactor;
    float _scaleX;
    float _scaleY;
    float _x;
    float _y;
    QSize _imageSize;
};

inline bool operator==(const PublishGLImage& lhs, const PublishGLImage& rhs){ return lhs.getTextureID() == rhs.getTextureID(); }
inline bool operator!=(const PublishGLImage& lhs, const PublishGLImage& rhs){return !operator==(lhs, rhs);}


#endif // PUBLISHGLIMAGE_H
