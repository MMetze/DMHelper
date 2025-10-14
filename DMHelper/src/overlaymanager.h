#ifndef OVERLAYMANAGER_H
#define OVERLAYMANAGER_H

#include <QObject>
#include <QSize>
#include <QList>

class Overlay;
class Campaign;

class OverlayManager : public QObject
{
    Q_OBJECT
public:
    OverlayManager(Campaign* campaign, QObject* parent = nullptr);
    virtual ~OverlayManager() override;

    void setCampaign(Campaign* camapaign);
    Campaign* getCampaign() const;

    bool isEmpty() const;

    QList<Overlay*> getOverlays();
    int getOverlayCount() const;
    int getOverlayIndex(Overlay* overlay);
    bool addOverlay(Overlay* overlay);
    bool removeOverlay(Overlay* overlay);
    bool moveOverlay(int from, int to);
    void clearOverlays();

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

    QList<Overlay*> _overlays;

};

#endif // OVERLAYMANAGER_H
