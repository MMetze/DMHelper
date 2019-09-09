#include "dicerolldialog.h"
#include "ui_dicerolldialog.h"
#include "character.h"
#include "dice.h"
#include <QtGlobal>
#include <QMouseEvent>
#include <QIntValidator>

DiceRollDialog::DiceRollDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiceRollDialog),
    _fireAndForget(false),
    _mouseDown(false),
    _mouseDownPos()
{
    ui->setupUi(this);
    init();
}

DiceRollDialog::DiceRollDialog(const Dice& dice, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiceRollDialog),
    _fireAndForget(false),
    _mouseDown(false),
    _mouseDownPos()
{
    ui->setupUi(this);
    init();

    ui->editRollCount->setText(QString("1"));
    ui->editDiceCount->setText(QString::number(dice.getCount()));
    ui->editDiceType->setText(QString::number(dice.getType()));
    ui->editBonus->setText(QString::number(dice.getBonus()));

    rollDice();
}

DiceRollDialog::~DiceRollDialog()
{
    delete ui;
}

void DiceRollDialog::fireAndForget()
{
    show();
    _fireAndForget = true;
}

void DiceRollDialog::rollDice()
{
    ui->editResult->clear();

    int rcEnd = ui->editRollCount->text().toInt();
    int dcEnd = ui->editDiceCount->text().toInt();
    int diceType = ui->editDiceType->text().toInt();
    int target = ui->editTarget->text().toInt();
    int bonus = ui->editBonus->text().toInt();
    int total = 0;

    for(int rc = 0; rc < rcEnd; ++rc)
    {
        int result = 0;
        QString resultStr;

        // Go through and roll the dice, building up the string along the way
        for(int dc = 0; dc < dcEnd; ++dc)
        {
            int roll = Dice::dX(diceType);
            if(dc > 0)
            {
                resultStr.append(QString(" + "));
            }
            resultStr.append(QString::number(roll));
            result += roll;
        }

        // Add the bonus number, if it exists
        if(bonus > 0)
        {
            resultStr.append(QString(" + ") + QString::number(bonus));
            result += bonus;
        }

        // If there was somehow more than one number shown, then we should bother showing the overall sum
        if( (dcEnd > 1) || ( bonus > 0 ) )
        {
            resultStr.append(QString(" = ") + QString::number(result));
        }

        // Set the text color based on whether or not we exceeded the target
        if(result >= target)
        {
            resultStr.prepend(QString("<font color=""#00ff00"">"));
        }
        else
        {
            resultStr.prepend(QString("<font color=""#ff0000"">"));
        }
        resultStr.append(QString("</font>\n"));

        total += result;

        // Add this result to the text
        ui->editResult->append(resultStr);
    }

    ui->editResult->append(QString("<b><font color=""#000000"">Total: ") + QString::number(total) + QString("</font></b>\n"));
}

void DiceRollDialog::hideEvent(QHideEvent * event)
{
    Q_UNUSED(event);

    if(_fireAndForget)
    {
        deleteLater();
    }
}

void DiceRollDialog::init()
{
    QValidator *valRollCount = new QIntValidator(1, 100, this);
    ui->editRollCount->setValidator(valRollCount);
    QValidator *valDiceCount = new QIntValidator(1, 100, this);
    ui->editDiceCount->setValidator(valDiceCount);
    QValidator *valDiceType = new QIntValidator(1, 100, this);
    ui->editDiceType->setValidator(valDiceType);
    QValidator *valBonus = new QIntValidator(0, 100, this);
    ui->editBonus->setValidator(valBonus);
    QValidator *valTarget = new QIntValidator(0, 100, this);
    ui->editTarget->setValidator(valTarget);

    connect(ui->btnRoll, SIGNAL(clicked()), this, SLOT(rollDice()));
}
