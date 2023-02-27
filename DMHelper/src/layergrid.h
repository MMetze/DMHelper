#ifndef LAYERGRID_H
#define LAYERGRID_H

#include "layer.h"
#include "gridconfig.h"

class Grid;
class PublishGLBattleGrid;

class LayerGrid : public Layer
{
    Q_OBJECT
public:
    explicit LayerGrid(const QString& name = QString(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerGrid() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual QImage getLayerIcon() const override;
    virtual bool defaultShader() const override;

    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;

    // Local Layer Interface (generally should call set*() versions below
    virtual void applyOrder(int order) override;
    virtual void applyLayerVisible(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;
    virtual void applyPosition(const QPoint& position) override;
    virtual void applySize(const QSize& size) override;

    QSize getLayerSize() const;
    void setLayerSize(const QSize& layerSize);

    GridConfig& getConfig();
    const GridConfig& getConfig() const;

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

    // Layer Specific Interface
    virtual void initialize(const QSize& layerSize) override;
    virtual void uninitialize() override;
    virtual void setScale(int scale) override;

    void setConfig(const GridConfig& config);

protected:
    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    // DM Window Methods
    void cleanupDM();

    // Player Window Methods
    void cleanupPlayer();

    // DM Window Members
    Grid* _grid;

    // Player Window Members
    PublishGLBattleGrid* _gridObject;

    // Core contents
    GridConfig _config;
    QSize _layerSize;

};

#endif // LAYERGRID_H
