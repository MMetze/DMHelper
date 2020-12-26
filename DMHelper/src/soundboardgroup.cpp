#include "soundboardgroup.h"

SoundboardGroup::SoundboardGroup(const QString& groupName, QObject *parent) :
    QObject(parent),
    _groupName(groupName),
    _tracks()
{
}

QString SoundboardGroup::getGroupName() const
{
    return _groupName;
}

QList<AudioTrack*> SoundboardGroup::getTracks() const
{
    return _tracks;
}

void SoundboardGroup::addTrack(AudioTrack* track)
{
    if(!track)
        return;

    _tracks.append(track);
}

void SoundboardGroup::removeTrack(AudioTrack* track)
{
    if(!track)
        return;

    _tracks.removeOne(track);
}

void SoundboardGroup::setGroupName(const QString& groupName)
{
    if(groupName != _groupName)
    {
        _groupName = groupName;
        emit groupNameChanged(_groupName);
    }
}
