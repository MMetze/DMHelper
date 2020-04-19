#ifndef RIBBONTABCAMPAIGN_H
#define RIBBONTABCAMPAIGN_H

#include "ribbonframe.h"

namespace Ui {
class RibbonTabCampaign;
}

class RibbonTabCampaign : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonTabCampaign(QWidget *parent = nullptr);
    ~RibbonTabCampaign();

    virtual PublishButtonRibbon* getPublishRibbon() override;

public slots:
    //void setPlayersWindow(bool checked);
    void setCampaignEnabled(bool enabled);
    void setUndoEnabled(bool enabled);

signals:
    void newAdventureClicked();
    void newCharacterClicked();
    void newMapClicked();
    void newNPCClicked();
    void newTextClicked();
    void newBattleClicked();
    void newScrollingTextClicked();
    void exportItemClicked();
    void importItemClicked();
    void importCharacterClicked();
    void importNPCClicked();

    void undoClicked();
    void redoClicked();
    //void playersWindowClicked(bool checked);
    //void previewClicked();

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    Ui::RibbonTabCampaign *ui;
};

#endif // RIBBONTABCAMPAIGN_H
