#ifndef MAPFRAME_H
#define MAPFRAME_H

#include "campaignobjectframe.h"
#include <QGraphicsScene>
#include <QImage>
#include <QRubberBand>
#include "undofowpath.h"
#include "videoplayer.h"
#include "unselectedpixmap.h"

namespace Ui {
class MapFrame;
}

class MapFrameScene;
class Map;
class PublishGLRenderer;
class PublishGLMapRenderer;
class Party;
class MapMarkerGraphicsItem;
class UndoMarker;
class CameraRect;
class Layer;
class GridSizer;

class MapFrame : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit MapFrame(QWidget *parent = nullptr);
    virtual ~MapFrame() override;

    virtual void activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer) override;
    virtual void deactivateObject() override;

    void setMap(Map* map);

    void mapMarkerMoved(UndoMarker* marker);
    void activateMapMarker(UndoMarker* marker);

    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    QAction* getUndoAction(QObject* parent);
    QAction* getRedoAction(QObject* parent);

signals:
    void encounterSelected(QUuid id);

    void publishImage(QImage image);
    void windowClosed(MapFrame* mapFrame);
    void dirty();
    void showPublishWindow();

    void partyChanged(Party* party);
    void partyIconChanged(const QString& partyIcon);
    void showPartyChanged(bool showParty);
    void partyScaleChanged(int scale);

    void showDistanceChanged(bool show);
    void showFreeDistanceChanged(bool show);
    void distanceScaleChanged(int scale);
    void distanceChanged(const QString& distance);
    void distanceLineColorChanged(const QColor& color);
    void distanceLineTypeChanged(int lineType);
    void distanceLineWidthChanged(int lineWidth);

    void fowChanged(const QImage& fow);
    void cameraRectChanged(const QRectF& cameraRect);

    void showMarkersChanged(bool show);
    void markerChanged();

    void registerRenderer(PublishGLRenderer* renderer);
    void setLayers(QList<Layer*> layers, int selected);

    void mapEditChanged(bool enabled);
    void zoomSelectChanged(bool enabled);
    void brushModeSet(int brushMode);

    void cameraSelectToggled(bool enabled);
    void cameraEditToggled(bool enabled);

    void pointerToggled(bool enabled);
    void pointerPositionChanged(const QPointF& pos);
    void pointerFileNameChanged(const QString& filename);

    void publishCancelled();

public slots:
    void fillFoW();
    void resetFoW();
    void clearFoW();
    void undoPaint();

    void colorize();

    void setParty(Party* party);
    void setPartyIcon(const QString& partyIcon);
    void setShowParty(bool showParty);
    void setPartyScale(int partyScale);
    void setPartySelected(bool selected);
    void resizeGrid();

    void setShowMarkers(bool show);
    void addNewMarker();
    void addMarker(const QPointF& markerPosition);
    void editMapMarker(UndoMarker* marker);
    void deleteMapMarker(UndoMarker* marker);

    void setMapEdit(bool enabled);
    void setBrushMode(int brushMode);
    void brushSizeChanged(int size);

    void editMapFile();
    void zoomIn();
    void zoomOut();
    void zoomOne();
    void zoomFit();
    void zoomSelect(bool enabled);
    void zoomDelta(int delta);
    void centerWindow(const QPointF& position);
    void cancelSelect();

    void setErase(bool enabled);
    void setSmooth(bool enabled);

    void setDistance(bool enabled);
    void setFreeDistance(bool enabled);
    void setDistanceScale(int scale);
    void setDistanceLineColor(const QColor& color);
    void setDistanceLineType(int lineType);
    void setDistanceLineWidth(int lineWidth);

    void setCameraCouple();
    void setCameraMap();
    void setCameraVisible();
    void setCameraSelect(bool enabled);
    void setCameraEdit(bool enabled);

    void setPointerOn(bool enabled);
    void setPointerFile(const QString& filename);

    void setTargetLabelSize(const QSize& targetSize);
    void publishWindowMouseDown(const QPointF& position);
    void publishWindowMouseMove(const QPointF& position);
    void publishWindowMouseRelease(const QPointF& position);

    void targetResized(const QSize& newSize);
    void layerSelected(int selected);

    // Publish slots from CampaignObjectFrame
    virtual void publishClicked(bool checked) override;
    virtual void setRotation(int rotation) override;
    virtual void setBackgroundColor(const QColor& color) override;
    virtual void editLayers() override;

protected:
    void initializeMap();
    void uninitializeMap();

    void cleanupSelectionItems();

    virtual void hideEvent(QHideEvent * event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;

    bool editModeToggled(int editMode);
    void changeEditMode(int editMode, bool active);

    bool checkMapMove(QEvent* event);
    bool execEventFilter(QObject *obj, QEvent *event);
    bool execEventFilterSelectZoom(QObject *obj, QEvent *event);
    bool execEventFilterEditModeFoW(QObject *obj, QEvent *event);
    bool execEventFilterEditModeEdit(QObject *obj, QEvent *event);
    bool execEventFilterEditModeMove(QObject *obj, QEvent *event);
    bool execEventFilterEditModeDistance(QObject *obj, QEvent *event);
    bool execEventFilterEditModeFreeDistance(QObject *obj, QEvent *event);
    bool execEventFilterCameraSelect(QObject *obj, QEvent *event);
    bool execEventFilterCameraEdit(QObject *obj, QEvent *event);
    bool execEventFilterPointer(QObject *obj, QEvent *event);

    void cleanupBuffers();

protected slots:
    void setMapCursor();
    void drawEditCursor();
    void setScale(qreal s);
    void storeViewRect();
    void loadViewRect();
    void checkPartyUpdate();

    void rendererActivated(PublishGLMapRenderer* renderer);
    void rendererDeactivated();

    void handleMapMousePress(const QPointF& pos);
    void handleMapMouseMove(const QPointF& pos);
    void handleMapMouseRelease(const QPointF& pos);

    void handleActivateMapMarker();

    void handleItemChanged(QGraphicsItem* item);
    void handleSceneChanged(const QList<QRectF> &region);
    void handleMapSceneChanged();

private:
    bool convertPublishToScene(const QPointF& publishPosition, QPointF& scenePosition);
    void setCameraToView();
    QGraphicsItem* findTopObject(const QPoint &pos);
    QPixmap getPointerPixmap();

    Ui::MapFrame *ui;

    MapFrameScene* _scene;
    UnselectedPixmap* _partyIcon;
    CameraRect* _cameraRect;

    int _editMode;
    bool _erase;
    bool _smooth;
    int _brushMode;
    int _brushSize;
    bool _isPublishing;
    bool _isVideo;

    int _rotation;

    bool _spaceDown;
    bool _mapMoveMouseDown;
    bool _mouseDown;
    QPoint _mouseDownPos;
    UndoFowPath* _undoPath;

    QGraphicsLineItem* _distanceLine;
    MapDraw* _mapItem;
    QGraphicsPathItem* _distancePath;
    QGraphicsSimpleTextItem* _distanceText;
    QString _pointerFile;

    bool _publishMouseDown;
    QPointF _publishMouseDownPos;

    QRubberBand* _rubberBand;
    qreal _scale;
    GridSizer* _gridSizer;

    Map* _mapSource;
    PublishGLMapRenderer* _renderer;

    QSize _targetSize;
    QSize _targetLabelSize;
    QRect _publishRect;
    QImage _bwFoWImage;

    QUuid _activatedId;
};

#endif // MAPFRAME_H
