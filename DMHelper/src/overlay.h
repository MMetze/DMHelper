#ifndef OVERLAY_H
#define OVERLAY_H

#include "popup.h"
#include <QSize>
#include <QOpenGLFunctions>

class Campaign;
class QDomElement;
class QDomDocument;
class QDir;

class Overlay : public Popup
{
    Q_OBJECT
public:
    explicit Overlay(const QString& name = QString(), QObject *parent = nullptr);

    // From Popup
    virtual bool isDMOnly() const override;

    // Local interface
    virtual void inputXML(const QDomElement &element);
    QDomElement outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory);

    virtual int getOverlayType() const = 0;
    virtual bool isInitialized() const;

    virtual bool isVisible() const;
    virtual qreal getScale() const;
    virtual int getOpacity() const;
    virtual QSize getSize() const;

    void setCampaign(Campaign* campaign);
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix, int yOffset);

public slots:
    virtual void recreateContents();
    virtual void updateContents();

    virtual void setVisible(bool visible);
    virtual void setScale(qreal scale);
    virtual void setOpacity(int opacity);

    virtual void setX(int x) = 0;
    virtual void setY(int y) = 0;

protected:
    // Local interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory);
    virtual void doSetCampaign(Campaign* campaign);
    virtual void doInitializeGL();
    virtual void doResizeGL(int w, int h);
    virtual void doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix);

    virtual void createContentsGL() = 0;
    virtual void updateContentsGL();

    QImage textToImage(const QString& text);

    // Overlay data
    bool _visible;
    qreal _scale;
    int _opacity;

    // Additional information
    Campaign* _campaign;
    bool _recreateContents;
    bool _updateContents;
    bool _initialized;
};

#endif // OVERLAY_H
