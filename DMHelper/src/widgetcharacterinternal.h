#ifndef WIDGETCHARACTERINTERNAL_H
#define WIDGETCHARACTERINTERNAL_H

#include "widgetcombatantbase.h"

class BattleDialogModelCharacter;
class QTimer;
class WidgetCharacter;

class WidgetCharacterInternal : public WidgetCombatantBase
{
    Q_OBJECT
public:
    explicit WidgetCharacterInternal(BattleDialogModelCharacter* character, WidgetCharacter *parent);

    virtual BattleDialogModelCombatant* getCombatant() override;
    virtual QFrame* getFrame() override;

    virtual int getInitiative() const override;
//    virtual bool isActive();
//    virtual bool isSelected();
//    virtual bool isShown();
//    virtual bool isKnown();

signals:

    //void contextMenu(BattleDialogModelCombatant* combatant, const QPoint& position);

public slots:

    virtual void updateData() override;
    virtual void setInitiative(int initiative) override;
    virtual void setHitPoints(int hp) override;
    //virtual void setActive(bool active);
    //virtual void setSelected(bool selected);

protected slots:
    virtual void setHighlighted(bool highlighted) override;
    //virtual void timerExpired();

protected:

    // From QWidget
    //virtual void leaveEvent(QEvent * event);
    //virtual void mousePressEvent(QMouseEvent * event);
    //virtual void mouseReleaseEvent(QMouseEvent * event);
    //virtual void mouseDoubleClickEvent(QMouseEvent *event);

    // local
    virtual void executeDoubleClick() override;
    //virtual void loadImage();
    //virtual QHBoxLayout* createPairLayout(const QString& pairName, const QString& pairValue);
    //virtual void updatePairData(QHBoxLayout* pair, const QString& pairValue);
    //virtual void setPairHighlighted(QHBoxLayout* pair, bool highlighted);
    //virtual void setWidgetHighlighted(QWidget* widget, bool highlighted);
    //virtual QString getStyleString();

    // Data
    WidgetCharacter* _widgetParent;
    BattleDialogModelCharacter* _character;

    //Qt::MouseButton _localMouseDown;
    //bool _active;
    //bool _selected;

    // UI elements
    //QLabel* _lblIcon;
    //QLabel* _lblInitName;
    //QLineEdit* _edtInit;

    // Highlight animation
    //int _angle;
    //QTimer* _timer;

};

#endif // WIDGETCHARACTERINTERNAL_H
