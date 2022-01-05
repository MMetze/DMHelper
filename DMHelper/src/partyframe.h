#ifndef PARTYFRAME_H
#define PARTYFRAME_H

#include "campaignobjectframe.h"
#include "partycharactergridframe.h"
#include <QList>

namespace Ui {
class PartyFrame;
}

class Party;
class Character;
//class QVBoxLayout;
class QGridLayout;
class PartyCharacterGridFrame;

class PartyFrame : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit PartyFrame(QWidget *parent = nullptr);
    ~PartyFrame();

    virtual void activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer) override;
    virtual void deactivateObject() override;

    void setParty(Party* party);

signals:
    void publishPartyImage(QImage img);
    void partyChanged();
    void characterSelected(const QUuid& characterId);

public slots:
    // Publish slots from CampaignObjectFrame
    virtual void publishClicked(bool checked) override;
    virtual void setRotation(int rotation) override;

    void handleCharacterChanged(QUuid id);

protected:
    virtual void mousePressEvent(QMouseEvent * event) override;
    virtual void mouseReleaseEvent(QMouseEvent * event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateAll();

private:
    void clearGrid();
    void clearList();
    void readPartyData();
    void loadPartyImage();
    void loadCharacters();
    void updateLayout();
    int getColumnCount();

    void calculateThresholds();
    void handlePublishClicked();

    Ui::PartyFrame *ui;
    //QVBoxLayout* _characterLayout;
    QGridLayout* _characterGrid;
    QList<PartyCharacterGridFrame* > _characterFrames;

    Party* _party;
    bool _mouseDown;
    int _rotation;
    int _layoutColumns;
};

#endif // PARTYFRAME_H
