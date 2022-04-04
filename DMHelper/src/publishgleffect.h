#ifndef PUBLISHGLEFFECT_H
#define PUBLISHGLEFFECT_H

#include <QObject>
#include <QMatrix4x4>

class PublishGLImage;

class PublishGLEffect : public QObject
{
    Q_OBJECT
public:
    explicit PublishGLEffect(PublishGLImage& effectImage, QObject *parent = nullptr);
    virtual ~PublishGLEffect() override;

    void setPositionScale(const QVector3D& pos, float sizeFactor);

    PublishGLImage& getImage() const;
    QMatrix4x4 getMatrix() const;
    const float* getMatrixData() const;

protected:
    PublishGLImage& _effectImage;
    QMatrix4x4 _modelMatrix;

};

#endif // PUBLISHGLEFFECT_H
