#ifndef CAMPAIGNOBJECTFRAME_H
#define CAMPAIGNOBJECTFRAME_H

#include <QFrame>

class CampaignObjectBase;

class CampaignObjectFrame : public QFrame
{
public:
    CampaignObjectFrame(QWidget *parent = nullptr);
    virtual ~CampaignObjectFrame();

    virtual void activateObject(CampaignObjectBase* object);
    virtual void deactivateObject();

    virtual bool isAnimated();
};

#endif // CAMPAIGNOBJECTFRAME_H
