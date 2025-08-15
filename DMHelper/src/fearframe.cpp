#include "fearframe.h"
#include "campaign.h"
#include "ui_fearframe.h"
#include <QPainter>

FearFrame::FearFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FearFrame),
    _campaign(nullptr),
    _buttonMap(),
    _fearOff(),
    _fearOn()
{
    ui->setupUi(this);

    _buttonMap.insert(1, ui->btnFear1);
    _buttonMap.insert(2, ui->btnFear2);
    _buttonMap.insert(3, ui->btnFear3);
    _buttonMap.insert(4, ui->btnFear4);
    _buttonMap.insert(5, ui->btnFear5);
    _buttonMap.insert(6, ui->btnFear6);
    _buttonMap.insert(7, ui->btnFear7);
    _buttonMap.insert(8, ui->btnFear8);
    _buttonMap.insert(9, ui->btnFear9);
    _buttonMap.insert(10, ui->btnFear10);
    _buttonMap.insert(11, ui->btnFear11);
    _buttonMap.insert(12, ui->btnFear12);

    connect(ui->btnFear1, &QPushButton::clicked, this, &FearFrame::buttonClicked);
    connect(ui->btnFear2, &QPushButton::clicked, this, &FearFrame::buttonClicked);
    connect(ui->btnFear3, &QPushButton::clicked, this, &FearFrame::buttonClicked);
    connect(ui->btnFear4, &QPushButton::clicked, this, &FearFrame::buttonClicked);
    connect(ui->btnFear5, &QPushButton::clicked, this, &FearFrame::buttonClicked);
    connect(ui->btnFear6, &QPushButton::clicked, this, &FearFrame::buttonClicked);
    connect(ui->btnFear7, &QPushButton::clicked, this, &FearFrame::buttonClicked);
    connect(ui->btnFear8, &QPushButton::clicked, this, &FearFrame::buttonClicked);
    connect(ui->btnFear9, &QPushButton::clicked, this, &FearFrame::buttonClicked);
    connect(ui->btnFear10, &QPushButton::clicked, this, &FearFrame::buttonClicked);
    connect(ui->btnFear11, &QPushButton::clicked, this, &FearFrame::buttonClicked);
    connect(ui->btnFear12, &QPushButton::clicked, this, &FearFrame::buttonClicked);

    QPixmap pmpFearOff(":/img/data/img/hood.png");
    if(!pmpFearOff.isNull())
    {
        QPixmap pmpFearOn(pmpFearOff.size());
        pmpFearOn.fill(Qt::transparent);

        QPainter p(&pmpFearOn);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(pmpFearOn.rect(), Qt::red);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.drawPixmap(0, 0, pmpFearOff);
        p.end();

        _fearOff = QIcon(pmpFearOff);
        _fearOn = QIcon(pmpFearOn);
    }
}

FearFrame::~FearFrame()
{
    delete ui;
}

void FearFrame::setCampaign(Campaign* campaign)
{
    _campaign = campaign;
    setFear(_campaign ? _campaign->getFearCount() : 0);
}

void FearFrame::buttonClicked()
{
    if(!_campaign)
        return;

    QPushButton* clickedButton = dynamic_cast<QPushButton*>(sender());
    if(!clickedButton)
        return;

    int buttonIndex = _buttonMap.key(clickedButton, -1);
    if(buttonIndex == -1)
        return;

    _campaign->setFearCount(_campaign->getFearCount() + ((buttonIndex > _campaign->getFearCount()) ? 1 : -1));
    setFear(_campaign->getFearCount());
}

void FearFrame::setFear(int fear)
{
    for(int i = 1; i <= 12; ++i)
    {
        QPushButton* button = _buttonMap.value(i, nullptr);
        if(button)
        {
            if(i <= fear)
                button->setIcon(_fearOn);
            else
                button->setIcon(_fearOff);
        }
    }
}
