#ifndef PUBLISHGLBATTLEBACKGROUND_H
#define PUBLISHGLBATTLEBACKGROUND_H

#include "publishglbattleobject.h"

class PublishGLBattleBackground : public PublishGLBattleObject
{
    Q_OBJECT

public:
    PublishGLBattleBackground(PublishGLBattleScene* scene, const QImage& image, int textureParam);
    virtual ~PublishGLBattleBackground() override;

    virtual void cleanup() override;
    virtual void paintGL() override;

    void setImage(const QImage& image);
    QSize getSize() const;

protected:
    void createImageObjects(const QImage& image);

    QSize _imageSize;
    int _textureParam;
    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;
};

#endif // PUBLISHGLBATTLEBACKGROUND_H
