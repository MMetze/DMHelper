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

signals:
    void newClicked();
    void openClicked();
    void saveClicked();
    void saveAsClicked();
    void closeClicked();

    void userGuideClicked();
    void gettingStartedClicked();
    void optionsClicked();
    void checkForUpdatesClicked();
    void helpClicked();
    void aboutClicked();

protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void showMRUMenu();
    void hideMRUMenu();

private:
    Ui::RibbonTabFile *ui;
    QMenu* _mruMenu;
};

#endif // RIBBONTABFILE_H
