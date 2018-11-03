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

    virtual BattleDialogModelCombatant* getCombatant();
    virtual QFrame* getFrame();

    virtual int getInitiative() const;
//    virtual bool isActive();
//    virtual bool isSelected();
//    virtual bool isShown();
//    virtual bool isKnown();

signals:

    //void contextMenu(BattleDialogModelCombatant* combatant, const QPoint& position);

public slots:

    virtual void updateData();
    virtual void setInitiative(int initiative);
    virtual void setHitPoints(int hp);
    //virtual void setActive(bool active);
    //virtual void setSelected(bool selected);

protected slots:
    virtual void setHighlighted(bool highlighted);
    //virtual void timerExpired();

protected:

    // From QWidget
    //virtual void leaveEvent(QEvent * event);
    //virtual void mousePressEvent(QMouseEvent * event);
    //virtual void mouseReleaseEvent(QMouseEvent * event);
    //virtual void mouseDoubleClickEvent(QMouseEvent *event);

    // local
    virtual void executeDoubleClick();
    //virtual void loadImage();
    //virtual QHBoxLayout* createPairLayout(const QString& pairName, const QString& pairValue);
    //virtual void updatePairData(QHBoxLayout* pair, const QString& pairValue);
    //virtual void setPairHighlighted(QHBoxLayout* pair, bool highlighted);
    //virtual void setWidgetHighlighted(QWidget* widget, bool highlighted);
    //virtual QString getStyleString();

    // Data
    WidgetCharacter* _widgetParent;
    BattleDialogModelCharacter* _character;

    Qt::MouseButton _mouseDown;
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
