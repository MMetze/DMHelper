#include "camerascene.h"

CameraScene::CameraScene(QObject *parent) :
    QGraphicsScene(parent)
{
}

CameraScene::~CameraScene()
{
}

void CameraScene::handleItemChanged(QGraphicsItem* item)
{
    emit itemChanged(item);
}
