#ifndef BATTLECOMBATANTWIDGET_H
#define BATTLECOMBATANTWIDGET_H

#include <QWidget>

class BattleDialogModelCombatant;

namespace Ui {
class BattleCombatantWidget;
}

class BattleCombatantWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BattleCombatantWidget(BattleDialogModelCombatant* combatant, QWidget *parent = nullptr);
    ~BattleCombatantWidget();

    bool hasAdvantage() const;
    bool hasDisadvantage() const;

    void setResult(const QString &text);
    void setResult(int result);
    int getResult() const;

    void applyDamage(int damage);
    void applyConditions(int conditions);

    bool isActive();

signals:
    void selectCombatant(BattleDialogModelCombatant* combatant);
    void combatantChanged(BattleDialogModelCombatant* combatant);
    void rerollNeeded(BattleCombatantWidget* widget);
    void hitPointsChanged(BattleDialogModelCombatant* combatant, int change);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

protected slots:
    void handleHitPointsChanged(const QString& text);
    void handleRerollRequest();
    void handleAdvantageClicked(bool checked);
    void handleDisadvantageClicked(bool checked);
    void handleCombatantActive(bool active);

private:
    void setCombatantValues();

    Ui::BattleCombatantWidget *ui;

    BattleDialogModelCombatant* _combatant;

    bool _mouseDown;
    QPoint _mouseDownPos;
    int _result;
};

#endif // BATTLECOMBATANTWIDGET_H
