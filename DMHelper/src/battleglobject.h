#ifndef BATTLEGLOBJECT_H
#define BATTLEGLOBJECT_H

#include <QObject>
#include <QMatrix4x4>
#include "battleglscene.h"

class BattleGLObject : public QObject
{
    Q_OBJECT

public:
    BattleGLObject(BattleGLScene& scene);
    virtual ~BattleGLObject() override;

    virtual void paintGL() = 0;

    unsigned int getTextureID() const;
    const float * getMatrixData() const;

signals:
    void changed();

protected:
    BattleGLScene& _scene;
    unsigned int _textureID;
    QMatrix4x4 _modelMatrix;

};

#endif // BATTLEGLOBJECT_H
