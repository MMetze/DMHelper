#include "publishglscene.h"
#include "dmconstants.h"

PublishGLScene::PublishGLScene() :
    _targetSize(),
    _sceneRect(),
    _gridScale(DMHelper::STARTING_GRID_SCALE)
{
}

PublishGLScene::~PublishGLScene()
{
}

QSize PublishGLScene::getTargetSize() const
{
    return _targetSize;
}

void PublishGLScene::setTargetSize(const QSize& size)
{
    _targetSize = size;
}

QRectF PublishGLScene::getSceneRect() const
{
    return _sceneRect;
}

/*
void PublishGLScene::setSceneRect(const QRectF& rect)
{
    _sceneRect = rect;
}
*/

void PublishGLScene::deriveSceneRectFromSize(const QSizeF& size)
{
    _sceneRect.setLeft(-size.width() / 2.0);
    _sceneRect.setRight(size.width() / 2.0);
    _sceneRect.setTop(-size.height() / 2.0);
    _sceneRect.setBottom(size.height() / 2.0);
}

int PublishGLScene::getGridScale() const
{
    return _gridScale;
}

void PublishGLScene::setGridScale(int gridScale)
{
    _gridScale = gridScale;
}
