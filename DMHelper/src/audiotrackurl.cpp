#include "audiotrackurl.h"

AudioTrackUrl::AudioTrackUrl(const QString& trackName, const QUrl& trackUrl, QObject *parent) :
    AudioTrack(trackName, parent),
    _url(trackUrl)
{
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

