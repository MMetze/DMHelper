#ifndef PUBLISHGLBATTLEOBJECT_H
#define PUBLISHGLBATTLEOBJECT_H

#include "publishglobject.h"
#include "publishglbattlescene.h"

class PublishGLBattleObject : public PublishGLObject
{
    Q_OBJECT

public:
    PublishGLBattleObject(PublishGLBattleScene* scene = nullptr);
    virtual ~PublishGLBattleObject() override;

    QPointF sceneToWorld(const QPointF& point) const;
    static QPointF sceneToWorld(const QRectF& sceneRect, const QPointF& point);

protected:
    PublishGLBattleScene* _scene;
};

#endif // PUBLISHGLBATTLEOBJECT_H
