#ifndef RIBBONTABFILE_H
#define RIBBONTABFILE_H

#include "ribbonframe.h"
#include <QMenu>

namespace Ui {
class RibbonTabFile;
}

class RibbonTabFile : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonTabFile(QWidget *parent = nullptr);
    ~RibbonTabFile();

    QMenu* getMRUMenu() const;

    virtual PublishButtonRibbon* getPublishRibbon() override;

public slots:
    void setNetworkActive(bool checked);

signals:
    void newClicked();
    void openClicked();
    void saveClicked();
    void saveAsClicked();

    void networkActiveClicked(bool checked);
    void sessionClicked();

    void optionsClicked();
    void closeClicked();

    void userGuideClicked();
    void gettingStartedClicked();
    void checkForUpdatesClicked();
    void aboutClicked();

protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void showMRUMenu();
    void hideMRUMenu();
    void setNetworkActiveImage(bool active);

private:
    Ui::RibbonTabFile *ui;
    QMenu* _mruMenu;
};

#endif // RIBBONTABFILE_H
