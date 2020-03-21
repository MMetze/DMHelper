#ifndef RIBBONTABBATTLE_H
#define RIBBONTABBATTLE_H

#include <QFrame>

namespace Ui {
class RibbonTabBattle;
}

class RibbonTabBattle : public QFrame
{
    Q_OBJECT

public:
    explicit RibbonTabBattle(QWidget *parent = nullptr);
    ~RibbonTabBattle();

public slots:
    void setShowLiving(bool checked);
    void setShowDead(bool checked);
    void setShowEffects(bool checked);
    void setShowMovement(bool checked);
    void setLairActions(bool checked);

signals:
    void newMapClicked();
    void reloadMapClicked();
    void addCharacterClicked();
    void addMonsterClicked();
    void addNPCClicked();
    void showLivingClicked(bool checked);
    void showDeadClicked(bool checked);
    void showEffectsClicked(bool checked);
    void showMovementClicked(bool checked);
    void lairActionsClicked(bool checked);
    void nextClicked();
    void sortClicked();

private:
    Ui::RibbonTabBattle *ui;
};

#endif // RIBBONTABBATTLE_H
