#ifndef MAPFRAME_H
#define MAPFRAME_H

#include "campaignobjectframe.h"
#include <QGraphicsScene>
#include <QFileInfo>
#include <QImage>
#include <QRubberBand>
#include "undopath.h"
#include "videoplayer.h"
#include "unselectedpixmap.h"

namespace Ui {
class MapFrame;
}

class MapFrameScene;
class Map;
class PublishGLMapRenderer;
class PublishGLRenderer;
class Party;
class MapMarkerGraphicsItem;
class UndoMarker;

class MapFrame : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit MapFrame(QWidget *parent = nullptr);
    virtual ~MapFrame() override;

    virtual void activateObject(CampaignObjectBase* object) override;
    virtual void deactivateObject() override;

    void setMap(Map* map);

    void mapMarkerMoved(int markerId);
    void activateMapMarker(int markerId);

    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    QAction* getUndoAction(QObject* parent);
    QAction* getRedoAction(QObject* parent);

signals:
    void encounterSelected(QUuid id);

    void publishImage(QImage image);
    void openPreview();
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

    void showMarkersChanged(bool show);

    void animationStarted();
    void animateImage(QImage img);
    void registerRenderer(PublishGLRenderer* renderer);

    void mapEditChanged(bool enabled);
    void zoomSelectChanged(bool enabled);
    void brushModeSet(int brushMode);

    void publishCancelled();

public slots:
    void updateFoW();
    void fillFoW();
    void resetFoW();
    void clearFoW();
    void undoPaint();
    void clear();

    void colorize();

    void cancelPublish();

    void setParty(Party* party);
    void setPartyIcon(const QString& partyIcon);
    void setShowParty(bool showParty);
    void setPartyScale(int partyScale);
    void setPartySelected(bool selected);

    void setShowMarkers(bool show);
    void addNewMarker();
    void addMarker(const QPointF& markerPosition);
    void createMapMarker(UndoMarker* undoEntry, MapMarker* marker);
    void editMapMarker(int markerId);
    void deleteMapMarker(int markerId);

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

    void setPublishZoom(bool enabled);
    void setPublishVisible(bool enabled);

    void setTargetLabelSize(const QSize& targetSize);
    void publishWindowMouseDown(const QPointF& position);
    void publishWindowMouseMove(const QPointF& position);
    void publishWindowMouseRelease(const QPointF& position);

    void targetResized(const QSize& newSize);

    // Publish slots from CampaignObjectFrame
    virtual void publishClicked(bool checked) override;
    virtual void setRotation(int rotation) override;

protected:
    void initializeFoW();
    void uninitializeFoW();

    void createMarkerItems();
    void cleanupMarkerItems();
    void cleanupSelectionItems();

    virtual void hideEvent(QHideEvent * event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void timerEvent(QTimerEvent *event) override;
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

    void startPublishTimer();
    void stopPublishTimer();

    void createVideoPlayer(bool dmPlayer);
    void cleanupBuffers();

protected slots:
    void setMapCursor();
    void drawEditCursor();
    void rotatePublish();
    void setScale(qreal s);
    void storeViewRect();
    void loadViewRect();
    void resetPublishFoW();
    void checkPartyUpdate();

    void handleScreenshotReady(const QImage& image);
    void rendererDeactivated();

    void handleMapMousePress(const QPointF& pos);
    void handleMapMouseMove(const QPointF& pos);
    void handleMapMouseRelease(const QPointF& pos);

    void handleActivateMapMarker();

    void handleSceneChanged(const QList<QRectF> &region);

private:
    bool convertPublishToScene(const QPointF& publishPosition, QPointF& scenePosition);
    void setBackgroundPixmap(const QPixmap& pixmap);

    Ui::MapFrame *ui;

    MapFrameScene* _scene;
    QGraphicsPixmapItem* _backgroundImage;
    //QGraphicsPixmapItem* _backgroundVideo;
    QGraphicsPixmapItem* _fow;
    UnselectedPixmap* _partyIcon;

    int _editMode;
    bool _erase;
    bool _smooth;
    int _brushMode;
    int _brushSize;
    bool _publishZoom;
    bool _publishVisible;
    bool _isPublishing;
    bool _isVideo;

    int _rotation;

    bool _spaceDown;
    bool _mapMoveMouseDown;
    bool _mouseDown;
    QPoint _mouseDownPos;
    UndoPath* _undoPath;

    QGraphicsLineItem* _distanceLine;
    QGraphicsPathItem* _distancePath;
    QGraphicsSimpleTextItem* _distanceText;

    bool _publishMouseDown;
    QPointF _publishMouseDownPos;

    QRubberBand* _rubberBand;
    qreal _scale;

    Map* _mapSource;
    PublishGLMapRenderer* _renderer;

    int _timerId;
    VideoPlayer* _videoPlayer;
    QSize _targetSize;
    QSize _targetLabelSize;
    QRect _publishRect;
    QImage _bwFoWImage;

    QUuid _activatedId;
};

#endif // MAPFRAME_H
