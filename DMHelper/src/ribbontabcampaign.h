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
    void setCampaignEnabled(bool enabled);
    void setUndoEnabled(bool enabled);
    void setAddPCButton(bool isPC);

signals:
    void newPartyClicked();
    void newCharacterClicked();
    void newMapClicked();
    void newTextClicked();
    void newBattleClicked();
    void newScrollingTextClicked();
    void newSoundClicked();
    void newSyrinscapeClicked();
    void newYoutubeClicked();
    void removeItemClicked();
    void exportItemClicked();
    void importItemClicked();
    void importCharacterClicked();

    void undoClicked();
    void redoClicked();

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    Ui::RibbonTabCampaign *ui;
};

#endif // RIBBONTABCAMPAIGN_H
