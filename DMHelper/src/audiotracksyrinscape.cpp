#include "audiotracksyrinscape.h"
#include "dmconstants.h"
#include <QDesktopServices>
#include <QMessageBox>

AudioTrackSyrinscape::AudioTrackSyrinscape(const QString& trackName, const QUrl& trackUrl, QObject *parent) :
    AudioTrackUrl(trackName, trackUrl, parent)
{
}

int AudioTrackSyrinscape::getAudioType() const
{
    return DMHelper::AudioType_Syrinscape;
}

void AudioTrackSyrinscape::play()
{
#ifdef Q_OS_MAC
        QMessageBox::information(nullptr, QString("Syrinscape integration"), QString("Syrinscape 3rd party integration is unfortunately only supported on Windows at this time. Audio playback of Syrinscape audio will be integrated into DMHelper as soon as this is supported!"));
#else
        QDesktopServices::openUrl(getUrl());
#endif

    emit trackStarted(this);
}

void AudioTrackSyrinscape::stop()
{
    emit trackStopped(this);
}

void AudioTrackSyrinscape::setMute(bool mute)
{
    Q_UNUSED(mute);
    emit muteChanged(mute);
}

void AudioTrackSyrinscape::setVolume(int volume)
{
    Q_UNUSED(volume);
    emit volumeChanged(volume);
}

void AudioTrackSyrinscape::setRepeat(bool repeat)
{
    Q_UNUSED(repeat);
    emit repeatChanged(repeat);
}
