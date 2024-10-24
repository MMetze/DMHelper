#ifndef LAYERVIDEO_H
#define LAYERVIDEO_H

#include "layer.h"
#include <QImage>

//#define LAYERVIDEO_USE_OPENGL

class QGraphicsPixmapItem;

#ifdef LAYERVIDEO_USE_OPENGL
    class VideoPlayerGLPlayer;
    class VideoPlayerGLScreenshot;
#else
    class VideoPlayer;
    class VideoPlayerScreenshot;
    class PublishGLBattleBackground;
#endif

class LayerVideo : public Layer
{
    Q_OBJECT
public:
    explicit LayerVideo(const QString& name = QString(), const QString& filename = QString(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerVideo() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual bool hasAudio() const override;

    virtual QRectF boundingRect() const override;
    virtual QImage getLayerIcon() const override;
    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;

    // Local Layer Interface
    virtual void applyOrder(int order) override;
    virtual void applyLayerVisibleDM(bool layerVisible) override;
    virtual void applyLayerVisiblePlayer(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;
    virtual void applyPosition(const QPoint& position) override;
    virtual void applySize(const QSize& size) override;

    virtual QString getVideoFile() const;
    virtual bool getPlayAudio() const;
    virtual QImage getScreenshot() const;

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene* scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene) override;
    virtual void playerGLUninitialize() override;
    virtual bool playerGLUpdate() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix) override;
    virtual void playerGLResize(int w, int h) override;
    virtual bool playerIsInitialized() override;

    // Layer Specific Interface
    virtual void initialize(const QSize& sceneSize) override;
    virtual void uninitialize() override;
    virtual void playerGLSetUniforms(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix);
    virtual void setPlayAudio(bool playAudio);

signals:
    void updateProjectionMatrix();
    void screenshotAvailable();

protected slots:
    // Local Interface
    virtual void handleScreenshotReady(const QImage& image);
    virtual void requestScreenshot();
    virtual void clearScreenshot();

protected:
    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    // DM Window Methods
    virtual void updateImage(const QSize& size = QSize());
    void cleanupDM();

    // Player Window Methods
    void createPlayerObjectGL(PublishGLRenderer* renderer);
    void cleanupPlayerObject();
    void cleanupPlayer();

    // DM Window Members
    QGraphicsPixmapItem* _graphicsItem;

    // Player Window Members
#ifdef LAYERVIDEO_USE_OPENGL
    VideoPlayerGLPlayer* _videoGLPlayer;
    VideoPlayerGLScreenshot* _screenshot;
#else
    VideoPlayer* _videoPlayer;
    VideoPlayerScreenshot* _screenshot;
    PublishGLBattleBackground* _videoObject;
    QSize _playerSize;
#endif
    PublishGLScene* _scene;

    // Core contents
    QString _filename;
    bool _playAudio;
    QImage _layerScreenshot;
    QGraphicsScene* _dmScene;
};

#endif // LAYERVIDEO_H
