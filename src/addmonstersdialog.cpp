#include "addmonstersdialog.h"
#include "ui_addmonstersdialog.h"
#include "bestiary.h"

AddMonstersDialog::AddMonstersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddMonstersDialog)
{
    ui->setupUi(this);

    ui->edtNumber->setValidator(new QIntValidator(1,999,this));
    ui->cmbMonsterType->addItems(Bestiary::Instance()->getMonsterList());
}

AddMonstersDialog::~AddMonstersDialog()
{
    delete ui;
}

int AddMonstersDialog::getMonsterCount() const
{
    return ui->edtNumber->text().toInt();
}

QString AddMonstersDialog::getMonsterType() const
{
    return ui->cmbMonsterType->currentText();
}

QString AddMonstersDialog::getMonsterName() const
{
    return ui->edtName->text();
}
