#ifndef CHASEDIALOG_H
#define CHASEDIALOG_H

#include <QDialog>
#include <QMap>
#include <QList>
#include <QFrame>
#include <QPair>
#include "combatant.h"
#include "complicationdata.h"

class QVBoxLayout;
class QGraphicsScene;
class QGraphicsLineItem;
class QGraphicsTextItem;
class QTimer;
class ComplicationData;
class Chaser;
class ChaserComplication;
class ChaserCombatant;
class ChaseAnimationDialog;

namespace Ui {
class ChaseDialog;
}

class ChaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChaseDialog(const QString& filename, QWidget *parent = 0);
    ~ChaseDialog();

    void addCombatant(Combatant* combatant, bool pursuer);
    void addCombatants(QList<Combatant*> combatants, bool pursuer);
    void addCombatantGroup(CombatantGroup group, bool pursuer);
    void addCombatantGroups(CombatantGroupList groups, bool pursuer);
    void removeCombatant(Combatant* combatant);

signals:
    void chaseComplete();
    void publishChaseScene(QImage img);

public slots:
    void generateComplication();
    void sortChasers();

    void selectNextChaser();
    void moveChaser();
    void dashChaser();

    void sceneSelectionChanged();

protected:
    virtual void keyPressEvent(QKeyEvent * event);
    virtual void showEvent(QShowEvent * event);
    virtual void hideEvent(QHideEvent * event);
    virtual bool eventFilter(QObject *obj, QEvent *event);

    bool getComplication(const QString& location, int value, ComplicationData& data) const;
    ChaserComplication* findComplication(QGraphicsTextItem* textItem) const;
    void showComplication(const ComplicationData& data);
    void showCombatant(Combatant* Combatant);
    void loadData();
    void registerChaser(ChaserCombatant* chaser);
    void setActiveChaser(int newChaser);
    void clearChaserWidgets();
    void buildChaserWidgets();
    void setHighlight(int chaser, bool highlightOn);

    void moveChaserBy(ChaserCombatant* chaser, int move);
    void validateLine();
    int calculateSceneHeight() const;
    int getChaserXPos(int index) const;
    QPair<int,int> getChaserMinMaxX() const;

    Chaser* getFurthest();
    Chaser* getFirstMover();
    Chaser* getNextMover();

protected slots:
    void handlePublishButton();
    void updateAnimation();

private:
    static bool CompareChasersPosition(const Chaser* a, const Chaser* b);
    static bool CompareChasersInitiative(const Chaser* a, const Chaser* b);

    Ui::ChaseDialog *ui;

    QVBoxLayout* _chaseLayout;
    QGraphicsScene* _chaseScene;
    QGraphicsLineItem* _centralLine;
    ChaseAnimationDialog* _animation;
    QTimer* _animationTimer;

    QList<ChaserComplication*> _complicationList;
    QList<ChaserCombatant*> _chaserList;
    int _activeChaser;

    QGraphicsTextItem* _selectedComplication;

    QString _chaseFileName;
    QMap<QString, QList<ComplicationData> > _chaseData;

};

#endif // CHASEDIALOG_H
