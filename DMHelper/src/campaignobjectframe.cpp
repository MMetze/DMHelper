#include "campaignobjectframe.h"

CampaignObjectFrame::CampaignObjectFrame(QWidget *parent) :
    QFrame(parent)
{
}

CampaignObjectFrame::~CampaignObjectFrame()
{
}

void CampaignObjectFrame::activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer)
{
    Q_UNUSED(object);
    Q_UNUSED(currentRenderer);

    emit checkableChanged(false);
    emit setPublishEnabled(false);
}

void CampaignObjectFrame::deactivateObject()
{
}

void CampaignObjectFrame::publishClicked(bool checked)
{
    Q_UNUSED(checked);
}

void CampaignObjectFrame::setRotation(int rotation)
{
    Q_UNUSED(rotation);
}

void CampaignObjectFrame::setBackgroundColor(const QColor& color)
{
    Q_UNUSED(color);
}

/*
void CampaignObjectFrame::reloadObject()
{
}
*/

void CampaignObjectFrame::editLayers()
{
}
