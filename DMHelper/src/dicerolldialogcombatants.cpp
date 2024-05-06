#include "dicerolldialogcombatants.h"
#include "ui_dicerolldialogcombatants.h"
#include "battlecombatantwidget.h"
#include "battledialogmodelcombatant.h"
#include "character.h"
#include "conditionseditdialog.h"
#include <QtGlobal>
#include <QMouseEvent>

DiceRollDialogCombatants::DiceRollDialogCombatants(const Dice& dice, const QList<BattleDialogModelCombatant*>& combatants, int rollDC, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiceRollDialogCombatants),
    _combatantLayout(nullptr),
    _combatants(combatants),
    _modifiers(),
    _fireAndForget(false),
    _conditions(0),
    _mouseDown(false),
    _mouseDownPos()
{
    ui->setupUi(this);
    init();

    _combatantLayout = new QVBoxLayout;
    _combatantLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ui->scrollAreaWidgetContents->setLayout(_combatantLayout);

    ui->edtDamage->setValidator(new QIntValidator(0, INT_MAX, this));

    ui->editDiceCount->setText(QString::number(dice.getCount()));
    ui->editDiceType->setText(QString::number(dice.getType()));
    ui->editBonus->setText(QString::number(dice.getBonus()));
    ui->edtTarget->setText(QString::number(rollDC));

    createCombatantWidgets();

    connect(ui->chkIncludeDead, SIGNAL(clicked(bool)), this, SLOT(setWidgetVisibility()));
}

DiceRollDialogCombatants::~DiceRollDialogCombatants()
{
    QLayoutItem *child;
    while ((child = _combatantLayout->takeAt(0)) != nullptr) {
        delete child;
    }

    delete ui;
}

void DiceRollDialogCombatants::fireAndForget()
{
    show();
    _fireAndForget = true;
}

void DiceRollDialogCombatants::rollDice()
{
    if(_combatants.count() != _combatantLayout->count())
        return;

    for(int rc = 0; rc < _combatants.count(); ++rc)
    {
        QLayoutItem* layoutItem = _combatantLayout->itemAt(rc);
        if(layoutItem)
        {
            BattleCombatantWidget* combatant = qobject_cast<BattleCombatantWidget*>(layoutItem->widget());
            if((combatant) && (combatant->isActive()))
            {
                rollForWidget(combatant, readDice(), (_modifiers.count() > 0) ? (_modifiers.at(rc)) : 0);
            }
        }
    }
}

void DiceRollDialogCombatants::applyDamage()
{
    if(_combatants.count() != _combatantLayout->count())
        return;

    int target = ui->edtTarget->text().toInt();
    int damage = ui->edtDamage->text().toInt();

    for(int rc = 0; rc < _combatants.count(); ++rc)
    {
        QLayoutItem* layoutItem = _combatantLayout->itemAt(rc);
        if(layoutItem)
        {
            BattleCombatantWidget* combatantWidget = qobject_cast<BattleCombatantWidget*>(layoutItem->widget());
            if((combatantWidget) && (combatantWidget->isActive()) && (combatantWidget->isVisible()))
            {
                if(combatantWidget->getResult() >= target)
                {
                    if(ui->chkHalfDamage->isChecked())
                        combatantWidget->applyDamage(damage / 2);
                }
                else
                {
                    combatantWidget->applyDamage(damage);
                    if(_conditions != 0)
                        combatantWidget->applyConditions(_conditions);
                }
            }
        }
    }
}

void DiceRollDialogCombatants::rerollWidget(BattleCombatantWidget* widget)
{
    if(!widget)
        return;

    int modifier = 0;
    if(_modifiers.count() > 0)
    {
        int index = _combatantLayout->indexOf(widget);
        if((index >= 0) && index < _modifiers.count())
            modifier = _modifiers.at(index);
    }

    rollForWidget(widget, readDice(), modifier);
}

void DiceRollDialogCombatants::setWidgetVisibility()
{
    if(_combatants.count() != _combatantLayout->count())
        return;

    for(int rc = 0; rc < _combatants.count(); ++rc)
    {
        BattleDialogModelCombatant* combatant = _combatants.at(rc);
        QLayoutItem* layoutItem = _combatantLayout->itemAt(rc);
        if(combatant && layoutItem && (layoutItem->widget()))
        {
            layoutItem->widget()->setVisible(ui->chkIncludeDead->isChecked() || combatant->getHitPoints() > 0);
        }
    }
}

void DiceRollDialogCombatants::hideEvent(QHideEvent * event)
{
    Q_UNUSED(event);

    if(_fireAndForget)
    {
        deleteLater();
    }
}

void DiceRollDialogCombatants::diceTypeChanged()
{
    ui->cmbType->setEnabled(ui->editDiceType->text().toInt() == 20);
}

void DiceRollDialogCombatants::modifierTypeChanged()
{
    _modifiers.clear();

    AbilitySkillPair abilitySkillPair(-1, -1);
    QVariant currentDataValue = ui->cmbType->currentData();
    if(currentDataValue.isValid())
        abilitySkillPair = currentDataValue.value<AbilitySkillPair>();

    for(BattleDialogModelCombatant* combatant : _combatants)
    {
        int modifier = 0;
        if(abilitySkillPair.second == -1)
            modifier = Combatant::getAbilityMod(combatant->getAbilityValue(static_cast<Combatant::Ability>(abilitySkillPair.first)));
        else
            modifier = combatant->getSkillModifier(static_cast<Combatant::Skills>(abilitySkillPair.second));
        _modifiers.append(modifier);
    }
}

void DiceRollDialogCombatants::editConditions()
{
    ConditionsEditDialog dlg;
    dlg.setConditions(_conditions);
    int result = dlg.exec();
    if(result == QDialog::Accepted)
    {
        _conditions = dlg.getConditions();
    }
}

void DiceRollDialogCombatants::init()
{
    QValidator *valDiceCount = new QIntValidator(1, 100, this);
    ui->editDiceCount->setValidator(valDiceCount);
    QValidator *valDiceType = new QIntValidator(1, 100, this);
    ui->editDiceType->setValidator(valDiceType);
    QValidator *valBonus = new QIntValidator(0, 100, this);
    ui->editBonus->setValidator(valBonus);
    QValidator *valTarget = new QIntValidator(0, 100, this);
    ui->edtTarget->setValidator(valTarget);

    connect(ui->btnRoll, SIGNAL(clicked()), this, SLOT(rollDice()));
    connect(ui->editDiceType, SIGNAL(textChanged(QString)), this, SLOT(diceTypeChanged()));
    connect(ui->cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(modifierTypeChanged()));
    connect(ui->btnDamage, SIGNAL(clicked()), this, SLOT(applyDamage()));
    connect(ui->btnApplyConditions, SIGNAL(clicked()), this, SLOT(editConditions()));

    ui->cmbType->addItem(QString("None"), QVariant());
    ui->cmbType->addItem(QString("Strength Check"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Strength, -1)));
    ui->cmbType->addItem(QString("   Strength Save"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Strength, Combatant::Skills_strengthSave)));
    ui->cmbType->addItem(QString("   Athletics"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Strength, Combatant::Skills_athletics)));
    ui->cmbType->insertSeparator(ui->cmbType->count());
    ui->cmbType->addItem(QString("Dexterity Check"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Dexterity, -1)));
    ui->cmbType->addItem(QString("   Dexterity Save"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Dexterity, Combatant::Skills_dexteritySave)));
    ui->cmbType->addItem(QString("   Stealth"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Dexterity, Combatant::Skills_stealth)));
    ui->cmbType->addItem(QString("   Acrobatics"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Dexterity, Combatant::Skills_acrobatics)));
    ui->cmbType->addItem(QString("   Sleight Of Hand"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Dexterity, Combatant::Skills_sleightOfHand)));
    ui->cmbType->insertSeparator(ui->cmbType->count());
    ui->cmbType->addItem(QString("Constitution Check"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Constitution, -1)));
    ui->cmbType->addItem(QString("   Constitution Save"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Constitution, Combatant::Skills_constitutionSave)));
    ui->cmbType->insertSeparator(ui->cmbType->count());
    ui->cmbType->addItem(QString("Intelligence Check"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Intelligence, -1)));
    ui->cmbType->addItem(QString("   Intelligence Save"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Intelligence, Combatant::Skills_intelligenceSave)));
    ui->cmbType->addItem(QString("   Investigation"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Intelligence, Combatant::Skills_investigation)));
    ui->cmbType->addItem(QString("   Arcana"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Intelligence, Combatant::Skills_arcana)));
    ui->cmbType->addItem(QString("   Nature"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Intelligence, Combatant::Skills_nature)));
    ui->cmbType->addItem(QString("   History"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Intelligence, Combatant::Skills_history)));
    ui->cmbType->addItem(QString("   Religion"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Intelligence, Combatant::Skills_religion)));
    ui->cmbType->insertSeparator(ui->cmbType->count());
    ui->cmbType->addItem(QString("Wisdom Check"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Wisdom, -1)));
    ui->cmbType->addItem(QString("   Wisdom Save"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Wisdom, Combatant::Skills_wisdomSave)));
    ui->cmbType->addItem(QString("   Medicine"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Wisdom, Combatant::Skills_medicine)));
    ui->cmbType->addItem(QString("   Animal Handling"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Wisdom, Combatant::Skills_animalHandling)));
    ui->cmbType->addItem(QString("   Perception"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Wisdom, Combatant::Skills_perception)));
    ui->cmbType->addItem(QString("   Insight"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Wisdom, Combatant::Skills_insight)));
    ui->cmbType->addItem(QString("   Survival"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Wisdom, Combatant::Skills_survival)));
    ui->cmbType->insertSeparator(ui->cmbType->count());
    ui->cmbType->addItem(QString("Charisma Check"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Charisma, -1)));
    ui->cmbType->addItem(QString("   Charisma Save"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Charisma, Combatant::Skills_charismaSave)));
    ui->cmbType->addItem(QString("   Performance"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Charisma, Combatant::Skills_performance)));
    ui->cmbType->addItem(QString("   Deception"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Charisma, Combatant::Skills_deception)));
    ui->cmbType->addItem(QString("   Persuasion"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Charisma, Combatant::Skills_persuasion)));
    ui->cmbType->addItem(QString("   Intimidation"), QVariant::fromValue(AbilitySkillPair(Combatant::Ability_Charisma, Combatant::Skills_intimidation)));

    ui->cmbType->setCurrentIndex(0);
    ui->cmbType->setEnabled(ui->editDiceType->text().toInt() == 20);

}

void DiceRollDialogCombatants::createCombatantWidgets()
{
    if(_combatantLayout->count() > 0)
        return;

    for(BattleDialogModelCombatant* combatant : _combatants)
    {
        BattleCombatantWidget* newWidget = new BattleCombatantWidget(combatant);
        _combatantLayout->addWidget(newWidget);
        connect(newWidget, SIGNAL(selectCombatant(BattleDialogModelCombatant*)), this, SIGNAL(selectCombatant(BattleDialogModelCombatant*)));
        connect(newWidget, SIGNAL(combatantChanged(BattleDialogModelCombatant*)), this, SIGNAL(combatantChanged(BattleDialogModelCombatant*)));
        connect(newWidget, SIGNAL(rerollNeeded(WidgetBattleCombatant*)), this, SLOT(rerollWidget(WidgetBattleCombatant*)));
        connect(newWidget, SIGNAL(hitPointsChanged(BattleDialogModelCombatant*, int)), this, SIGNAL(hitPointsChanged(BattleDialogModelCombatant*, int)));
        newWidget->setVisible(ui->chkIncludeDead->isChecked() || combatant->getHitPoints() > 0);
    }
}

int DiceRollDialogCombatants::rollOnce(const Dice& dice, int modifier, QString& resultStr)
{
    int result = 0;

    // Go through and roll the dice, building up the string along the way
    for(int dc = 0; dc < dice.getCount(); ++dc)
    {
        //int roll = 1 + (qrand() * dice.getType())/RAND_MAX;
        int roll = Dice::dX(dice.getType()); //1 + (qrand() * dice.getType())/RAND_MAX;
        if(dc > 0)
        {
            resultStr.append(QString(" + "));
        }
        resultStr.append(QString::number(roll));
        result += roll;
    }

    if(modifier != 0)
    {
        result += modifier;
        resultStr.append(QString(" + ") + QString::number(modifier));
    }

    // Add the bonus number, if it exists
    if(dice.getBonus() > 0)
    {
        resultStr.append(QString(" + ") + QString::number(dice.getBonus()));
        result += dice.getBonus();
    }

    // If there was somehow more than one number shown, then we should bother showing the overall sum
    if((dice.getCount() > 1) || (dice.getBonus() > 0) || (modifier != 0))
    {
        resultStr.append(QString(" = ") + QString::number(result));
    }

    return result;
}

void DiceRollDialogCombatants::rollForWidget(BattleCombatantWidget* widget, const Dice& dice, int modifier)
{
    if(!widget)
        return;

    QString resultStr;
    int roll1 = rollOnce(dice, modifier, resultStr);
    int roll2 = 0;
    if((widget->hasAdvantage()) || (widget->hasDisadvantage()))
    {
        resultStr.append(QString("<br>"));
        roll2 = rollOnce(dice, modifier, resultStr);
    }

    // Set the text color based on whether or not we exceeded the target
    int result = roll1;
    if(widget->hasAdvantage())
        result = qMax(roll1, roll2);
    else if(widget->hasDisadvantage())
        result = qMin(roll1, roll2);
    else
        result = roll1;

    int target = ui->edtTarget->text().toInt();
    if(result >= target)
    {
        resultStr.prepend(QString("<font color=""#00ff00"">"));
        resultStr.append(QString("</font>\n"));
    }
    else
    {
        resultStr.prepend(QString("<font color=""#ff0000"">"));
        resultStr.append(QString("</font>\n"));
    }

    // Add this result to the text
    widget->setResult(resultStr);
    widget->setResult(result);
}

Dice DiceRollDialogCombatants::readDice()
{
    return Dice(ui->editDiceCount->text().toInt(),
                ui->editDiceType->text().toInt(),
                ui->editBonus->text().toInt());
}
