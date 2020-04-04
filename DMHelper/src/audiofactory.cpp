#include "audiofactory.h"
#include "audiotrack.h"
#include "dmconstants.h"
#include <QDomElement>

AudioFactory::AudioFactory(QObject *parent) :
    ObjectFactory(parent)
{
}

CampaignObjectBase* AudioFactory::createObject(int objectType, int subType, const QString& objectName, bool isImport)
{
    Q_UNUSED(isImport);
    Q_UNUSED(subType);

    switch(objectType)
    {
        case DMHelper::CampaignType_AudioTrack:
            return new AudioTrack(objectName);
        default:
            return nullptr;
    }
}

CampaignObjectBase* AudioFactory::createObject(const QDomElement& element, bool isImport)
{
    Q_UNUSED(isImport);

    if(element.tagName() == QString("track"))
        return new AudioTrack();
    else
        return nullptr;
}
