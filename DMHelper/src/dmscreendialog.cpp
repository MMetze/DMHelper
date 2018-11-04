#include "dmscreendialog.h"
#include "dicerolldialog.h"
#include "ui_dmscreendialog.h"
#include <QMessageBox>

DMScreenDialog::DMScreenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DMScreenDialog)
{
    ui->setupUi(this);

    connect(ui->lbl01,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl02,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl03,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl04,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl05,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl06,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl07,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl08,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl09,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl10,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl11,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl12,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl13,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl14,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl15,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl16,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl17,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl18,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl19,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl20,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl21,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl22,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl23,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl24,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl25,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl26,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl27,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl28,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl29,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl30,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl31,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl32,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl33,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl34,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl35,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl36,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl37,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
}

DMScreenDialog::~DMScreenDialog()
{
    delete ui;
}

void DMScreenDialog::triggerRoll(const QString& link)
{
    if(!link.startsWith(QString("#roll")))
        return;

    QString diceInput(link);
    diceInput.remove(0,5);
    Dice dice(diceInput.trimmed());
    DiceRollDialog *drDlg = new DiceRollDialog(dice, this);
    drDlg->show();
}
