#include "dicerolldialog.h"
#include "ui_dicerolldialog.h"
#include "characterv2.h"
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
    int rcEnd = ui->editRollCount->text().toInt();
    int target = ui->editTarget->text().toInt();
    int total = 0;

    for(int rc = 0; rc < rcEnd; ++rc)
    {
        int result = 0;
        QString resultStr;

        if(ui->edtText->text().isEmpty())
            rollDiceSpecified(result, resultStr);
        else
            rollDiceString(result, resultStr);

        // Set the text color based on whether or not we exceeded the target
        if(result >= target)
            resultStr.prepend(QString("<font color=""#00ff00"">"));
        else
            resultStr.prepend(QString("<font color=""#ff0000"">"));

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
        deleteLater();
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
    connect(ui->btnClearHistory, &QAbstractButton::clicked, ui->editResult, &QTextEdit::clear);
}

void DiceRollDialog::rollDiceString(int& resultValue, QString& resultString)
{
    QString sourceString = ui->edtText->text();
    QString spacesOut = sourceString.remove(QChar(' '));
    QStringList rollStrings = spacesOut.split(QChar('+'));

    for(int i = 0; i < rollStrings.count(); ++i)
    {
        if(i > 0)
            resultString.append(QString(" + "));

        if(rollStrings.count() > 1)
            resultString.append(QChar('('));

        rollOnce(Dice(rollStrings.at(i)), resultValue, resultString);

        if(rollStrings.count() > 1)
            resultString.append(QChar(')'));
    }

    resultString.append(QString(" = ") + QString::number(resultValue));
}

void DiceRollDialog::rollDiceSpecified(int& resultValue, QString& resultString)
{
    int diceCount = ui->editDiceCount->text().toInt();
    int diceType = ui->editDiceType->text().toInt();
    int bonus = ui->editBonus->text().toInt();

    rollOnce(Dice(diceCount, diceType, bonus), resultValue, resultString);

    // If there was somehow more than one number shown, then we should bother showing the overall sum
    if((diceCount > 1) || (bonus > 0))
        resultString.append(QString(" = ") + QString::number(resultValue));
}

void DiceRollDialog::rollOnce(const Dice& dice, int& resultValue, QString& resultString)
{
    // Go through and roll the dice, building up the string along the way
    for(int dc = 0; dc < dice.getCount(); ++dc)
    {
        int roll = Dice::dX(dice.getType());
        if(dc > 0)
            resultString.append(QString(" + "));

        resultString.append(QString::number(roll));
        resultValue += roll;
    }

    // Add the bonus number, if it exists
    if(dice.getBonus() > 0)
    {
        if(dice.getCount() > 0)
            resultString.append(QString(" + "));
        resultString.append(QString::number(dice.getBonus()));
        resultValue += dice.getBonus();
    }
}
