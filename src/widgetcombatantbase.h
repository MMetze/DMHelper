#ifndef WIDGETCOMBATANTBASE_H
#define WIDGETCOMBATANTBASE_H

#include <QObject>

class BattleDialogModelCombatant;
class QTimer;
class QMouseEvent;
class QHBoxLayout;
class QFrame;

class WidgetCombatantBase : public QObject
{
    Q_OBJECT
public:
    explicit WidgetCombatantBase(QWidget *parent = 0);

    virtual BattleDialogModelCombatant* getCombatant() = 0;
    virtual QFrame* getFrame() = 0;

    virtual int getInitiative() const;
    virtual bool isActive();
    virtual bool isSelected();
    virtual bool isShown();
    virtual bool isKnown();

    // From QWidget
    virtual void leaveEvent(QEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

signals:

    void contextMenu(BattleDialogModelCombatant* combatant, const QPoint& position);
    void hitPointsChanged();

public slots:

    virtual void updateData();
    virtual void setInitiative(int initiative) = 0;
    virtual void initiativeChanged(int initiative);
    virtual void setHitPoints(int hp) = 0;
    virtual void handleHitPointsChanged(int hp);
    virtual void setActive(bool active);
    virtual void setSelected(bool selected);

protected slots:
    virtual void setHighlighted(bool highlighted);
    virtual void timerExpired();

protected:

    // local
    virtual void executeDoubleClick();
    //virtual void loadImage();
    //virtual QHBoxLayout* createPairLayout(const QString& pairName, const QString& pairValue);
    //virtual void updatePairData(QHBoxLayout* pair, const QString& pairValue);
    //virtual void setPairHighlighted(QHBoxLayout* pair, bool highlighted);
    virtual void setWidgetHighlighted(QWidget* widget, bool highlighted);
    virtual QString getStyleString();

    // Data
    Qt::MouseButton _mouseDown;
    bool _active;
    bool _selected;

    // UI elements
    //QLabel* _lblIcon;
    //QLabel* _lblInitName;
    //QLineEdit* _edtInit;

    // Highlight animation
    int _angle;
    QTimer* _timer;

};

#endif // WIDGETCOMBATANTBASE_H
