#ifndef RIBBONTABTOOLS_H
#define RIBBONTABTOOLS_H

#include "ribbonframe.h"

namespace Ui {
class RibbonTabTools;
}

class RibbonTabTools : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonTabTools(QWidget *parent = nullptr);
    ~RibbonTabTools();

    virtual PublishButtonRibbon* getPublishRibbon() override;

signals:
    void bestiaryClicked();
    void exportBestiaryClicked();
    void importBestiaryClicked();

    void spellbookClicked();

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
    Ui::RibbonTabTools *ui;
};

#endif // RIBBONTABTOOLS_H
