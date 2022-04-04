#include "publishglbattlescene.h"
#include "dmconstants.h"

PublishGLBattleScene::PublishGLBattleScene() :
    _targetSize(),
    _sceneRect(),
    _gridScale(DMHelper::STARTING_GRID_SCALE)
{
}

PublishGLBattleScene::~PublishGLBattleScene()
{
}

QSize PublishGLBattleScene::getTargetSize() const
{
    return _targetSize;
}

void PublishGLBattleScene::setTargetSize(const QSize& size)
{
    _targetSize = size;
}

QRectF PublishGLBattleScene::getSceneRect() const
{
    return _sceneRect;
}

void PublishGLBattleScene::setSceneRect(const QRectF& rect)
{
    _sceneRect = rect;
}

void PublishGLBattleScene::deriveSceneRectFromSize(const QSizeF& size)
{
    _sceneRect.setLeft(-size.width() / 2.0);
    _sceneRect.setRight(size.width() / 2.0);
    _sceneRect.setTop(-size.height() / 2.0);
    _sceneRect.setBottom(size.height() / 2.0);
}

int PublishGLBattleScene::getGridScale() const
{
    return _gridScale;
}

void PublishGLBattleScene::setGridScale(int gridScale)
{
    _gridScale = gridScale;
}
