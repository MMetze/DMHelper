#include "dmscreentabwidget.h"
#include "dicerolldialog.h"
#include "ui_dmscreentabwidget.h"

DMScreenTabWidget::DMScreenTabWidget(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::DMScreenTabWidget)
{
    ui->setupUi(this);

    connect(ui->lbl01_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl02_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl03_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl04_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl05_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl06_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl07_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl08_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl09_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl10_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl11_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl12_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl13_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl14_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl15_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl16_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl17_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl18_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl19_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl20_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl21_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl22_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl23_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl24_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl25_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl26_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl27_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl28_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl29_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl30_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl31_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl32_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl33_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl34_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl35_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl36_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
    connect(ui->lbl37_2,SIGNAL(linkActivated(QString)),this,SLOT(triggerRoll(QString)));
}

DMScreenTabWidget::~DMScreenTabWidget()
{
    delete ui;
}

void DMScreenTabWidget::triggerRoll(const QString& link)
{
    if(!link.startsWith(QString("#roll")))
        return;

    QString diceInput(link);
    diceInput.remove(0,5);
    Dice dice(diceInput.trimmed());
    DiceRollDialog *drDlg = new DiceRollDialog(dice, this);
    drDlg->show();
}
