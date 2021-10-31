#ifndef BATTLEGLOBJECT_H
#define BATTLEGLOBJECT_H

#include "publishglobject.h"
#include "battleglscene.h"

class BattleGLObject : public PublishGLObject
{
    Q_OBJECT

public:
    BattleGLObject(BattleGLScene* scene = nullptr);
    virtual ~BattleGLObject() override;

protected:
    BattleGLScene* _scene;
};

#endif // BATTLEGLOBJECT_H
