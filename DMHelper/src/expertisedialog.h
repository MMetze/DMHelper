#ifndef EXPERTISEDIALOG_H
#define EXPERTISEDIALOG_H

#include "character.h"
#include <QDialog>
#include <QMap>
#include <QCheckBox>

namespace Ui {
class ExpertiseDialog;
}

class ExpertiseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExpertiseDialog(Character& character, QWidget *parent = nullptr);
    ~ExpertiseDialog();

private slots:
    void prepareCheckboxes();
    void skillChecked();
    void jackChecked();

private:
    Ui::ExpertiseDialog *ui;

    Character& _character;
    QMap<QCheckBox*, Combatant::Skills> _checkMap;
};

#endif // EXPERTISEDIALOG_H
