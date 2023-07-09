#ifndef PUBLISHGLTOKENHIGHLIGHT_H
#define PUBLISHGLTOKENHIGHLIGHT_H

#include <QObject>
#include <QMatrix4x4>

class QOpenGLFunctions;

class PublishGLTokenHighlight : public QObject
{
    Q_OBJECT
public:
    explicit PublishGLTokenHighlight(QObject *parent = nullptr);
    virtual ~PublishGLTokenHighlight() override;

    virtual void paintGL(QOpenGLFunctions *f, int shaderModelMatrix) = 0;

    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;

    virtual void setPositionScale(const QVector3D& pos, float sizeFactor);

    QMatrix4x4 getMatrix() const;
    const float* getMatrixData() const;

protected:
    QMatrix4x4 _modelMatrix;

};

#endif // PUBLISHGLTOKENHIGHLIGHT_H
