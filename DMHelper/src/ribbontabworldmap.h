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
    void setScale(int scale);

    void registerPartyIcon(Party* party);
    void removePartyIcon(Party* party);
    void clearPartyIcons();

signals:
    void showPartyClicked(bool showParty);
    void partySelected(Party* party);
    void scaleChanged(int scale);

protected:
    virtual void showEvent(QShowEvent *event) override;

protected slots:
    void selectAction(QAction* action);

private:
    Ui::RibbonTabWorldMap *ui;

    QMenu* _menu;
};

class RibbonTabWorldMap_PartyAction : public QAction
{
    Q_OBJECT

public:
    explicit RibbonTabWorldMap_PartyAction(Party* party, QObject *parent = nullptr);
    virtual ~RibbonTabWorldMap_PartyAction() override;

    Party* getParty();

public slots:
    void updateParty();
    void partyDestroyed();

protected:
    Party* _party;

};

#endif // RIBBONTABWORLDMAP_H
