#ifndef EMPTYCAMPAIGNFRAME_H
#define EMPTYCAMPAIGNFRAME_H

#include "campaignobjectframe.h"

namespace Ui {
class EmptyCampaignFrame;
}

class EmptyCampaignFrame : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit EmptyCampaignFrame(QWidget *parent = nullptr);
    ~EmptyCampaignFrame();

private:
    Ui::EmptyCampaignFrame *ui;
};

#endif // EMPTYCAMPAIGNFRAME_H
