#ifndef PUBLISHGLOBJECT_H
#define PUBLISHGLOBJECT_H

#include "dmh_opengl.h"
#include <QObject>
#include <QMatrix4x4>

class PublishGLObject : public QObject
{
    Q_OBJECT
public:
    explicit PublishGLObject(QObject *parent = nullptr);
    virtual ~PublishGLObject() override;

    virtual void cleanup();
    virtual void paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix) = 0;
    virtual bool hasCustomShaders() const;

    unsigned int getTextureID() const;
    QMatrix4x4 getMatrix() const;
    const float * getMatrixData() const;

signals:
    void changed();

protected:
    unsigned int _textureID;
    QMatrix4x4 _modelMatrix;

};

#endif // PUBLISHGLOBJECT_H
