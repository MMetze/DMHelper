#ifndef BATTLEGLSCENE_H
#define BATTLEGLSCENE_H

#include <QSize>
#include <QRectF>

class BattleGLScene
{
public:
    BattleGLScene();
    virtual ~BattleGLScene();

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

#endif // BATTLEGLSCENE_H
