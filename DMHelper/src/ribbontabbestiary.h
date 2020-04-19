#ifndef RIBBONTABBESTIARY_H
#define RIBBONTABBESTIARY_H

#include "ribbonframe.h"

namespace Ui {
class RibbonTabBestiary;
}

class RibbonTabBestiary : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonTabBestiary(QWidget *parent = nullptr);
    ~RibbonTabBestiary();

    virtual PublishButtonRibbon* getPublishRibbon() override;

signals:
    void bestiaryClicked();
    void exportBestiaryClicked();
    void importBestiaryClicked();

    void screenClicked();
    void tablesClicked();
    void referenceClicked();
    void soundboardClicked();

    void rollDiceClicked();
    void publishTextClicked();
    void translateTextClicked();
    void randomMarketClicked();
    void calendarClicked();
    void countdownClicked();

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    Ui::RibbonTabBestiary *ui;
};

#endif // RIBBONTABBESTIARY_H
