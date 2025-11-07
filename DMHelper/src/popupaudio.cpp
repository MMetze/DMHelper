#include "popupaudio.h"
#include "audiotrackfile.h"
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QBoxLayout>

PopupAudio::PopupAudio(AudioTrackFile* track, QObject *parent) :
    Popup{parent},
    _track(track),
    _btnPlay(nullptr),
    _btnMute(nullptr),
    _btnRepeat(nullptr)
{
}

PopupAudio::~PopupAudio()
{
    clearButtons();
}

void PopupAudio::prepareFrame(QBoxLayout* frameLayout, int insertIndex)
{
    if((!_track) || (!frameLayout) || (_btnPlay) || (_btnMute) || (_btnRepeat) || (insertIndex < 0))
        return;

    QLineEdit* edtTrackName = new QLineEdit();
    edtTrackName->setText(_track->getName());
    edtTrackName->setCursorPosition(0);
    edtTrackName->setReadOnly(true);

    _btnPlay = new QPushButton(QIcon(":/img/data/icon_play.png"), QString());
    _btnPlay->setCheckable(true);
    _btnPlay->setChecked(_track->isPlaying());
    _btnPlay->setIcon(_track->isPlaying()? QIcon(":/img/data/icon_pause.png") : QIcon(":/img/data/icon_play.png"));
    connect(_btnPlay, &QPushButton::toggled, this, &PopupAudio::playButtonToggled);
    connect(_track, &AudioTrack::trackStatusChanged, this, [this](AudioTrack::AudioTrackStatus newState)
            {
                Q_UNUSED(newState);
                if(_btnPlay)
                {
                    _btnPlay->setChecked(_track->isPlaying());
                    _btnPlay->setIcon(_track->isPlaying() ? QIcon(":/img/data/icon_pause.png") : QIcon(":/img/data/icon_play.png"));
                }
            });

    QPushButton* btnStop = new QPushButton(QIcon(":/img/data/icon_stop.png"), QString());
    connect(btnStop, &QPushButton::clicked, _track, &AudioTrackFile::stop);

    _btnMute = new QPushButton(QIcon(":/img/data/icon_volumeon.png"), QString());
    _btnMute->setCheckable(true);
    _btnMute->setChecked(_track->isMuted());
    _btnMute->setIcon(_track->isMuted()? QIcon(":/img/data/icon_volumeoff.png") : QIcon(":/img/data/icon_volumeon.png"));
    connect(_btnMute, &QPushButton::toggled, this, &PopupAudio::muteButtonToggled);
    connect(_track, &AudioTrackFile::trackMuteChanged, this, [this](bool mute)
            {
                if(_btnMute)
                {
                    _btnMute->setChecked(mute);
                    _btnMute->setIcon(mute ? QIcon(":/img/data/icon_volumeoff.png") : QIcon(":/img/data/icon_volumeon.png"));
                }
            });

    _btnRepeat = new QPushButton(QIcon(":/img/data/icon_redo.png"), QString());
    _btnRepeat->setCheckable(true);
    _btnRepeat->setChecked(_track->isRepeat());
    connect(_btnRepeat, &QPushButton::toggled, _track, &AudioTrackFile::setRepeat);
    connect(_track, &AudioTrackFile::trackRepeatChanged, _btnRepeat, &QPushButton::setChecked);

    frameLayout->insertWidget(insertIndex, _btnRepeat);
    frameLayout->insertWidget(insertIndex, _btnMute);
    frameLayout->insertWidget(insertIndex, btnStop);
    frameLayout->insertWidget(insertIndex, _btnPlay);
    frameLayout->insertWidget(insertIndex, edtTrackName);

    connect(_btnPlay, &QPushButton::destroyed, this, &PopupAudio::clearButtons);
    connect(_btnMute, &QPushButton::destroyed, this, &PopupAudio::clearButtons);
    connect(_btnRepeat, &QPushButton::destroyed, this, &PopupAudio::clearButtons);
}

AudioTrackFile* PopupAudio::getTrack() const
{
    return _track;
}

QPushButton* PopupAudio::getPlayButton(QBoxLayout* frameLayout, int insertIndex)
{
    Q_UNUSED(frameLayout);
    Q_UNUSED(insertIndex);

    return _btnPlay;
}

void PopupAudio::playButtonToggled(bool checked)
{
    if((!_btnPlay) || (!_track) || (checked == _track->isPlaying()))
        return;

    if(checked)
        _track->play();
    else
        _track->pause();

    _btnPlay->setIcon(checked ? QIcon(":/img/data/icon_pause.png") : QIcon(":/img/data/icon_play.png"));
}

void PopupAudio::muteButtonToggled(bool checked)
{
    if((!_btnMute) || (!_track) || (checked == _track->isMuted()))
        return;

    _track->setMute(checked);
    _btnMute->setIcon(checked ? QIcon(":/img/data/icon_volumeoff.png") : QIcon(":/img/data/icon_volumeon.png"));
}

void PopupAudio::clearButtons()
{
    _btnPlay = nullptr;
    _btnMute = nullptr;
    _btnRepeat = nullptr;
}
