#ifndef LAYERIMAGE_H
#define LAYERIMAGE_H

#include "layer.h"
#include "mapcolorizefilter.h"
#include <QImage>

class QGraphicsPixmapItem;
class PublishGLBattleBackground;

class LayerImage : public Layer
{
    Q_OBJECT
public:
    explicit LayerImage(const QImage& image, int order = 0, QObject *parent = nullptr);
    virtual ~LayerImage() override;

    virtual QRectF boundingRect() const override;    
    virtual DMHelper::LayerType getType() const override;

    QImage getImage() const;
    QImage getImageUnfiltered() const;

    bool isFilterApplied() const;
    MapColorizeFilter getFilter() const;

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene& scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize() override;
    virtual void playerGLUninitialize() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint modelMatrix) override;
    virtual void playerGLResize(int w, int h) override;

    // Local Interface
    void updateImage(const QImage& image);
    void setApplyFilter(bool applyFilter);
    void setFilter(const MapColorizeFilter& filter);

signals:
    void imageChanged(const QImage& image);

protected slots:
    // Local Interface
    void updateImageInternal();

protected:
    // DM Window Methods
    void cleanupDM();

    // Player Window Methods
    void cleanupPlayer();

    // DM Window Members
    QGraphicsPixmapItem* _graphicsItem;

    // Player Window Members
    PublishGLBattleBackground* _backgroundObject;

    // Core contents
    QImage _layerImage;
    bool _filterApplied;
    MapColorizeFilter _filter;

};

#endif // LAYERIMAGE_H
