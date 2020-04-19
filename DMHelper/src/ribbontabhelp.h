#ifndef RIBBONTABHELP_H
#define RIBBONTABHELP_H

#include "ribbonframe.h"

namespace Ui {
class RibbonTabHelp;
}

class RibbonTabHelp : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonTabHelp(QWidget *parent = nullptr);
    ~RibbonTabHelp();

    virtual PublishButtonRibbon* getPublishRibbon() override;

signals:
    void userGuideClicked();
    void gettingStartedClicked();
    void checkForUpdatesClicked();
    void aboutClicked();

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    Ui::RibbonTabHelp *ui;
};

#endif // RIBBONTABHELP_H
