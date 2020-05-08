#ifndef PARTYFRAME_H
#define PARTYFRAME_H

#include "campaignobjectframe.h"

namespace Ui {
class PartyFrame;
}

class Party;
class Character;
class QVBoxLayout;

class PartyFrame : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit PartyFrame(QWidget *parent = nullptr);
    ~PartyFrame();

    virtual void activateObject(CampaignObjectBase* object) override;
    virtual void deactivateObject() override;

    void setParty(Party* party);

signals:
    void publishPartyImage(QImage img);
    void partyChanged();

public slots:
    // Publish slots from CampaignObjectFrame
    virtual void publishClicked(bool checked) override;
    virtual void setRotation(int rotation) override;

    void loadCharacters();

protected:
    virtual void mousePressEvent(QMouseEvent * event) override;
    virtual void mouseReleaseEvent(QMouseEvent * event) override;

private:
    void readPartyData();
    void loadPartyImage();
    void handlePublishClicked();

    void clearList();
    void addCharacter(Character* character);

    Ui::PartyFrame *ui;
    QVBoxLayout* _characterLayout;

    Party* _party;
    bool _mouseDown;
    int _rotation;
};

#endif // PARTYFRAME_H
