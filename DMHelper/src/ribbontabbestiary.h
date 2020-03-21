#ifndef RIBBONTABBESTIARY_H
#define RIBBONTABBESTIARY_H

#include <QFrame>

namespace Ui {
class RibbonTabBestiary;
}

class RibbonTabBestiary : public QFrame
{
    Q_OBJECT

public:
    explicit RibbonTabBestiary(QWidget *parent = nullptr);
    ~RibbonTabBestiary();

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

private:
    Ui::RibbonTabBestiary *ui;
};

#endif // RIBBONTABBESTIARY_H
