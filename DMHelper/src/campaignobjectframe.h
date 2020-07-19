#ifndef CAMPAIGNOBJECTFRAME_H
#define CAMPAIGNOBJECTFRAME_H

#include <QFrame>

class CampaignObjectBase;

class CampaignObjectFrame : public QFrame
{
    Q_OBJECT

public:
    CampaignObjectFrame(QWidget *parent = nullptr);
    virtual ~CampaignObjectFrame();

    virtual void activateObject(CampaignObjectBase* object);
    virtual void deactivateObject();

signals:
    void setPublishEnabled(bool enabled);
    void checkedChanged(bool checked);
    void checkableChanged(bool checkable);
    void rotationChanged(int rotation);
    void backgroundColorChanged(QColor color);

public slots:
    virtual void publishClicked(bool checked);
    virtual void setRotation(int rotation);
    virtual void setBackgroundColor(QColor color);
    virtual void reloadObject();
};

#endif // CAMPAIGNOBJECTFRAME_H
