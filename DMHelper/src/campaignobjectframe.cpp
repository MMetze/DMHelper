#include "campaignobjectframe.h"

CampaignObjectFrame::CampaignObjectFrame(QWidget *parent) :
    QFrame(parent)
{
}

CampaignObjectFrame::~CampaignObjectFrame()
{
}

void CampaignObjectFrame::activateObject(CampaignObjectBase* object)
{
    Q_UNUSED(object);
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

void CampaignObjectFrame::setBackgroundColor(QColor color)
{
    Q_UNUSED(color);
}

void CampaignObjectFrame::reloadObject()
{
}
