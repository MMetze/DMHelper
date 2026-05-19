#ifndef PUBLISHGLBATTLEBACKGROUND_H
#define PUBLISHGLBATTLEBACKGROUND_H

#include "publishglbattleobject.h"
#include <QRect>

class PublishGLBattleBackground : public PublishGLBattleObject
{
    Q_OBJECT

public:
    // sourceIsRgba8888: when true, loadTexture() skips the convertToFormat(RGBA8888)
    //   deep copy (FOW path — image is already Format_RGBA8888).
    // sourceNeedsVerticalFlip: when false, loadTexture() skips the CPU vertical flip
    //   and createImageObjects() inverts V-coordinates instead (FOW path).
    // Default values preserve existing behaviour for all non-FOW callers.
    PublishGLBattleBackground(PublishGLScene* scene, const QImage& image, int textureParam,
                               bool sourceIsRgba8888 = false, bool sourceNeedsVerticalFlip = true);
    virtual ~PublishGLBattleBackground() override;

    virtual void cleanup() override;
    virtual void paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix) override;

    void setImage(const QImage& image);
    void updateImage(const QImage& image);

    // Upload a sub-rectangle of the texture using glTexSubImage2D.
    // The image must be Format_RGBA8888 (sourceIsRgba8888=true). Callable only
    // from a *GL* function where the player window's GL context is current.
    void updateImageRegion(const QImage& image, const QRect& region);

    QSize getSize() const;

    void setPosition(const QPoint& position);
    void setTargetSize(const QSize& size);

protected:
    void createImageObjects(const QImage& image);
    void updateModelMatrix();
    void loadTexture(const QImage& image);

    QSize _imageSize;
    QPoint _position;
    QSize _targetSize;
    int _textureParam;
    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;

    // Per-instance source-image flags (set at construction; consumed by loadTexture,
    // createImageObjects, and updateImageRegion).
    bool _sourceIsRgba8888;       // true  → skip convertToFormat in loadTexture (FOW path)
    bool _sourceNeedsVerticalFlip; // false → skip CPU flip; V-coords inverted in vertex array (FOW path)
};

#endif // PUBLISHGLBATTLEBACKGROUND_H
