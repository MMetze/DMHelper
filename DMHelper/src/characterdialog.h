#ifndef CHARACTERDIALOG_H
#define CHARACTERDIALOG_H

#include <QDialog>
#include "character.h"

class MainWindow;
class QCheckBox;

namespace Ui {
class CharacterDialog;
}

class CharacterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CharacterDialog(QWidget *parent = 0);
    ~CharacterDialog();

    void setCharacter(Character* character);

public slots:
    void calculateMods();

protected:
    // From QWidget
    void closeEvent(QCloseEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);

private slots:
    void readCharacterData();
    void writeCharacterData();
    
private:
    void loadCharacterImage();
    void updateCheckboxName(QCheckBox* chk, int abilityMod, int proficiencyBonus);

    Ui::CharacterDialog *ui;
    Character* _character;
    bool mouseDown;
};

#endif // CHARACTERDIALOG_H
