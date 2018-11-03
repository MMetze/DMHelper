#ifndef WIDGETCHARACTER_H
#define WIDGETCHARACTER_H

#include <QFrame>

namespace Ui {
class WidgetCharacter;
}

class WidgetCharacterInternal;
class BattleDialogModelCombatant;

class WidgetCharacter : public QFrame
{
    Q_OBJECT

public:
    explicit WidgetCharacter(QWidget *parent = nullptr);
    ~WidgetCharacter();

    // From CombatantWidget
    virtual BattleDialogModelCombatant* getCombatant();

    void setInternals(WidgetCharacterInternal* internals);

public slots:
    virtual void updateData();

protected:
    // From QWidget
    virtual void leaveEvent(QEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void edtInitiativeChanged(const QString &text);
    void edtHPChanged(const QString &text);

private:

    virtual void loadImage();

    Ui::WidgetCharacter *ui;

    WidgetCharacterInternal* _internals;
};

#endif // WIDGETCHARACTER_H
