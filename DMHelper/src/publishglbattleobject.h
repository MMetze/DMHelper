#ifndef PUBLISHGLBATTLEOBJECT_H
#define PUBLISHGLBATTLEOBJECT_H

#include "publishglobject.h"
#include "publishglscene.h"

class PublishGLBattleObject : public PublishGLObject
{
    Q_OBJECT

public:
    PublishGLBattleObject(PublishGLScene* scene = nullptr);
    virtual ~PublishGLBattleObject() override;

    QPointF sceneToWorld(const QPointF& point) const;
    static QPointF sceneToWorld(const QRectF& sceneRect, const QPointF& point);

protected:
    PublishGLScene* _scene;
};

#endif // PUBLISHGLBATTLEOBJECT_H
