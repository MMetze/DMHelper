#ifndef CHARACTERIMPORTDIALOG_H
#define CHARACTERIMPORTDIALOG_H

#include <QDialog>

namespace Ui {
class CharacterImportDialog;
}

class CharacterImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CharacterImportDialog(QWidget *parent = nullptr);
    ~CharacterImportDialog();

    QString getCharacterId();

private:
    Ui::CharacterImportDialog *ui;
};

#endif // CHARACTERIMPORTDIALOG_H
