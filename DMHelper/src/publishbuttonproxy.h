#ifndef PUBLISHBUTTONPROXY_H
#define PUBLISHBUTTONPROXY_H

#include "layer.h"
#include <QObject>
#include <QColor>

class PublishButtonRibbon;

class PublishButtonProxy : public QObject
{
    Q_OBJECT
public:
    explicit PublishButtonProxy(QObject *parent = nullptr);

    void addPublishButton(PublishButtonRibbon* publishButton);
    void removePublishButton(PublishButtonRibbon* publishButton);

    int getRotation() const;

public slots:
    void setPublishEnabled(bool enabled, bool layered);
    void setChecked(bool checked);
    void setCheckable(bool checkable);
    void setRotation(int rotation);
    void setColor(const QColor& color);
    void cancelPublish();
    void setPlayersWindow(bool checked);
    void setLayers(QList<Layer*> layers, int selected);

signals:
    void clicked(bool checked = false);
    void layerSelected(int selected);
    void layersClicked();
    void rotateCW();
    void rotateCCW();
    void rotationChanged(int rotation);
    void colorChanged(const QColor& color);
    void playersWindowClicked(bool checked);

    void sharePublishEnabled(bool enabled);
    void shareLayersEnabled(bool enabled);
    void shareSetChecked(bool checked);
    void shareSetCheckable(bool checkable);
    void shareSetRotation(int rotation);
    void shareSetColor(const QColor& color);
    void shareCancelPublish();
    void shareSetPlayersWindow(bool checked);
    void shareSetLayers(QList<Layer*> layers, int selected);

protected slots:
    void internalSetRotation(int rotation);

protected:
    int _rotation;
};

#endif // PUBLISHBUTTONPROXY_H
