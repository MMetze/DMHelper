#include "ribbontabaudio.h"
#include "dmconstants.h"
#include "audiotrack.h"
#include "ui_ribbontabaudio.h"

RibbonTabAudio::RibbonTabAudio(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabAudio),
    _trackType(DMHelper::AudioType_File),
    _trackStatus(AudioTrack::AudioTrackStatus_Stop)
{
    ui->setupUi(this);

    connect(ui->btnPlay, &QPushButton::clicked, this, &RibbonTabAudio::playClicked);
    connect(ui->btnPause, &QPushButton::clicked, this, &RibbonTabAudio::pauseClicked);
    connect(ui->btnStop, &QPushButton::clicked, this, &RibbonTabAudio::stopClicked);
    connect(ui->btnRepeat, &QPushButton::clicked, this, &RibbonTabAudio::repeatClicked);
    connect(ui->btnMute, &QPushButton::clicked, this, [this](bool checked)
            {
                ui->btnMute->setIcon(checked ? QIcon(":/img/data/icon_volumeoff.png") : QIcon(":/img/data/icon_volumeon.png"));
                emit muteClicked(checked);
            });
    connect(ui->sliderVolume, &QAbstractSlider::valueChanged, this, &RibbonTabAudio::handleVolumeChanged);

    setTrackType(_trackType);
}

RibbonTabAudio::~RibbonTabAudio()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabAudio::getPublishRibbon()
{
    return ui->framePublish;
}

void RibbonTabAudio::setTrackType(int type)
{
    if(type == _trackType)
        return;

    _trackType = type;

    switch(_trackType)
    {
        case DMHelper::AudioType_Syrinscape:
            ui->lblAudioType->setPixmap(QPixmap(QString(":/img/data/icon_syrinscape.png")));
            ui->btnRepeat->setEnabled(false);
            ui->btnMute->setEnabled(false);
            ui->sliderVolume->setEnabled(false);
            break;
        case DMHelper::AudioType_Youtube:
            ui->lblAudioType->setPixmap(QPixmap(QString(":/img/data/icon_playerswindow.png")));
            ui->btnRepeat->setEnabled(true);
            ui->btnMute->setEnabled(true);
            ui->sliderVolume->setEnabled(true);
            break;
        case DMHelper::AudioType_File:
        default:
            ui->lblAudioType->setPixmap(QPixmap(QString(":/img/data/icon_soundboard.png")));
            ui->btnRepeat->setEnabled(true);
            ui->btnMute->setEnabled(true);
            ui->sliderVolume->setEnabled(true);
            break;
    }

    updateButtonStatus();
}

void RibbonTabAudio::setTrackStatus(int status)
{
    if(status == _trackStatus)
        return;

    _trackStatus = status;

    updateButtonStatus();
}

void RibbonTabAudio::setRepeat(bool checked)
{
    if(ui->btnRepeat->isChecked() == checked)
        return;

    ui->btnRepeat->setChecked(checked);
}

void RibbonTabAudio::setMute(bool checked)
{
    if(ui->btnMute->isChecked() == checked)
        return;

    ui->btnMute->setIcon(checked ? QIcon(":/img/data/icon_volumeoff.png") : QIcon(":/img/data/icon_volumeon.png"));
    ui->btnMute->setChecked(checked);
}

void RibbonTabAudio::setVolume(float volume)
{
    ui->sliderVolume->setValue(static_cast<int>(volume * 100.f));
}

void RibbonTabAudio::handleVolumeChanged(int volume)
{
    emit volumeChanged(static_cast<float>(volume) / 100.f);
}

void RibbonTabAudio::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();

    QFontMetrics metrics = ui->lblPlay->fontMetrics();
    int labelHeight = getLabelHeight(metrics, frameHeight);
    int iconDim = frameHeight - labelHeight;
    setWidgetSize(*ui->lblAudioType, iconDim, iconDim);

    setLineHeight(*ui->line_2, frameHeight);
    setStandardButtonSize(*ui->lblPlay, *ui->btnPlay, frameHeight);
    setStandardButtonSize(*ui->lblPause, *ui->btnPause, frameHeight);
    setStandardButtonSize(*ui->lblStop, *ui->btnStop, frameHeight);
    setLineHeight(*ui->line, frameHeight);
    setStandardButtonSize(*ui->lblRepeat, *ui->btnRepeat, frameHeight);
    setStandardButtonSize(*ui->lblMute, *ui->btnMute, frameHeight);
}

void RibbonTabAudio::updateButtonStatus()
{
    switch(_trackStatus)
    {
    case AudioTrack::AudioTrackStatus_Stop:
        ui->btnPlay->setEnabled(true);
        ui->btnPause->setEnabled(false);
        ui->btnStop->setEnabled(false);
        break;
    case AudioTrack::AudioTrackStatus_Play:
        ui->btnPlay->setEnabled(false);
        ui->btnPause->setEnabled(_trackType == DMHelper::AudioType_File);
        ui->btnStop->setEnabled((_trackType != DMHelper::AudioType_Syrinscape) && (_trackType != DMHelper::AudioType_SyrinscapeOnline));
        break;
    case AudioTrack::AudioTrackStatus_Pause:
        ui->btnPlay->setEnabled(true);
        ui->btnPause->setEnabled(false);
        ui->btnStop->setEnabled(true);
        break;
    default:
        break;
    }

}
