#ifndef ADDMONSTERSDIALOG_H
#define ADDMONSTERSDIALOG_H

#include <QDialog>

namespace Ui {
class AddMonstersDialog;
}

class AddMonstersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddMonstersDialog(QWidget *parent = 0);
    ~AddMonstersDialog();

    int getMonsterCount() const;
    QString getMonsterType() const;
    QString getMonsterName() const;

private:
    Ui::AddMonstersDialog *ui;
};

#endif // ADDMONSTERSDIALOG_H
