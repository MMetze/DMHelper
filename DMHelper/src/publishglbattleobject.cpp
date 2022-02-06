#include "publishglbattleobject.h"

PublishGLBattleObject::PublishGLBattleObject(PublishGLBattleScene* scene) :
    PublishGLObject(),
    _scene(scene)
{
}

PublishGLBattleObject::~PublishGLBattleObject()
{
}

QPointF PublishGLBattleObject::sceneToWorld(const QPointF& point) const
{
    QPointF result;

    if(_scene)
        result = QPointF(point.x() - (_scene->getSceneRect().width() / 2),
                         (_scene->getSceneRect().height() / 2) - point.y());

    return result;
}

QPointF PublishGLBattleObject::sceneToWorld(const QRectF& sceneRect, const QPointF& point)
{
    return QPointF(point.x() - (sceneRect.width() / 2), (sceneRect.height() / 2) - point.y());
}
