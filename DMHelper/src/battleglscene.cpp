#include "battleglscene.h"
#include "dmconstants.h"

BattleGLScene::BattleGLScene() :
    _targetSize(),
    _sceneRect(),
    _gridScale(DMHelper::STARTING_GRID_SCALE)
{
}

BattleGLScene::~BattleGLScene()
{
}

QSize BattleGLScene::getTargetSize() const
{
    return _targetSize;
}

void BattleGLScene::setTargetSize(const QSize& size)
{
    _targetSize = size;
}

QRectF BattleGLScene::getSceneRect() const
{
    return _sceneRect;
}

void BattleGLScene::setSceneRect(const QRectF& rect)
{
    _sceneRect = rect;
}

void BattleGLScene::deriveSceneRectFromSize(const QSizeF& size)
{
    _sceneRect.setLeft(-size.width() / 2.0);
    _sceneRect.setRight(size.width() / 2.0);
    _sceneRect.setTop(-size.height() / 2.0);
    _sceneRect.setBottom(size.height() / 2.0);
}

int BattleGLScene::getGridScale() const
{
    return _gridScale;
}

void BattleGLScene::setGridScale(int gridScale)
{
    _gridScale = gridScale;
}
