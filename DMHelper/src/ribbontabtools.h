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

public slots:
    void setGridLocked(bool locked);

signals:
    void bestiaryClicked();
    void exportBestiaryClicked();
    void importBestiaryClicked();

    void screenClicked();
    void tablesClicked();
    void referenceClicked();
    void soundboardClicked();
    void spellbookClicked();
    void searchClicked();

    void rollDiceClicked();
    void randomMarketClicked();
    void calendarClicked();
    void countdownClicked();

    void lockGridClicked(bool checked);
    void configureGridClicked();

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    Ui::RibbonTabTools *ui;
};

#endif // RIBBONTABTOOLS_H
