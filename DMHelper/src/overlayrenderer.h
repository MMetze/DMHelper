#ifndef OVERLAYRENDERER_H
#define OVERLAYRENDERER_H

#include <QObject>
#include <QSize>
#include <QList>

class Overlay;
class Campaign;
class QDomElement;
class QDomDocument;
class QDir;

class OverlayRenderer : public QObject
{
    Q_OBJECT
public:
    OverlayRenderer(Campaign* campaign, QObject* parent = nullptr);
    virtual ~OverlayRenderer() override;

    void setCampaign(Campaign* camapaign);
    Campaign* getCampaign() const;

    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();

signals:
    void updateWindow();

private:
    Campaign* _campaign;

    QSize _targetSize;
    int _shaderProgramRGB;
    int _shaderModelMatrixRGB;
    int _shaderProjectionMatrixRGB;
};

#endif // OVERLAYRENDERER_H
