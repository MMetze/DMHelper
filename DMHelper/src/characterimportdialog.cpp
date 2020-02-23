#include "characterimportdialog.h"
#include "ui_characterimportdialog.h"

CharacterImportDialog::CharacterImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CharacterImportDialog)
{
    ui->setupUi(this);
}

CharacterImportDialog::~CharacterImportDialog()
{
    delete ui;
}

QString CharacterImportDialog::getCharacterId()
{
    QStringList list = ui->edtUrl->text().split(QString("/"));
    for(int i = 0; i < list.count(); ++i)
    {
        bool ok = false;
        list.at(i).toInt(&ok);
        if(ok)
            return list.at(i);
    }

    return QString();
}
