#ifndef MONSTERACTIONEDITDIALOG_H
#define MONSTERACTIONEDITDIALOG_H

#include <QDialog>
#include "monsteraction.h"

namespace Ui {
class MonsterActionEditDialog;
}

class MonsterActionEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MonsterActionEditDialog(const MonsterAction& action, bool allowDelete = false, QWidget *parent = nullptr);
    ~MonsterActionEditDialog();

    const MonsterAction& getAction();

    bool isDeleted() const;

protected slots:
    void deleteClicked();
    void dataChanged();

private:
    Ui::MonsterActionEditDialog *ui;

    bool _deleteAction;
    MonsterAction _action;
};

#endif // MONSTERACTIONEDITDIALOG_H
