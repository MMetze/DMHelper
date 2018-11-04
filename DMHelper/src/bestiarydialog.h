#ifndef BESTIARYDIALOG_H
#define BESTIARYDIALOG_H

#include <QDialog>

class MonsterClass;
class Bestiary;

namespace Ui {
class BestiaryDialog;
}

class BestiaryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BestiaryDialog(QWidget *parent = 0);
    ~BestiaryDialog();

    MonsterClass* getMonster() const;

signals:
    void monsterChanged();
    void publishMonsterImage(QImage img);

public slots:
    void setMonster(MonsterClass* monster, bool edit = true);
    void setMonster(const QString& monsterName, bool edit = true);
    void createNewMonster();
    void deleteCurrentMonster();

    void previousMonster();
    void nextMonster();

    void dataChanged();

protected slots:
    void abilityChanged();
    void updateAbilityMods();
    void monsterRenamed();
    void handlePublishButton();

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

    Ui::BestiaryDialog *ui;

    MonsterClass* _monster;
    bool _edit;
    bool _mouseDown;
};

#endif // BESTIARYDIALOG_H
