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
    virtual bool isHover();
    virtual bool isShown();
    virtual bool isKnown();

    virtual void setShowDone(bool showDone) = 0;

signals:

    void contextMenu(BattleDialogModelCombatant* combatant, const QPoint& position);
    void isShownChanged(bool isShown);
    void isKnownChanged(bool isKnown);
    void imageChanged(BattleDialogModelCombatant* combatant);

public slots:

    virtual void updateData();
    virtual void updateMove();
    virtual void setInitiative(int initiative);
    virtual void initiativeChanged();
    virtual void setActive(bool active);
    virtual void setSelected(bool selected);
    virtual void setHover(bool hover);
    virtual void selectCombatant() = 0;

protected:

    // From QWidget
    virtual void showEvent(QShowEvent* event) override;
    virtual void enterEvent(QEnterEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

    // local
    virtual void loadImage();
    virtual QHBoxLayout* createPairLayout(const QString& pairName, const QString& pairValue);
    virtual void updatePairData(QHBoxLayout* pair, const QString& pairValue);
    virtual QString getStyleString();

    // Data
    Qt::MouseButton _mouseDown;
    bool _active;
    bool _selected;
    bool _hover;

    // UI elements
    QLabel* _lblIcon;
    QLabel* _lblInitName;
    QLineEdit* _edtInit;

};

#endif // COMBATANTWIDGET_H
