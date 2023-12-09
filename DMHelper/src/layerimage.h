#ifndef LAYERIMAGE_H
#define LAYERIMAGE_H

#include "layer.h"
#include "mapcolorizefilter.h"
#include <QImage>

class PublishGLBattleBackground;
class QGraphicsPixmapItem;

class LayerImage : public Layer
{
    Q_OBJECT
public:
    explicit LayerImage(const QString& name = QString(), const QString& filename = QString(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerImage() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual QRectF boundingRect() const override;
    virtual QImage getLayerIcon() const override;
    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;

    // Local Interface
    virtual void applyOrder(int order) override;
    virtual void applyLayerVisibleDM(bool layerVisible) override;
    virtual void applyLayerVisiblePlayer(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;
    virtual void applyPosition(const QPoint& position) override;
    virtual void applySize(const QSize& size) override;

    QString getImageFile() const;
    QImage getImage() const;
    QImage getImageUnfiltered() const;

    bool isFilterApplied() const;
    MapColorizeFilter getFilter() const;

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene* scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene) override;
    virtual void playerGLUninitialize() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix) override;
    virtual void playerGLResize(int w, int h) override;
    virtual bool playerIsInitialized() override;

    // Layer Specific Interface
    virtual void initialize(const QSize& sceneSize) override;
    virtual void uninitialize() override;

    // Local Interface
    void updateImage(const QImage& image);
    void setFileName(const QString& filename);
    void setApplyFilter(bool applyFilter);
    void setFilter(const MapColorizeFilter& filter);

signals:
    void imageChanged(const QImage& image);

protected slots:
    // Local Interface
    void updateImageInternal(const QImage& newImage);
    void changeImageInternal(const QImage& newImage);

protected:
    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    // DM Window Methods
    void cleanupDM();

    // Player Window Methods
    void cleanupPlayer();

    // DM Window Members
    QGraphicsPixmapItem* _graphicsItem;

    // Player Window Members
    PublishGLBattleBackground* _imageGLObject;
    PublishGLScene* _scene;

    // Core contents
    QString _filename;
    QImage _originalImage;
    QImage _layerImage;
    bool _filterApplied;
    MapColorizeFilter _filter;

};

#endif // LAYERIMAGE_H
