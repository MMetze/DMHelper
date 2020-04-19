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
}

void CampaignObjectFrame::deactivateObject()
{
}

bool CampaignObjectFrame::isAnimated()
{
    return false;
}
