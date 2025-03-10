#include "soundboardgroup.h"
#include "soundboardtrack.h"
#include "campaign.h"
#include "audiotrack.h"
#include <QDomElement>
#include <QUuid>

SoundboardGroup::SoundboardGroup(const QString& groupName, QObject *parent) :
    QObject(parent),
    _groupName(groupName),
    _volume(100),
    _mute(false),
    _tracks()
{
}

SoundboardGroup::SoundboardGroup(Campaign& campaign, const QDomElement& element, bool isImport) :
    QObject(nullptr),
    _groupName(),
    _volume(100),
    _mute(false),
    _tracks()
{
    internalPostProcessXML(campaign, element, isImport);
}

SoundboardGroup::~SoundboardGroup()
{
    qDeleteAll(_tracks);
}

void SoundboardGroup::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute("groupname", getGroupName());
    element.setAttribute("volume", getVolume());
    element.setAttribute("mute", getMute());

    for(SoundboardTrack* track : _tracks)
    {
        if((track) && (track->getTrack()))
        {
            QDomElement trackElement = doc.createElement("soundboardtrack");
            trackElement.setAttribute("trackID", track->getTrack()->getID().toString());
            trackElement.setAttribute("volume", track->getVolume());
            trackElement.setAttribute("mute", track->getMute());
            element.appendChild(trackElement);
        }
    }
}

QString SoundboardGroup::getGroupName() const
{
    return _groupName;
}

int SoundboardGroup::getVolume() const
{
    return _volume;
}

bool SoundboardGroup::getMute() const
{
    return _mute;
}

QList<SoundboardTrack*> SoundboardGroup::getTracks() const
{
    return _tracks;
}

void SoundboardGroup::addTrack(SoundboardTrack* track)
{
    if(!track)
        return;

    _tracks.append(track);
}

void SoundboardGroup::addTrack(AudioTrack* track)
{
    if(!track)
        return;

    _tracks.append(new SoundboardTrack(track, 100, false, this));
}

void SoundboardGroup::removeTrack(SoundboardTrack* track)
{
    if(!track)
        return;

    if(_tracks.removeOne(track))
        delete track;
}

void SoundboardGroup::setGroupName(const QString& groupName)
{
    if(groupName != _groupName)
    {
        _groupName = groupName;
        emit groupNameChanged(_groupName);
    }
}

void SoundboardGroup::setVolume(int volume)
{
    if(volume != _volume)
    {
        _volume = volume;
        emit volumeChanged(_volume);
    }
}

void SoundboardGroup::setMute(bool mute)
{
    if(mute != _mute)
    {
        _mute = mute;
        emit muteChanged(_mute);
    }
}

void SoundboardGroup::internalPostProcessXML(Campaign& campaign, const QDomElement& element, bool isImport)
{
    Q_UNUSED(isImport);

    _groupName = element.attribute("groupname");
    _volume = element.attribute("volume", QString("100")).toInt();
    _mute = static_cast<bool>(element.attribute("mute").toInt());

    QDomElement trackElement = element.firstChildElement("soundboardtrack");
    while(!trackElement.isNull())
    {
        QUuid trackId(trackElement.attribute("trackID"));
        AudioTrack* track = dynamic_cast<AudioTrack*>(campaign.getObjectById(trackId));
        if(track)
        {
            int volume = trackElement.attribute("volume", QString("100")).toInt();
            bool mute = static_cast<bool>(trackElement.attribute("mute").toInt());
            addTrack(new SoundboardTrack(track, volume, mute, this));
        }

        trackElement = trackElement.nextSiblingElement("soundboardtrack");
    }
}
