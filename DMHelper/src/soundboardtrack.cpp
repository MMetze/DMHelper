    #include "soundboardtrack.h"
#include "ui_soundboardtrack.h"
#include "audiotrack.h"
#include "audioplayer.h"
#include "dmconstants.h"
#include <QPainter>
#include <QTimer>
#include <QDebug>

SoundboardTrack::SoundboardTrack(AudioTrack* track, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SoundboardTrack),
    _track(nullptr),
    _localMute(false),
    _currentMute(false)
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

    /*
    QPixmap iconScaled(":/img/data/icon_music.png");
    if(!iconScaled.isNull())
    {
        QPixmap finalPix = iconScaled;
        QPainter painter(&finalPix);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(iconScaled.rect(), QColor(115, 18, 0));
        painter.end();
        ui->btnPlay->setIcon(finalPix);
    }*/

    connect(ui->btnMute, &QAbstractButton::clicked, this, &SoundboardTrack::toggleMute);
    connect(ui->btnPlay, &QAbstractButton::toggled, this, &SoundboardTrack::togglePlay);
}

SoundboardTrack::~SoundboardTrack()
{
    delete ui;
}

void SoundboardTrack::setTrack(AudioTrack* track)
{
    if(_track)
    {
        disconnect(this, &SoundboardTrack::play, _track, &AudioTrack::play);
        disconnect(this, &SoundboardTrack::stop, _track, &AudioTrack::stop);
        disconnect(this, &SoundboardTrack::muteChanged, _track, &AudioTrack::setMute);
        disconnect(ui->slideVolume, &QAbstractSlider::valueChanged, _track, &AudioTrack::setVolume);
    }

    if((track) && (_track != track))
    {
        _track = track;
        ui->lblName->setText(_track->getName());
        setToolTip(_track->getUrl().toDisplayString());
        connect(this, &SoundboardTrack::play, _track, &AudioTrack::play);
        connect(this, &SoundboardTrack::stop, _track, &AudioTrack::stop);
        connect(this, &SoundboardTrack::muteChanged, _track, &AudioTrack::setMute);
        connect(ui->slideVolume, &QAbstractSlider::valueChanged, _track, &AudioTrack::setVolume);
    }
}

AudioTrack* SoundboardTrack::getTrack() const
{
    return _track;
}

void SoundboardTrack::setMute(bool mute)
{
    setCurrentMute(mute);
    _localMute = mute;
}

void SoundboardTrack::parentMuteChanged(bool mute)
{
    setCurrentMute(mute || _localMute);

    /*
    if(mute)
    {
        setCurrentMute(true);
    }
    else
    {
        setCurrentMute(_localMute);
    }
    */
}

void SoundboardTrack::togglePlay(bool checked)
{
    if(checked)
    {
        ui->btnPlay->setStyleSheet("background-color: rgb(115, 18, 0); border: 5px solid rgb(115, 18, 0);");
        /*
        QPixmap iconScaled(":/img/data/icon_music.png");
        if(!iconScaled.isNull())
        {
            QPixmap finalPix = iconScaled;
            QPainter painter(&finalPix);
            painter.setCompositionMode(QPainter::CompositionMode_SourceOut);
            painter.fillRect(iconScaled.rect(), QColor(115, 18, 0));
            painter.end();
            ui->btnPlay->setIcon(finalPix);
        }
        */

        if(_currentMute)
            ui->btnMute->click();

        emit play(nullptr);

        if((_track) && (_track->getAudioType() == DMHelper::AudioType_Syrinscape))
            QTimer::singleShot(500, ui->btnPlay, &QAbstractButton::click);
    }
    else
    {
        ui->btnPlay->setStyleSheet("");
        /*
        QPixmap iconScaled(":/img/data/icon_music.png");
        if(!iconScaled.isNull())
        {
            QPixmap finalPix = iconScaled;
            QPainter painter(&finalPix);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(iconScaled.rect(), QColor(115, 18, 0));
            painter.end();
            ui->btnPlay->setIcon(finalPix);
        }
        */
        emit stop();
    }
}

void SoundboardTrack::toggleMute()
{
    bool newMute = ui->btnMute->isChecked();
//    emit muteChanged(newMute);
    setCurrentMute(newMute);
    _localMute = newMute;
}

void SoundboardTrack::setCurrentMute(bool mute)
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
