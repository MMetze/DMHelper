#ifndef BESTIARYDIALOG_H
#define BESTIARYDIALOG_H

#include <QDialog>

class MonsterClass;
class MonsterAction;
class Bestiary;
class QVBoxLayout;

namespace Ui {
class BestiaryDialog;
}

class BestiaryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BestiaryDialog(QWidget *parent = nullptr);
    ~BestiaryDialog();

    MonsterClass* getMonster() const;

signals:
    void monsterChanged();
    void publishMonsterImage(QImage img, QColor color);

public slots:
    void setMonster(MonsterClass* monster, bool edit = true);
    void setMonster(const QString& monsterName, bool edit = true);
    void createNewMonster();
    void deleteCurrentMonster();

    void previousMonster();
    void nextMonster();

    void dataChanged();

protected slots:
    void hitDiceChanged();
    void abilityChanged();
    void updateAbilityMods();
    void monsterRenamed();
    void handlePublishButton();

    void handleReloadImage();
    void handleClearImage();

    void addAction();
    void deleteAction(const MonsterAction& action);
    void addLegendaryAction();
    void deleteLegendaryAction(const MonsterAction& action);
    void addSpecialAbility();
    void deleteSpecialAbility(const MonsterAction& action);
    void addReaction();
    void deleteReaction(const MonsterAction& action);

    void handleChallengeEdited();
    void handleEditedData();

protected:
    // From QWidget
    virtual void closeEvent(QCloseEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void showEvent(QShowEvent * event);
    virtual void hideEvent(QHideEvent * event);
    virtual void focusOutEvent(QFocusEvent * event);

private:
    void loadMonsterImage();
    void storeMonsterData();

    void clearActionWidgets();
    void clearWidget(QWidget* widget);

    void interpretChallengeRating(const QString& inputCR);

    Ui::BestiaryDialog *ui;
    QWidget* _actionsWidget;
    QWidget* _legendaryActionsWidget;
    QWidget* _specialAbilitiesWidget;
    QWidget* _reactionsWidget;

    MonsterClass* _monster;
    bool _edit;
    bool _mouseDown;
};

#endif // BESTIARYDIALOG_H
