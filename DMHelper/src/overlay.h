#ifndef OVERLAY_H
#define OVERLAY_H

#include "dmhobjectbase.h"
#include <QSize>
#include <QOpenGLFunctions>

class Campaign;

class Overlay : public DMHObjectBase
{
    Q_OBJECT
public:
    explicit Overlay(QObject *parent = nullptr);

    // DMHObjectBase
    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &parentElement, QDir& targetDirectory, bool isExport) override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void postProcessXML(Campaign* campaign, const QDomElement &element, bool isImport);

    // Local interface
    virtual int getOverlayType() const = 0;
    virtual bool isInitialized() const;

    virtual bool isVisible() const;
    virtual qreal getScale() const;
    virtual int getOpacity() const;

    void setCampaign(Campaign* campaign);
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix);

public slots:
    virtual void recreateContents();
    virtual void updateContents();

    virtual void setVisible(bool visible);
    virtual void setScale(qreal scale);
    virtual void setOpacity(int opacity);

signals:
    void dirty();
    void triggerUpdate();

protected:
    // DMHObjectBase
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    // Local interface
    virtual void doSetCampaign(Campaign* campaign);
    virtual void doInitializeGL();
    virtual void doResizeGL(int w, int h);
    virtual void doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix);

    virtual void createContentsGL() = 0;
    virtual void updateContentsGL();
    virtual void updateContentsScale(int w, int h);

    // Overlay data
    bool _visible;
    qreal _scale;
    int _opacity;

    // Additional information
    Campaign* _campaign;
    bool _recreateContents;
    bool _updateContents;
};

#endif // OVERLAY_H
