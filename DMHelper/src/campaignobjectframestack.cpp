#include "campaignobjectframestack.h"
#include "campaignobjectframe.h"
#include <QDebug>

CampaignObjectFrameStack::CampaignObjectFrameStack(QWidget *parent) :
    QStackedWidget(parent),
    _frames()
{
}

int CampaignObjectFrameStack::addFrame(int objectType, CampaignObjectFrame* newFrame)
{
    if(_frames.contains(objectType))
    {
        qDebug() << "[CampaignObjectFrameStack] ERROR: Attempted to add a duplicate object type: " << objectType << ", frame: " << newFrame;
        return -1;
    }

    int newIndex = indexOf(newFrame);
    if(newIndex == -1)
        newIndex = addWidget(newFrame);

    _frames.insert(objectType, newFrame);
    return newIndex;
}

bool CampaignObjectFrameStack::addFrames(QList<int> objectTypeList, CampaignObjectFrame* newFrame)
{
    bool success = true;
    for(int objectType : objectTypeList)
    {
        if(addFrame(objectType, newFrame) == -1)
            success = false;
    }

    return success;
}

CampaignObjectFrame* CampaignObjectFrameStack::setCurrentFrame(int objectType)
{
    if(!_frames.contains(objectType))
    {
        qDebug() << "[CampaignObjectFrameStack] ERROR: unable to activate widget for unknown type: " << objectType;
        return nullptr;
    }

    CampaignObjectFrame* newFrame = _frames.value(objectType);
    qDebug() << "[CampaignObjectFrameStack] Activating stacked widget frame from " << currentWidget() << " to " << newFrame << " for type " << objectType;

    setCurrentWidget(newFrame);
    return newFrame;
}

CampaignObjectFrame* CampaignObjectFrameStack::getCurrentFrame()
{
    return dynamic_cast<CampaignObjectFrame*>(currentWidget());
}


