#ifndef COMBATANTWIDGET_H
#define COMBATANTWIDGET_H

#include <QFrame>

class BattleDialogModelCombatant;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QTimer;

class CombatantWidget : public QFrame
{
    Q_OBJECT
public:
    explicit CombatantWidget(QWidget *parent = nullptr);

    virtual BattleDialogModelCombatant* getCombatant() = 0;

    virtual int getInitiative() const;
    virtual bool isActive();
    virtual bool isSelected();
    virtual bool isShown();
    virtual bool isKnown();

signals:

    void contextMenu(BattleDialogModelCombatant* combatant, const QPoint& position);
    void isShownChanged(bool isShown);
    void isKnownChanged(bool isKnown);

public slots:

    virtual void updateData();
    virtual void setInitiative(int initiative);
    virtual void initiativeChanged();
    virtual void setActive(bool active);
    virtual void setSelected(bool selected);

protected slots:
    virtual void setHighlighted(bool highlighted);
    virtual void timerExpired();

protected:

    // From QWidget
    virtual void leaveEvent(QEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

    // local
    virtual void executeDoubleClick();
    virtual void loadImage();
    virtual QHBoxLayout* createPairLayout(const QString& pairName, const QString& pairValue);
    virtual void updatePairData(QHBoxLayout* pair, const QString& pairValue);
    virtual void setPairHighlighted(QHBoxLayout* pair, bool highlighted);
    virtual void setWidgetHighlighted(QWidget* widget, bool highlighted);
    virtual QString getStyleString();

    // Data
    Qt::MouseButton _mouseDown;
    bool _active;
    bool _selected;

    // UI elements
    QLabel* _lblIcon;
    QLabel* _lblInitName;
    QLineEdit* _edtInit;

    // Highlight animation
    int _angle;
    QTimer* _timer;

};

#endif // COMBATANTWIDGET_H
