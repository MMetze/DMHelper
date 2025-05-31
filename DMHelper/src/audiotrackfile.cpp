#include "audiotrackfile.h"
#include "dmconstants.h"
#include <QAudioOutput>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QDomElement>
#include <QDebug>

AudioTrackFile::AudioTrackFile(const QString& trackName, const QUrl& trackUrl, QObject *parent) :
    AudioTrackUrl(trackName, trackUrl, parent),
    _player(nullptr),
    _repeat(true),
    _volume(1.0),
    _mute(false)
{
}

void AudioTrackFile::inputXML(const QDomElement &element, bool isImport)
{
    _repeat = static_cast<bool>(element.attribute("repeat", QString::number(1)).toInt());
    _volume = static_cast<float>(element.attribute("volume", QString::number(100)).toInt()) / 100.f;
    _mute = static_cast<bool>(element.attribute("mute", QString::number(0)).toInt());

    AudioTrackUrl::inputXML(element, isImport);
}

void AudioTrackFile::copyValues(const CampaignObjectBase* other)
{
    const AudioTrackFile* otherAudioTrack = dynamic_cast<const AudioTrackFile*>(other);
    if(!otherAudioTrack)
        return;

    _repeat = otherAudioTrack->_repeat;
    _volume = otherAudioTrack->_volume;
    _mute = otherAudioTrack->_mute;

    AudioTrackUrl::copyValues(other);
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
    return _repeat;
}

bool AudioTrackFile::isMuted() const
{
    return _mute;
}

float AudioTrackFile::getVolume() const
{
    return _volume;
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

    QAudioOutput* audioOutput = new QAudioOutput;
    _player->setAudioOutput(audioOutput);

    connect(_player, &QMediaPlayer::durationChanged, this, &AudioTrackFile::handleDurationChanged);
    connect(_player, &QMediaPlayer::positionChanged, this, &AudioTrackFile::handlePositionChanged);
    connect(_player, &QMediaPlayer::errorOccurred, this, &AudioTrackFile::handleErrorOccurred);

    if(_mute)
        _player->audioOutput()->setMuted(_mute);
    _player->audioOutput()->setVolume(_volume);
    _player->setLoops(_repeat ? QMediaPlayer::Infinite : 1);

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
    if(_mute == mute)
        return;

    _mute = mute;
    if((_player) && (_player->audioOutput()))
        _player->audioOutput()->setMuted(mute);
    emit dirty();
}

void AudioTrackFile::setVolume(float volume)
{
    if(_volume == volume)
        return;

    _volume = volume;
    if((_player) && (_player->audioOutput()))
        _player->audioOutput()->setVolume(volume);
    emit dirty();
}

void AudioTrackFile::setRepeat(bool repeat)
{
    if(_repeat == repeat)
        return;

    _repeat = repeat;
    if(_player)
        _player->setLoops(_repeat ? QMediaPlayer::Infinite : 1);
    emit dirty();
}

void AudioTrackFile::handleDurationChanged(qint64 position)
{
    emit trackLengthChanged(position / 1000);
}

void AudioTrackFile::handlePositionChanged(qint64 position)
{
    emit trackPositionChanged(position / 1000);
}

void AudioTrackFile::handleErrorOccurred(QMediaPlayer::Error error, const QString &errorString)
{
    qDebug() << "[AudioTrackFile] ERROR: " << error << ", " << errorString;
}

void AudioTrackFile::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    if(_mute)
        element.setAttribute("mute", _mute);

    if(!_repeat)
        element.setAttribute("repeat", _repeat);

    if(_volume < 1.0)
        element.setAttribute("volume", static_cast<int>(_volume * 100.f));

    AudioTrackUrl::internalOutputXML(doc, element, targetDirectory, isExport);
}
