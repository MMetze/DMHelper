#include "campaignobjectframe.h"
#include <QDebug>

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
    emit setPublishEnabled(false, false);
}

void CampaignObjectFrame::deactivateObject()
{
}

void CampaignObjectFrame::publishClicked(bool checked)
{
    Q_UNUSED(checked);
    qDebug() << "[CampaignObjectFrame] Unexpected base class call of publishClicked for " << this;
}

void CampaignObjectFrame::setRotation(int rotation)
{
    Q_UNUSED(rotation);
    qDebug() << "[CampaignObjectFrame] Unexpected base class call of setRotation for " << this;
}

void CampaignObjectFrame::setBackgroundColor(const QColor& color)
{
    Q_UNUSED(color);
    qDebug() << "[CampaignObjectFrame] Unexpected base class call of setBackgroundColor for " << this;
}

/*
void CampaignObjectFrame::reloadObject()
{
}
*/

void CampaignObjectFrame::editLayers()
{
    qDebug() << "[CampaignObjectFrame] Unexpected base class call of editLayers for " << this;
}
