#ifndef PUBLISHGLBATTLESCENE_H
#define PUBLISHGLBATTLESCENE_H

#include <QSize>
#include <QRectF>

class PublishGLBattleScene
{
public:
    PublishGLBattleScene();
    virtual ~PublishGLBattleScene();

    QSize getTargetSize() const;
    void setTargetSize(const QSize& size);

    QRectF getSceneRect() const;
    void setSceneRect(const QRectF& rect);
    void deriveSceneRectFromSize(const QSizeF& size);

    int getGridScale() const;
    void setGridScale(int gridScale);

protected:
    QSize _targetSize;
    QRectF _sceneRect;
    int _gridScale;

};

#endif // PUBLISHGLBATTLESCENE_H
