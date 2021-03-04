#include "objectdispatcher.h"
#include "campaign.h"
#include "audiotrack.h"
#include "soundboardframe.h"
#include "networkcontroller.h"

ObjectDispatcher::ObjectDispatcher(QObject *parent) :
    QObject(parent),
    _campaign(nullptr),
    _soundboard(nullptr),
    _networkController(nullptr)
{
}

void ObjectDispatcher::setCampaign(Campaign* campaign)
{
    if(campaign == _campaign)
        return;

    _campaign = campaign;

    if(_campaign)
        connectAllObjects();
}

void ObjectDispatcher::setSoundboard(SoundboardFrame* soundboard)
{
    if(_soundboard == soundboard)
        return;

    _soundboard = soundboard;

    if(_soundboard)
        connectAllObjects();
}

void ObjectDispatcher::setNetworkController(NetworkController* networkController)
{
    if(_networkController == networkController)
        return;

    _networkController = networkController;

    if(_networkController)
        connectAllObjects();
}

void ObjectDispatcher::addObject(CampaignObjectBase* object)
{
    if(!object)
        return;

    if(object->getObjectType() != DMHelper::CampaignType_AudioTrack)
        return;

    AudioTrack* track = dynamic_cast<AudioTrack*>(object);
    if(_soundboard)
        _soundboard->addTrackToTree(track);

    if(_networkController)
    {
        connect(track, &AudioTrack::trackStarted, _networkController, &NetworkController::addTrack);
        connect(track, &AudioTrack::trackStopped, _networkController, &NetworkController::removeTrack);
    }
}

void ObjectDispatcher::connectAllObjects()
{
    if(!_campaign)
        return;

    QList<CampaignObjectBase*> tracks = _campaign->getChildObjectsByType(DMHelper::CampaignType_AudioTrack);
    for(CampaignObjectBase* trackObject : tracks)
    {
        addObject(trackObject);
    }
}
