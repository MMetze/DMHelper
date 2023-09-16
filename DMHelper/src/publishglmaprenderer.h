#ifndef PUBLISHGLMAPRENDERER_H
#define PUBLISHGLMAPRENDERER_H

#include "publishglrenderer.h"
#include "publishglscene.h"
#include <QColor>
#include <QImage>
#include <QMatrix4x4>
#include <QPixmap>

class Map;
class Party;
class PublishGLBattleBackground;
class PublishGLImage;
class Layer;
class QOpenGLFunctions;

class PublishGLMapRenderer : public PublishGLRenderer
{
    Q_OBJECT
public:
    PublishGLMapRenderer(Map* map, QObject *parent = nullptr);
    virtual ~PublishGLMapRenderer() override;

    virtual CampaignObjectBase* getObject() override;
    virtual QColor getBackgroundColor() override;

    // DMH OpenGL renderer calls
    virtual void rendererDeactivated() override;
    virtual bool deleteOnDeactivation() override;
    virtual QRect getScissorRect() override;

    virtual void setBackgroundColor(const QColor& color) override;

    // Standard OpenGL calls
    virtual void initializeGL() override;
    virtual void cleanupGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

public slots:
    // DMH OpenGL renderer calls
    virtual void updateProjectionMatrix() override;

    //virtual void setRotation(int rotation) override;

    //void setImage(const QImage& image);
    void distanceChanged();
    void fowChanged(const QImage& fow);
    void setCameraRect(const QRectF& cameraRect);
    void markerChanged();

protected:
    // Background overrides
    /*
    virtual void initializeBackground() = 0;
    virtual bool isBackgroundReady() = 0;
    virtual void resizeBackground(int w, int h) = 0;
    virtual void paintBackground(QOpenGLFunctions* functions) = 0;
    virtual QSizeF getBackgroundSize() = 0;
    virtual void updateBackground();
    */

    void createPartyToken();
    void createLineToken(const QSize& sceneSize);
    void createMarkerTokens(const QSize& sceneSize);

    virtual void updateFoW();
    virtual void updateContents();

    virtual void createShaders();
    void destroyShaders();

protected slots:
    void handlePartyChanged(Party* party);
    void handlePartyIconChanged(const QString& partyIcon);
    void handlePartyIconPosChanged(const QPoint& pos);
    void handleShowPartyChanged(bool showParty);
    void handlePartyScaleChanged(int partyScale);
    void layerAdded(Layer* layer);

protected:
    Map* _map;
    PublishGLScene _scene;
    QSize _targetSize;
    QColor _color;
    QMatrix4x4 _projectionMatrix;
    QRectF _cameraRect;
    QRect _scissorRect;
    bool _initialized;
    unsigned int _shaderProgramRGB;
    int _shaderModelMatrixRGB;
    int _shaderProjectionMatrixRGB;
    unsigned int _shaderProgramRGBA;
    int _shaderModelMatrixRGBA;
    int _shaderProjectionMatrixRGBA;
    int _shaderAlphaRGBA;
    unsigned int _shaderProgramRGBColor;
    int _shaderModelMatrixRGBColor;
    int _shaderProjectionMatrixRGBColor;
    int _shaderRGBColor;
//    unsigned int _shaderProgram;
//    int _shaderModelMatrix;
//    QImage _fowImage;
//    PublishGLBattleBackground* _fowObject;
    PublishGLImage* _partyToken;
    PublishGLImage* _lineImage;
    QList<PublishGLImage*> _markerTokens;

    bool _recreatePartyToken;
    bool _recreateLineToken;
    bool _recreateMarkers;
    bool _updateFow;
};

#endif // PUBLISHGLMAPRENDERER_H
