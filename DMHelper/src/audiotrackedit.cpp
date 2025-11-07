#include "audiotrackedit.h"
#include "ui_audiotrackedit.h"
#include "audiotrack.h"
#include <QTimer>
#include <QDebug>

AudioTrackEdit::AudioTrackEdit(QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::AudioTrackEdit),
    _track(nullptr)
{
    ui->setupUi(this);
}

AudioTrackEdit::~AudioTrackEdit()
{
    delete ui;
}

void AudioTrackEdit::activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer)
{
    Q_UNUSED(currentRenderer);

    AudioTrack* track = dynamic_cast<AudioTrack*>(object);
    if(!track)
        return;

    if(_track != nullptr)
    {
        qDebug() << "[AudioTrackEdit] ERROR: New audio track activated without deactivating the previous track object first!";
        deactivateObject();
    }

    setTrack(track);
}

void AudioTrackEdit::deactivateObject()
{
    if(!_track)
    {
        qDebug() << "[AudioTrackEdit] WARNING: Invalid (nullptr) track object deactivated!";
        return;
    }

    setTrack(nullptr);
}

void AudioTrackEdit::setTrack(AudioTrack* track)
{
    if(_track)
    {
        disconnect(_track, &AudioTrack::trackStatusChanged, this, &AudioTrackEdit::trackStatusChanged);
    }

    _track = track;

    if(_track)
    {
        emit trackTypeChanged(_track->getAudioType());
        emit trackStatusChanged(_track->getTrackStatus());
        emit repeatChanged(_track->isRepeat());
        emit muteChanged(_track->isMuted());
        emit volumeChanged(_track->getVolume());

        connect(_track, &AudioTrack::trackStatusChanged, this, &AudioTrackEdit::trackStatusChanged);
    }
}

/*
void AudioTrackEdit::setPlay(bool checked)
{
    if((!_track) || (checked == _track->isPlaying()))
        return;

    if(checked)
    {
        _track->play();

        if(_track->getAudioType() == DMHelper::AudioType_Syrinscape)
            QTimer::singleShot(500, this, &AudioTrackEdit::unclick);
    }
    else
    {
        _track->stop();
    }

    emit playChanged(checked);
}
*/

void AudioTrackEdit::play()
{
    if((!_track) || (_track->getTrackStatus() == AudioTrack::AudioTrackStatus_Play))
        return;

    _track->play();
    emit trackStatusChanged(AudioTrack::AudioTrackStatus_Play);
}

void AudioTrackEdit::pause()
{
    if((!_track) || (_track->getTrackStatus() != AudioTrack::AudioTrackStatus_Play))
        return;

    _track->pause();
    emit trackStatusChanged(AudioTrack::AudioTrackStatus_Pause);
}

void AudioTrackEdit::stop()
{
    if((!_track) || (_track->getTrackStatus() == AudioTrack::AudioTrackStatus_Stop))
        return;

    _track->stop();
    emit trackStatusChanged(AudioTrack::AudioTrackStatus_Stop);
}

void AudioTrackEdit::setRepeat(bool checked)
{
    if(_track)
        _track->setRepeat(checked);
}

void AudioTrackEdit::setMute(bool checked)
{
    if(_track)
        _track->setMute(checked);
}

void AudioTrackEdit::setVolume(float volume)
{
    if(_track)
        _track->setVolume(volume);
}

/*
void AudioTrackEdit::unclick()
{
    emit playChanged(false);
}
*/
