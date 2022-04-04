#include "audiotrackurl.h"

AudioTrackUrl::AudioTrackUrl(const QString& trackName, const QUrl& trackUrl, QObject *parent) :
    AudioTrack(trackName, parent),
    _url(trackUrl)
{
}

void AudioTrackUrl::copyValues(const CampaignObjectBase* other)
{
    const AudioTrackUrl* otherAudioTrackUrl = dynamic_cast<const AudioTrackUrl*>(other);
    if(!otherAudioTrackUrl)
        return;

    _url = otherAudioTrackUrl->_url;

    AudioTrack::copyValues(other);
}

QUrl AudioTrackUrl::getUrl() const
{
    return _url;
}

void AudioTrackUrl::setUrl(const QUrl& trackUrl)
{
    if(_url != trackUrl)
    {
        _url = trackUrl;
        emit changed();
        emit dirty();
    }
}

