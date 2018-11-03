#include "dicerollframe.h"
#include "ui_dicerollframe.h"
#include <QtGlobal>
#include <QIntValidator>

DiceRollFrame::DiceRollFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DiceRollFrame)
{
    ui->setupUi(this);
    init();
}

DiceRollFrame::DiceRollFrame(const Dice& dice, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DiceRollFrame)
{
    ui->setupUi(this);
    init();

    ui->editRollCount->setText(QString("1"));
    ui->editDiceCount->setText(QString::number(dice.getCount()));
    ui->editDiceType->setText(QString::number(dice.getType()));
    ui->editBonus->setText(QString::number(dice.getBonus()));

    rollDice();
}

DiceRollFrame::~DiceRollFrame()
{
    delete ui;
}

void DiceRollFrame::rollDice()
{
    ui->editResult->clear();

    int rcEnd = ui->editRollCount->text().toInt();
    int dcEnd = ui->editDiceCount->text().toInt();
    int diceType = ui->editDiceType->text().toInt();
    int target = ui->editTarget->text().toInt();
    int bonus = ui->editBonus->text().toInt();
    int randNum;
    int total = 0;

    for(int rc = 0; rc < rcEnd; ++rc)
    {
        int roll;
        int result = 0;
        QString resultStr;

        // Go through and roll the dice, building up the string along the way
        for(int dc = 0; dc < dcEnd; ++dc)
        {
            randNum = qrand();
            roll = 1 + (randNum * diceType)/RAND_MAX;
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

    ui->edtTotal->setText(QString::number(total));
}

void DiceRollFrame::init()
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

    connect(ui->btnRoll,SIGNAL(clicked()),this,SLOT(rollDice()));
}
