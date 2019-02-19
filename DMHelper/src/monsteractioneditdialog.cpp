#include "monsteractioneditdialog.h"
#include "ui_monsteractioneditdialog.h"
#include <QIntValidator>

MonsterActionEditDialog::MonsterActionEditDialog(const MonsterAction& action, bool allowDelete, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MonsterActionEditDialog),
    _deleteAction(false),
    _action(action)
{
    ui->setupUi(this);

    ui->edtName->setText(action.getName());
    ui->edtDescription->setPlainText(action.getDescription());

    ui->edtAttackBonus->setText(QString::number(action.getAttackBonus()));
    ui->edtAttackBonus->setValidator(new QIntValidator(0,999999,this));
    ui->edtDamageDiceCount->setText(QString::number(action.getDamageDice().getCount()));
    ui->edtDamageDiceCount->setValidator(new QIntValidator(0,999999,this));
    ui->edtDamageDiceType->setText(QString::number(action.getDamageDice().getType()));
    ui->edtDamageDiceType->setValidator(new QIntValidator(0,999999,this));
    ui->edtDamageDiceBonus->setText(QString::number(action.getDamageDice().getBonus()));
    ui->edtDamageDiceBonus->setValidator(new QIntValidator(0,999999,this));

    ui->btnDelete->setVisible(allowDelete);
    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(deleteClicked()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

MonsterActionEditDialog::~MonsterActionEditDialog()
{
    delete ui;
}

const MonsterAction& MonsterActionEditDialog::getAction()
{
    dataChanged();
    return _action;
}

bool MonsterActionEditDialog::isDeleted() const
{
    return _deleteAction;
}

void MonsterActionEditDialog::deleteClicked()
{
    _deleteAction = true;
    reject();
}

void MonsterActionEditDialog::dataChanged()
{
    _action = MonsterAction(ui->edtAttackBonus->text().toInt(),
                            ui->edtDescription->toPlainText(),
                            ui->edtName->text(),
                            Dice(ui->edtDamageDiceCount->text().toInt(),
                                 ui->edtDamageDiceType->text().toInt(),
                                 ui->edtDamageDiceBonus->text().toInt()));
}
