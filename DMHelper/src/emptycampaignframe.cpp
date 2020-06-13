#include "emptycampaignframe.h"
#include "ui_emptycampaignframe.h"

EmptyCampaignFrame::EmptyCampaignFrame(QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::EmptyCampaignFrame)
{
    ui->setupUi(this);
}

EmptyCampaignFrame::~EmptyCampaignFrame()
{
    delete ui;
}
