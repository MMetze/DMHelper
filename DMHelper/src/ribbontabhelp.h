#ifndef RIBBONTABHELP_H
#define RIBBONTABHELP_H

#include <QFrame>

namespace Ui {
class RibbonTabHelp;
}

class RibbonTabHelp : public QFrame
{
    Q_OBJECT

public:
    explicit RibbonTabHelp(QWidget *parent = nullptr);
    ~RibbonTabHelp();

signals:
    void userGuideClicked();
    void gettingStartedClicked();
    void checkForUpdatesClicked();
    void aboutClicked();

private:
    Ui::RibbonTabHelp *ui;
};

#endif // RIBBONTABHELP_H
