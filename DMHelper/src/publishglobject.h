#ifndef PUBLISHGLOBJECT_H
#define PUBLISHGLOBJECT_H

#include <QObject>
#include <QMatrix4x4>

class PublishGLObject : public QObject
{
    Q_OBJECT
public:
    explicit PublishGLObject(QObject *parent = nullptr);
    virtual ~PublishGLObject() override;

    virtual void cleanup();
    virtual void paintGL() = 0;

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
