#include "audiotrackfile.h"
#include "dmconstants.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>

AudioTrackFile::AudioTrackFile(const QString& trackName, const QUrl& trackUrl, QObject *parent) :
    AudioTrackUrl(trackName, trackUrl, parent),
    _player(nullptr),
    _repeat(false)
{
}

int AudioTrackFile::getAudioType() const
{
    return DMHelper::AudioType_File;
}

bool AudioTrackFile::isPlaying() const
{
    return ((_player) && (_player->playbackState() == QMediaPlayer::PlayingState));
}

bool AudioTrackFile::isRepeat() const
{
    return ((_player) && (_player->loops() == QMediaPlayer::Infinite));
}

bool AudioTrackFile::isMuted() const
{
    return ((_player) && (_player->audioOutput()) && (_player->audioOutput()->isMuted()));
}

float AudioTrackFile::getVolume() const
{
    if((!_player) || (!_player->audioOutput()))
        return 0;

    return _player->audioOutput()->volume();
}

void AudioTrackFile::play()
{
    if(_player)
        return;

    QString fileString = getUrl().toString();
    if(!QFile::exists(fileString))
    {
        QMessageBox::critical(nullptr,
                              QString("DMHelper Audio Track File Not Found"),
                              QString("The audio track could not be found: ") + fileString);
        qDebug() << "[AudioTrackFile] Audio track file not found: " << fileString;
        return;
    }

    QFileInfo fileInfo(fileString);
    if(!fileInfo.isFile())
    {
        QMessageBox::critical(nullptr,
                              QString("DMHelper Audio Track File Not Valid"),
                              QString("The audio track isn't a file: ") + fileString);
        qDebug() << "[AudioTrackFile] Audio track file not a file: " << fileString;
        return;
    }

    fileString = fileInfo.canonicalFilePath();
    QUrl url = QUrl(fileString);
    url.setScheme(QString("file"));
    _player = new QMediaPlayer(this);
    _player->setLoops(_repeat ? QMediaPlayer::Infinite : 1);

    QAudioOutput audioOutput;
    _player->setAudioOutput(&audioOutput);

    connect(_player, &QMediaPlayer::durationChanged, this, &AudioTrackFile::handleDurationChanged);
    connect(_player, &QMediaPlayer::positionChanged, this, &AudioTrackFile::handlePositionChanged);

    _player->setSource(url);
    _player->play();
}

void AudioTrackFile::stop()
{
    if(!_player)
        return;

    _player->stop();

    _player->deleteLater();
    _player = nullptr;
}

void AudioTrackFile::setMute(bool mute)
{
    if((!_player) || (!_player->audioOutput()))
        return;

    _player->audioOutput()->setMuted(mute);
}

void AudioTrackFile::setVolume(float volume)
{
    if((!_player) || (!_player->audioOutput()))
        return;

    _player->audioOutput()->setVolume(volume);
}

void AudioTrackFile::setRepeat(bool repeat)
{
    if((!_player) || (_repeat != repeat))
        return;

    _player->setLoops(_repeat ? QMediaPlayer::Infinite : 1);
}

void AudioTrackFile::handleDurationChanged(qint64 position)
{
    emit trackLengthChanged(position / 1000);
}

void AudioTrackFile::handlePositionChanged(qint64 position)
{
    emit trackPositionChanged(position / 1000);
}

