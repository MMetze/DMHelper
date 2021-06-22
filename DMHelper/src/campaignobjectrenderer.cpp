#include "campaignobjectrenderer.h"

CampaignObjectRenderer::CampaignObjectRenderer(QObject *parent) :
    QObject(parent)
{
}

CampaignObjectRenderer::~CampaignObjectRenderer()
{
}

void CampaignObjectRenderer::rendering(bool render)
{
    if(render)
        startRendering();
    else
        stopRendering();
}

void CampaignObjectRenderer::publishWindowMouseDown(const QPointF& position)
{
    Q_UNUSED(position);
}

void CampaignObjectRenderer::publishWindowMouseMove(const QPointF& position)
{
    Q_UNUSED(position);
}

void CampaignObjectRenderer::publishWindowMouseRelease(const QPointF& position)
{
    Q_UNUSED(position);
}

