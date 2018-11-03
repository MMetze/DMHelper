#include "audioplaybackframe.h"
#include "ui_audioplaybackframe.h"
#include "audiotrack.h"
#include <QFileDialog>
#include <QInputDialog>

AudioPlaybackFrame::AudioPlaybackFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::AudioPlaybackFrame),
    _currentDuration(0),
    _currentPosition(0),
    _currentVolume(100)
{
    ui->setupUi(this);

    trackChanged(nullptr);
    connect(ui->btnPlay, SIGNAL(clicked(bool)), this, SLOT(togglePlay(bool)));
    connect(ui->sliderVolume, SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));

    ui->sliderVolume->setValue(_currentVolume);
}

AudioPlaybackFrame::~AudioPlaybackFrame()
{
    delete ui;
}

void AudioPlaybackFrame::setDuration(qint64 duration)
{
    if(_currentDuration == duration)
        return;

    _currentDuration = duration;
    qlonglong seconds = duration / 1000;
    ui->lblLength->setText(QString("%1:%2").arg(seconds / 60).arg(seconds % 60, 2, 10, QChar('0')));
    ui->sliderPlayback->setSliderPosition(_currentDuration == 0 ? 0 : 100 * _currentPosition / _currentDuration);
}

void AudioPlaybackFrame::setPosition(qint64 position)
{
    if(_currentPosition == position)
        return;

    _currentPosition = position;
    qlonglong seconds = position / 1000;
    ui->lblPlayed->setText(QString("%1:%2").arg(seconds / 60).arg(seconds % 60, 2, 10, QChar('0')));
    ui->sliderPlayback->setSliderPosition(_currentDuration == 0 ? 0 : 100 * _currentPosition / _currentDuration);

    emit positionChanged(_currentPosition);
}

void AudioPlaybackFrame::setVolume(int volume)
{
    if(_currentVolume == volume)
        return;

    if(volume < 0) volume = 0;
    if(volume > 100) volume = 100;

    _currentVolume = volume;
    ui->sliderVolume->setValue(_currentVolume);
    emit volumeChanged(_currentVolume);
}

void AudioPlaybackFrame::trackChanged(AudioTrack* track)
{
    if(track == nullptr)
    {
        setPlayerEnabled(false);
        ui->lblCurrent->setText(QString("No track"));
        ui->lblPlayed->setText(QString("--:--"));
        ui->lblLength->setText(QString("--:--"));
    }
    else
    {
        setPlayerEnabled(true);
        ui->lblCurrent->setText(track->getName());
        ui->lblPlayed->setText(QString("0:00"));
        ui->lblLength->setText(QString("0:00"));
    }
}

void AudioPlaybackFrame::stateChanged(AudioPlayer::State state)
{
    switch(state)
    {
        case AudioPlayer::Playing:
            ui->btnPlay->setChecked(true);
            break;
        case AudioPlayer::Paused:
        case AudioPlayer::Stopped:
        default:
            ui->btnPlay->setChecked(false);
            break;
    }
}

void AudioPlaybackFrame::togglePlay(bool checked)
{
    if(checked)
        emit play();
    else
        emit pause();
}

void AudioPlaybackFrame::setPlayerEnabled(bool enabled)
{
    ui->sliderPlayback->setEnabled(enabled);
    ui->lblLength->setEnabled(enabled);
    ui->lblPlayed->setEnabled(enabled);
    ui->btnPlay->setEnabled(enabled);
}
