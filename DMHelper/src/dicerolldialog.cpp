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
//    _combatants(),
//    _modifiers(),
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
//    _combatants(),
//    _modifiers(),
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

/*
DiceRollDialog::DiceRollDialog(const Dice& dice, const QList<BattleDialogModelCombatant*>& combatants, int rollDC, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiceRollDialog),
    _combatants(combatants),
    _modifiers(),
    _fireAndForget(false),
    _mouseDown(false),
    _mouseDownPos()
{
    ui->setupUi(this);
    init();

    if(_combatants.count() > 0)
    {
        ui->editRollCount->setText(QString::number(_combatants.count()));
        if(ui->editResult->viewport())
        {
            ui->editResult->viewport()->installEventFilter(this);
        }
    }
    else
    {
        ui->editRollCount->setText(QString("1"));
    }

    ui->editDiceCount->setText(QString::number(dice.getCount()));
    ui->editDiceType->setText(QString::number(dice.getType()));
    ui->editBonus->setText(QString::number(dice.getBonus()));

    ui->editTarget->setText(QString::number(rollDC));
}
*/

/*
DiceRollDialog::DiceRollDialog(const Dice& dice, const QList<BattleDialogModelCombatant*>& combatants, const QList<int>& modifiers, int rollDC, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiceRollDialog),
    _combatants(combatants),
    _modifiers(),
    _fireAndForget(false),
    _mouseDown(false),
    _mouseDownPos()
{
    ui->setupUi(this);
    init();

    if(_combatants.count() > 0)
    {
        ui->editRollCount->setText(QString::number(_combatants.count()));
        if(ui->editResult->viewport())
        {
            ui->editResult->viewport()->installEventFilter(this);
        }
    }
    else
    {
        ui->editRollCount->setText(QString("1"));
    }

    if(modifiers.count() == _combatants.count())
        _modifiers = modifiers;

    ui->editDiceCount->setText(QString::number(dice.getCount()));
    ui->editDiceType->setText(QString::number(dice.getType()));
    ui->editBonus->setText(QString::number(dice.getBonus()));

    ui->editTarget->setText(QString::number(rollDC));
}
*/

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
    //int randNum;
    int total = 0;

    for(int rc = 0; rc < rcEnd; ++rc)
    {
        int result = 0;
        QString resultStr;

        /*
        if((_combatants.count() > rc) && (_combatants.at(rc)))
        {
            resultStr = _combatants.at(rc)->getName();
            resultStr.append(QString(": "));
        }
        */

        // Go through and roll the dice, building up the string along the way
        for(int dc = 0; dc < dcEnd; ++dc)
        {
            //randNum = qrand();
            int roll = Dice::dX(diceType); //1 + (randNum * diceType)/RAND_MAX;
            if(dc > 0)
            {
                resultStr.append(QString(" + "));
            }
            resultStr.append(QString::number(roll));
            result += roll;
        }

        /*
        if(_modifiers.count() > 0)
        {
            result += _modifiers.at(rc);
            resultStr.append(QString(" + ") + QString::number(_modifiers.at(rc)));
        }
        */

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

void DiceRollDialog::hideEvent(QHideEvent * event)
{
    Q_UNUSED(event);

    if(_fireAndForget)
    {
        deleteLater();
    }
}

/*
bool DiceRollDialog::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if(mouseEvent)
        {
            _mouseDown = true;
            _mouseDownPos = mouseEvent->pos();
        }
    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        if(_mouseDown)
        {
            QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if((mouseEvent) && (_mouseDownPos == mouseEvent->pos()))
            {
                QTextCursor cursor = ui->editResult->textCursor();
                if((cursor.blockNumber() >= 0) && (cursor.blockNumber() < _combatants.count()))
                    emit selectCombatant(_combatants.at(cursor.blockNumber()));
            }

            _mouseDown = false;
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}
*/

/*
void DiceRollDialog::diceTypeChanged()
{
    ui->cmbType->setEnabled(ui->editDiceType->text().toInt() == 20);
}

void DiceRollDialog::modifierTypeChanged()
{
    _modifiers.clear();

    AbilitySkillPair abilitySkillPair(-1, -1);
    QVariant currentDataValue = ui->cmbType->currentData();
    if(currentDataValue.isValid())
        abilitySkillPair = currentDataValue.value<AbilitySkillPair>();

    for(BattleDialogModelCombatant* combatant : _combatants)
    {
        int modifier = Combatant::getAbilityMod(combatant->getAbilityValue((Combatant::Ability)abilitySkillPair.first));
        if((combatant->getCombatant()) && (combatant->getCombatant()->getType() == DMHelper::CombatantType_Character))
        {
            Character* character = dynamic_cast<Character*>(combatant->getCombatant());
            if((character) && (character->getBoolValue((Character::BoolValue)abilitySkillPair.second)))
            {
                modifier += character->getProficiencyBonus();
            }
        }
        _modifiers.append(modifier);
    }
}
*/

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
    /*
    connect(ui->editDiceType, SIGNAL(textChanged(QString)), this, SLOT(diceTypeChanged()));
    connect(ui->cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(modifierTypeChanged()));

    //ui->cmbType->addItem.setLabel(QString("Select the Saving Throw or Skill:"));
    ui->cmbType->addItem(QString("None"), QVariant());
    ui->cmbType->addItem(QString("Strength Check"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Strength, -1)));
    ui->cmbType->addItem(QString("   Strength Save"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Strength, Character::BoolValue_strengthSave)));
    ui->cmbType->addItem(QString("   Athletics"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Strength, Character::BoolValue_athletics)));
    ui->cmbType->insertSeparator(ui->cmbType->count());
    ui->cmbType->addItem(QString("Dexterity Check"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Dexterity, -1)));
    ui->cmbType->addItem(QString("   Dexterity Save"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Dexterity, Character::BoolValue_dexteritySave)));
    ui->cmbType->addItem(QString("   Stealth"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Dexterity, Character::BoolValue_stealth)));
    ui->cmbType->addItem(QString("   Acrobatics"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Dexterity, Character::BoolValue_acrobatics)));
    ui->cmbType->addItem(QString("   Sleight Of Hand"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Dexterity, Character::BoolValue_sleightOfHand)));
    ui->cmbType->insertSeparator(ui->cmbType->count());
    ui->cmbType->addItem(QString("Constitution Check"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Constitution, -1)));
    ui->cmbType->addItem(QString("   Constitution Save"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Constitution, Character::BoolValue_constitutionSave)));
    ui->cmbType->insertSeparator(ui->cmbType->count());
    ui->cmbType->addItem(QString("Intelligence Check"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Intelligence, -1)));
    ui->cmbType->addItem(QString("   Intelligence Save"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Intelligence, Character::BoolValue_intelligenceSave)));
    ui->cmbType->addItem(QString("   Investigation"),QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Intelligence, Character::BoolValue_investigation)));
    ui->cmbType->addItem(QString("   Arcana"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Intelligence, Character::BoolValue_arcana)));
    ui->cmbType->addItem(QString("   Nature"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Intelligence, Character::BoolValue_nature)));
    ui->cmbType->addItem(QString("   History"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Intelligence, Character::BoolValue_history)));
    ui->cmbType->addItem(QString("   Religion"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Intelligence, Character::BoolValue_religion)));
    ui->cmbType->insertSeparator(ui->cmbType->count());
    ui->cmbType->addItem(QString("Wisdom Check"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Wisdom, -1)));
    ui->cmbType->addItem(QString("   Wisdom Save"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Wisdom, Character::BoolValue_wisdomSave)));
    ui->cmbType->addItem(QString("   Medicine"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Wisdom, Character::BoolValue_medicine)));
    ui->cmbType->addItem(QString("   Animal Handling"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Wisdom, Character::BoolValue_animalHandling)));
    ui->cmbType->addItem(QString("   Perception"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Wisdom, Character::BoolValue_perception)));
    ui->cmbType->addItem(QString("   Insight"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Wisdom, Character::BoolValue_insight)));
    ui->cmbType->addItem(QString("   Survival"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Wisdom, Character::BoolValue_survival)));
    ui->cmbType->insertSeparator(ui->cmbType->count());
    ui->cmbType->addItem(QString("Charisma Check"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Charisma, -1)));
    ui->cmbType->addItem(QString("   Charisma Save"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Charisma, Character::BoolValue_charismaSave)));
    ui->cmbType->addItem(QString("   Performance"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Charisma, Character::BoolValue_performance)));
    ui->cmbType->addItem(QString("   Deception"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Charisma, Character::BoolValue_deception)));
    ui->cmbType->addItem(QString("   Persuasion"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Charisma, Character::BoolValue_persuasion)));
    ui->cmbType->addItem(QString("   Intimidation"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Charisma, Character::BoolValue_intimidation)));

    ui->cmbType->setCurrentIndex(0);
    ui->cmbType->setEnabled(ui->editDiceType->text().toInt() == 20);
    */
}
