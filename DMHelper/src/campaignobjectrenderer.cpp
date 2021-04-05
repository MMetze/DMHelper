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
