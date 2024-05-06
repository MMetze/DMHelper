#ifndef COMBATANTWIDGETINTERNALS_H
#define COMBATANTWIDGETINTERNALS_H

#include <QObject>

class BattleDialogModelCombatant;
class QTimer;
class QMouseEvent;
class QHBoxLayout;
class QFrame;

class CombatantWidgetInternals : public QObject
{
    Q_OBJECT
public:
    explicit CombatantWidgetInternals(QWidget *parent = nullptr);

    virtual BattleDialogModelCombatant* getCombatant() = 0;
    virtual QFrame* getFrame() = 0;

    virtual int getInitiative() const;
    virtual bool isShown();
    virtual bool isKnown();

    // From QWidget
    virtual void leaveEvent(QEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

signals:

    void contextMenu(BattleDialogModelCombatant* combatant, const QPoint& position);
    void hitPointsChanged(BattleDialogModelCombatant* combatant, int change);

public slots:
    virtual void updateData();
    virtual void updateImage();
    virtual void setInitiative(int initiative) = 0;
    virtual void initiativeChanged(int initiative);
    virtual void moveChanged(int move);
    virtual void setHitPoints(int hp) = 0;
    virtual void handleHitPointsChanged(int hp);
    virtual void executeDoubleClick();

protected:

    // Data
    Qt::MouseButton _mouseDown;

};

#endif // COMBATANTWIDGETINTERNALS_H
