#ifndef LAYERFOW_H
#define LAYERFOW_H

#include "layer.h"
#include "mapcontent.h"
#include <QImage>

class FowGraphicsItem;
class PublishGLBattleBackground;
class QTimer;
class QUndoStack;
class UndoFowBase;

class LayerFow : public Layer
{
    Q_OBJECT
public:
    explicit LayerFow(const QString& name = QString(), const QSize& imageSize = QSize(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerFow() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual QRectF boundingRect() const override;
    virtual QImage getLayerIcon() const override;
    virtual bool hasSettings() const override;
    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;

    // Local Layer Interface (generally should call set*() versions below
    virtual void applyOrder(int order) override;
    virtual void applyLayerVisibleDM(bool layerVisible) override;
    virtual void applyLayerVisiblePlayer(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;
    virtual void applyPosition(const QPoint& position) override;
    virtual void applySize(const QSize& size) override;

    QImage getImage() const;

    QUndoStack* getUndoStack() const;
    void undoPaint();
    void applyPaintTo(int index, int startIndex = 0);

    void paintFoWPoint(QPoint point, const MapDraw& mapDraw);
    void paintFoWPoints(const QList<QPoint>& points, const MapDraw& mapDraw);
    void paintFoWRect(QRect rect, const MapEditShape& mapEditShape);
    void paintFoWPolygon(const MapEditPolygon& mapEditPolygon);
    void fillFoW(const QColor& color);

    QRect getFoWVisibleRect() const;

    void dipOpacity();
    void raiseOpacity();
    void resetOpacity();

    void flushPendingUpdate();

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
    virtual void aboutToDelete() override;
    virtual void editSettings() override;

signals:
    // Emitted by dispatchFowUpdate() to wake the player renderer. Visual-only —
    // do NOT connect to save logic.
    void changed();

protected:
    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    void challengeUndoStack();

    // DM Window Methods
    void cleanupDM();

    // Player Window Methods
    void cleanupPlayer();
    void fillFoWImage();

    // Generic Methods
    void initializeUndoStack();

    // DM Window Members
    FowGraphicsItem* _graphicsItem;

    // Player Window Members
    PublishGLBattleBackground* _fowGLObject;
    PublishGLScene* _scene;

    // Core contents
    QColor _fowColor;
    // Stored as Format_ARGB32_Premultiplied — Qt's software rasterizer has SIMD-optimised
    // compositing paths (CompositionMode_Source, CompositionMode_DestinationIn) for this
    // format. _cachedImage is kept as RGBA8888 for GL upload; the format conversion happens
    // once per coalesce window in dispatchFowUpdate(), not on every brush-stroke pixel.
    QImage _imageFow;
    // Same format as _imageFow so getImage() compositing stays within
    // ARGB32_Premultiplied throughout the CPU painting path.
    QImage _imageFowTexture;
    QString _fowTextureFile;
    int _fowTextureScale;
    QUndoStack* _undoStack;
    QList<UndoFowBase*> _undoItems;
    bool _batchProcessing;

private slots:
    // Unions region into _pendingDirtyRect and starts the coalescing timer if not
    // already running. Called by paintFoWPoint / paintFoWPoints with the footprint
    // rect of the changed area.
    void requestFowUpdate(const QRect& region);

private:
    // Pending-state flags set by GUI-thread paths; consumed only from *GL* functions.
    // See applyGLPendingUpdates() — the sole function permitted to call GL-state-
    // mutating methods on _fowGLObject.
    bool _fowGLImageDirty = false;
    bool _fowGLDeferredDestroy = false;

    // Callable only from a *GL* function with a current GL context.
    void applyGLPendingUpdates();

    // Deferred-upload contract — runs on the GUI thread. Sets pending flags and emits
    // signals; never calls into _fowGLObject. The next playerGLPaint() consumes
    // _fowGLImageDirty / _fowGLPendingRegion via applyGLPendingUpdates().
    void dispatchFowUpdate(const QRect& region);

    // Accumulated dirty rect for the coalescing timer (DM-side). Unioned by
    // requestFowUpdate(); consumed and reset by the timer slot and flushPendingUpdate().
    QRect _pendingDirtyRect;

    // Accumulated dirty region for the GL upload path (player-side). Unioned by
    // dispatchFowUpdate(); consumed and reset by applyGLPendingUpdates() inside
    // playerGLPaint(). Chunk 4 will use this for glTexSubImage2D; chunk 3 populates
    // it so the accumulator is ready.
    QRect _fowGLPendingRegion;

    // Cached composed image in Format_RGBA8888 (LayerFow owns this; FowGraphicsItem holds
    // a pointer to it). Updated by dispatchFowUpdate() via partial QPainter composite from
    // _imageFow (ARGB32_Premultiplied). Stable address — never reassigned by value (only
    // overwritten in-place). The RGBA8888 format lets applyGLPendingUpdates() upload directly
    // via glTexSubImage2D without an extra convertToFormat() copy.
    QImage _cachedImage;

    // Single-shot coalescing timer: fires at most once per FOW_UPDATE_COALESCE_MS window.
    // Started by requestFowUpdate(); cancelled by flushPendingUpdate().
    QTimer* _updateCoalescer;

    void rebuildBrushStamp(const MapDraw& mapDraw);

    // Pre-rendered smooth-erase brush stamp (ARGB32_Premultiplied). Rebuilt when radius,
    // brushType, or fowColor changes. Applied via drawImage + CompositionMode_DestinationIn
    // in paintFoWPoint / paintFoWPoints, replacing per-call radial gradient rasterization
    // on the large _imageFow image.
    QImage _brushStamp;
    int _brushStampBrushType = -1;
    bool _brushStampSmooth = false;
    QRgb _brushStampFowColor = 0;

};

#endif // LAYERFOW_H
