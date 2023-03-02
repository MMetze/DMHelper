#ifndef PUBLISHGLBATTLEBACKGROUND_H
#define PUBLISHGLBATTLEBACKGROUND_H

#include "publishglbattleobject.h"

class PublishGLBattleBackground : public PublishGLBattleObject
{
    Q_OBJECT

public:
    PublishGLBattleBackground(PublishGLScene* scene, const QImage& image, int textureParam);
    virtual ~PublishGLBattleBackground() override;

    virtual void cleanup() override;
    virtual void paintGL() override;

    void setImage(const QImage& image);
    QSize getSize() const;

    void setPosition(const QPoint& position);
    void setTargetSize(const QSize& size);

protected:
    void createImageObjects(const QImage& image);
    void updateModelMatrix();

    QSize _imageSize;
    QPoint _position;
    QSize _targetSize;
    int _textureParam;
    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;
};

#endif // PUBLISHGLBATTLEBACKGROUND_H
