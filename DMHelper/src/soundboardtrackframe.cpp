#include "soundboardtrackframe.h"
#include "ui_soundboardtrackframe.h"
#include "audiotrack.h"
#include "audioplayer.h"
#include "dmconstants.h"
#include <QPainter>
#include <QTimer>
#include <QDebug>

SoundboardTrackFrame::SoundboardTrackFrame(AudioTrack* track, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SoundboardTrackFrame),
    _track(nullptr),
    _localMute(false),
    _currentMute(false),
    _trackLength("0:00"),
    _trackPosition("0:00")
{
    ui->setupUi(this);
    setTrack(track);

    if(_track)
    {
        if(_track->getAudioType() == DMHelper::AudioType_Syrinscape)
        {
            ui->btnPlay->setIcon(QIcon(QString(":/img/data/icon_syrinscape.png")));
            ui->btnRepeat->setEnabled(false);
            ui->btnMute->setEnabled(false);
            ui->slideVolume->setEnabled(false);
            ui->lblProgress->setText(QString("--:-- / --:--"));
            ui->lblProgress->setEnabled(false);
        }
        else if(_track->getAudioType() == DMHelper::AudioType_Youtube)
        {
            ui->btnPlay->setIcon(QIcon(QString(":/img/data/icon_playerswindow.png")));
        }
    }

    connect(ui->btnMute, &QAbstractButton::clicked, this, &SoundboardTrackFrame::toggleMute);
    connect(ui->btnPlay, &QAbstractButton::toggled, this, &SoundboardTrackFrame::togglePlay);
    connect(ui->btnRepeat, &QAbstractButton::toggled, this, &SoundboardTrackFrame::repeatChanged);
    connect(ui->btnRemove, &QAbstractButton::clicked, this, &SoundboardTrackFrame::handleRemove);
    connect(track, &AudioTrack::trackLengthChanged, this, &SoundboardTrackFrame::setTrackLength);
    connect(track, &AudioTrack::trackPositionChanged, this, &SoundboardTrackFrame::setTrackPosition);
}

SoundboardTrackFrame::~SoundboardTrackFrame()
{
    delete ui;
}

void SoundboardTrackFrame::setTrack(AudioTrack* track)
{
    if(_track)
    {
        disconnect(this, &SoundboardTrackFrame::play, _track, &AudioTrack::play);
        disconnect(this, &SoundboardTrackFrame::stop, _track, &AudioTrack::stop);
        disconnect(this, &SoundboardTrackFrame::muteChanged, _track, &AudioTrack::setMute);
        disconnect(ui->slideVolume, &QAbstractSlider::valueChanged, _track, &AudioTrack::setVolume);
        disconnect(_track, &AudioTrack::destroyed, this, &SoundboardTrackFrame::handleRemove);
    }

    if((track) && (_track != track))
    {
        _track = track;
        ui->lblName->setText(_track->getName());
        setToolTip(_track->getUrl().toDisplayString());
        connect(this, &SoundboardTrackFrame::play, _track, &AudioTrack::play);
        connect(this, &SoundboardTrackFrame::stop, _track, &AudioTrack::stop);
        connect(this, &SoundboardTrackFrame::muteChanged, _track, &AudioTrack::setMute);
        connect(ui->slideVolume, &QAbstractSlider::valueChanged, _track, &AudioTrack::setVolume);
        connect(_track, &AudioTrack::destroyed, this, &SoundboardTrackFrame::handleRemove);
    }
}

AudioTrack* SoundboardTrackFrame::getTrack() const
{
    return _track;
}

void SoundboardTrackFrame::setMute(bool mute)
{
    setCurrentMute(mute);
    _localMute = mute;
}

void SoundboardTrackFrame::parentMuteChanged(bool mute)
{
    setCurrentMute(mute || _localMute);
}

void SoundboardTrackFrame::setTrackLength(int trackLength)
{
    _trackLength = QString("%1:%2").arg(QString::number(trackLength / 60), 2, QChar('0')).arg(QString::number(trackLength % 60), 2, QChar('0'));
    updateProgress();
}

void SoundboardTrackFrame::setTrackPosition(int trackPosition)
{
    _trackPosition = QString("%1:%2").arg(QString::number(trackPosition / 60), 2, QChar('0')).arg(QString::number(trackPosition % 60), 2, QChar('0'));
    updateProgress();
}

void SoundboardTrackFrame::setRepeat(bool repeat)
{
    ui->btnRepeat->setChecked(repeat);
}

void SoundboardTrackFrame::togglePlay(bool checked)
{
    if(checked)
    {
        ui->btnPlay->setStyleSheet("background-color: rgb(115, 18, 0); border: 5px solid rgb(115, 18, 0);");

        if(_currentMute)
            ui->btnMute->click();

        emit play(nullptr);

        if((_track) && (_track->getAudioType() == DMHelper::AudioType_Syrinscape))
            QTimer::singleShot(500, ui->btnPlay, &QAbstractButton::click);
    }
    else
    {
        ui->btnPlay->setStyleSheet("");
        emit stop();
    }
}

void SoundboardTrackFrame::toggleMute()
{
    bool newMute = ui->btnMute->isChecked();
    setCurrentMute(newMute);
    _localMute = newMute;
}

void SoundboardTrackFrame::setCurrentMute(bool mute)
{
    if(_currentMute != mute)
    {
        emit muteChanged(mute);
        _currentMute = mute;
        ui->btnMute->setIcon(mute ? QIcon(QPixmap(":/img/data/icon_volumeoff.png")) : QIcon(QPixmap(":/img/data/icon_volumeon.png")));
        if(_track->getAudioType() != DMHelper::AudioType_Syrinscape)
            ui->slideVolume->setEnabled(!mute);
        ui->btnMute->setChecked(mute);
    }
}

void SoundboardTrackFrame::updateProgress()
{
    ui->lblProgress->setText(_trackPosition +  QString(" / ") + _trackLength);
}

void SoundboardTrackFrame::handleRemove()
{
    if(_track)
        emit removeTrack(_track);
}
