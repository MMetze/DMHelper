#include "ribbontabaudio.h"
#include "dmconstants.h"
#include "ui_ribbontabaudio.h"

RibbonTabAudio::RibbonTabAudio(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabAudio)
{
    ui->setupUi(this);

    connect(ui->btnPlay, SIGNAL(clicked(bool)), this, SIGNAL(playClicked(bool)));
    connect(ui->btnRepeat, SIGNAL(clicked(bool)), this, SIGNAL(repeatClicked(bool)));
    connect(ui->btnMute, SIGNAL(clicked(bool)), this, SIGNAL(muteClicked(bool)));
    connect(ui->sliderVolume, SIGNAL(valueChanged(int)), this, SLOT(handleVolumeChanged(int)));
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
    switch(type)
    {
        case DMHelper::AudioType_Syrinscape:
            ui->btnPlay->setIcon(QIcon(QString(":/img/data/icon_syrinscape.png")));
            ui->btnRepeat->setEnabled(false);
            ui->btnMute->setEnabled(false);
            ui->sliderVolume->setEnabled(false);
            break;
        case DMHelper::AudioType_Youtube:
            ui->btnPlay->setIcon(QIcon(QString(":/img/data/icon_playerswindow.png")));
            ui->btnRepeat->setEnabled(true);
            ui->btnMute->setEnabled(true);
            ui->sliderVolume->setEnabled(true);
            break;
        case DMHelper::AudioType_File:
        default:
            ui->btnPlay->setIcon(QIcon(QString(":/img/data/icon_soundboard.png")));
            ui->btnRepeat->setEnabled(true);
            ui->btnMute->setEnabled(true);
            ui->sliderVolume->setEnabled(true);
            break;
    }
}

void RibbonTabAudio::setPlay(bool checked)
{
    ui->btnPlay->setChecked(checked);
}

void RibbonTabAudio::setRepeat(bool checked)
{
    ui->btnRepeat->setChecked(checked);
}

void RibbonTabAudio::setMute(bool checked)
{
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

    setStandardButtonSize(*ui->lblPlay, *ui->btnPlay, frameHeight);
    setLineHeight(*ui->line, frameHeight);
    setStandardButtonSize(*ui->lblRepeat, *ui->btnRepeat, frameHeight);
    setStandardButtonSize(*ui->lblMute, *ui->btnRepeat, frameHeight);
}
