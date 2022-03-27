#ifndef RIBBONTABWORLDMAP_H
#define RIBBONTABWORLDMAP_H

#include "dmconstants.h"
#include "ribbonframe.h"
#include <QAction>

namespace Ui {
class RibbonTabWorldMap;
}

class PublishButtonRibbon;
class Party;
class Campaign;
class QMenu;

class RibbonTabWorldMap : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonTabWorldMap(QWidget *parent = nullptr);
    virtual ~RibbonTabWorldMap() override;

    PublishButtonRibbon* getPublishRibbon() override;

public slots:
    void setShowParty(bool showParty);
    void setParty(Party* party);
    void setPartyIcon(const QString& partyIcon);
    void setScale(int scale);

    void registerPartyIcon(Party* party);
    void removePartyIcon(Party* party);
    void clearPartyIcons();

    void setShowMarkers(bool checked);

signals:
    void showPartyClicked(bool showParty);
    void partySelected(Party* party);
    void partyIconSelected(const QString& partyIcon);
    void scaleChanged(int scale);

    void showMarkersClicked(bool checked);
    void addMarkerClicked();

protected:
    virtual void showEvent(QShowEvent *event) override;

protected slots:
    void selectAction(QAction* action);
    void setPartyButtonIcon(const QIcon &icon);

private:
    Ui::RibbonTabWorldMap *ui;

    QMenu* _menu;
};

class RibbonTabWorldMap_PartyAction : public QAction
{
    Q_OBJECT

public:
    enum PartyActionType
    {
        PartyActionType_Invalid = -1,
        PartyActionType_Party = 0,
        PartyActionType_Default,
        PartyActionType_Select
    };

    explicit RibbonTabWorldMap_PartyAction(Party* party, int partyType = PartyActionType_Party, QObject *parent = nullptr);
    virtual ~RibbonTabWorldMap_PartyAction() override;

    Party* getParty() const;
    int getPartyType() const;

public slots:
    void updateParty();
    void partyDestroyed();

protected:
    Party* _party;
    int _partyType;

};

#endif // RIBBONTABWORLDMAP_H
