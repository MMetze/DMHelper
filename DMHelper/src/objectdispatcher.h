#ifndef OBJECTDISPATCHER_H
#define OBJECTDISPATCHER_H

#include <QObject>

class Campaign;
class CampaignObjectBase;
class SoundboardFrame;
class NetworkController;

class ObjectDispatcher : public QObject
{
    Q_OBJECT
public:
    explicit ObjectDispatcher(QObject *parent = nullptr);

signals:

public slots:
    void setCampaign(Campaign* campaign);
    void setSoundboard(SoundboardFrame* soundboard);
    void setNetworkController(NetworkController* networkController);

    void addObject(CampaignObjectBase* object);

private:
    void connectAllObjects();

    Campaign* _campaign;
    SoundboardFrame* _soundboard;
    NetworkController* _networkController;
};

#endif // OBJECTDISPATCHER_H
