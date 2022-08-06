#include "publishbuttonproxy.h"
#include "publishbuttonribbon.h"

PublishButtonProxy::PublishButtonProxy(QObject *parent) :
    QObject(parent),
    _rotation(0)
{
    connect(this, SIGNAL(rotationChanged(int)), this, SLOT(internalSetRotation(int)));
    connect(this, SIGNAL(shareSetRotation(int)), this, SLOT(internalSetRotation(int)));
}

void PublishButtonProxy::addPublishButton(PublishButtonRibbon* publishButton)
{
    if(!publishButton)
        return;

    connect(publishButton, SIGNAL(clicked(bool)), this, SIGNAL(clicked(bool)));
    connect(publishButton, SIGNAL(clicked(bool)), this, SIGNAL(shareSetChecked(bool)));
    connect(publishButton, &PublishButtonRibbon::layersClicked, this, &PublishButtonProxy::layersClicked);
    connect(publishButton, SIGNAL(rotateCW()), this, SIGNAL(rotateCW()));
    connect(publishButton, SIGNAL(rotateCCW()), this, SIGNAL(rotateCCW()));
    connect(publishButton, SIGNAL(rotationChanged(int)), this, SIGNAL(rotationChanged(int)));
    connect(publishButton, SIGNAL(rotationChanged(int)), this, SIGNAL(shareSetRotation(int)));
    connect(publishButton, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(colorChanged(const QColor&)));
    connect(publishButton, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(shareSetColor(const QColor&)));
    connect(publishButton, SIGNAL(buttonColorChanged(const QColor&)), this, SIGNAL(shareSetColor(const QColor&)));
    connect(publishButton, SIGNAL(playersWindowClicked(bool)), this, SIGNAL(playersWindowClicked(bool)));
    connect(publishButton, SIGNAL(playersWindowClicked(bool)), this, SIGNAL(shareSetPlayersWindow(bool)));

    connect(this, SIGNAL(sharePublishEnabled(bool)), publishButton, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(shareSetChecked(bool)), publishButton, SLOT(setChecked(bool)));
    connect(this, SIGNAL(shareSetCheckable(bool)), publishButton, SLOT(setCheckable(bool)));
    connect(this, SIGNAL(shareSetRotation(int)), publishButton, SLOT(setRotation(int)));
    connect(this, SIGNAL(shareSetColor(const QColor&)), publishButton, SLOT(setColor(const QColor&)));
    connect(this, SIGNAL(shareCancelPublish()), publishButton, SLOT(cancelPublish()));
    connect(this, SIGNAL(shareSetPlayersWindow(bool)), publishButton, SLOT(setPlayersWindow(bool)));
}

void PublishButtonProxy::removePublishButton(PublishButtonRibbon* publishButton)
{
    if(!publishButton)
        return;

    disconnect(publishButton, nullptr, this, nullptr);
    disconnect(this, nullptr, publishButton, nullptr);
}

int PublishButtonProxy::getRotation() const
{
    return _rotation;
}

void PublishButtonProxy::setPublishEnabled(bool enabled)
{
    emit sharePublishEnabled(enabled);
}

void PublishButtonProxy::setChecked(bool checked)
{
    emit shareSetChecked(checked);
}

void PublishButtonProxy::setCheckable(bool checkable)
{
    Q_UNUSED(checkable);

    emit shareSetCheckable(true);
}

void PublishButtonProxy::setRotation(int rotation)
{
    emit shareSetRotation(rotation);
}

void PublishButtonProxy::setColor(const QColor& color)
{
    emit shareSetColor(color);
}

void PublishButtonProxy::cancelPublish()
{
    emit shareCancelPublish();
}

void PublishButtonProxy::setPlayersWindow(bool checked)
{
    emit shareSetPlayersWindow(checked);
}

void PublishButtonProxy::internalSetRotation(int rotation)
{
    _rotation = rotation;
}
