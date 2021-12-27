#ifndef PUBLISHGLMAPIMAGERENDERER_H
#define PUBLISHGLMAPIMAGERENDERER_H

#include "publishglrenderer.h"
#include <QColor>
#include <QImage>
#include <QMatrix4x4>

class Map;
class Party;
class BattleGLBackground;
class PublishGLImage;

class PublishGLMapImageRenderer : public PublishGLRenderer
{
    Q_OBJECT
public:
    PublishGLMapImageRenderer(Map* map, QObject *parent = nullptr);
    virtual ~PublishGLMapImageRenderer() override;

    virtual CampaignObjectBase* getObject() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;
    virtual bool deleteOnDeactivation() override;
    virtual void setBackgroundColor(const QColor& color) override;

    // Standard OpenGL calls
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

    const QImage& getImage() const;
    QColor getColor() const;

public slots:
    void setImage(const QImage& image);
    void distanceChanged();
    void fowChanged();
    void setCameraRect(const QRectF& cameraRect);

    void pointerToggled(bool enabled);
    void setPointerPosition(const QPointF& pos);
    void setPointerFileName(const QString& filename);

protected:
    void setOrthoProjection();
    void createPartyToken();
    void createLineToken(const QSize& sceneSize);
    void createMarkerTokens(const QSize& sceneSize);
    void evaluatePointer();
    QPixmap getPointerPixmap();

protected slots:
    void handlePartyChanged(Party* party);
    void handlePartyIconChanged(const QString& partyIcon);
    void handlePartyIconPosChanged(const QPoint& pos);
    void handleShowPartyChanged(bool showParty);
    void handlePartyScaleChanged(int partyScale);

private:
    Map* _map;
    QImage _image;
    QSize _targetSize;
    QColor _color;
    QMatrix4x4 _projectionMatrix;
    QRectF _cameraRect;
    QRect _scissorRect;
    bool _initialized;
    unsigned int _shaderProgram;
    int _shaderModelMatrix;
    BattleGLBackground* _backgroundObject;
    BattleGLBackground* _fowObject;
    PublishGLImage* _partyToken;
    PublishGLImage* _itemImage;
    PublishGLImage* _pointerImage;
    QList<PublishGLImage*> _markerTokens;

    bool _pointerActive;
    QPointF _pointerPos;
    QString _pointerFile;

    bool _recreatePartyToken;
    bool _recreateLineToken;
    bool _updateFow;
};

#endif // PUBLISHGLMAPIMAGERENDERER_H
