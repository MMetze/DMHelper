#include "characterimportdialog.h"
#include "ui_characterimportdialog.h"

CharacterImportDialog::CharacterImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CharacterImportDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
}

CharacterImportDialog::~CharacterImportDialog()
{
    delete ui;
}

QString CharacterImportDialog::getCharacterId()
{
    return getCharacterIdFromUrl(ui->edtUrl->text());
}

QString CharacterImportDialog::getCharacterIdFromUrl(const QString& url)
{
    QStringList list = url.split(QString("/"));
    for(int i = 0; i < list.count(); ++i)
    {
        bool ok = false;
        list.at(i).toInt(&ok);
        if(ok)
            return list.at(i);
    }

    return QString();
}

