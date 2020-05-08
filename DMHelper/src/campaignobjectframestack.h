#ifndef CAMPAIGNOBJECTFRAMESTACK_H
#define CAMPAIGNOBJECTFRAMESTACK_H

#include <QStackedWidget>
#include <QMap>
#include <QList>

class CampaignObjectFrame;

class CampaignObjectFrameStack : public QStackedWidget
{
    Q_OBJECT
public:
    explicit CampaignObjectFrameStack(QWidget *parent = nullptr);

    int addFrame(int objectType, CampaignObjectFrame* newFrame);
    bool addFrames(QList<int> objectTypeList, CampaignObjectFrame* newFrame);

    CampaignObjectFrame* setCurrentFrame(int objectType);
    CampaignObjectFrame* getCurrentFrame();

signals:

protected:
    QMap<int, CampaignObjectFrame*> _frames;

};

#endif // CAMPAIGNOBJECTFRAMESTACK_H
