#include "expertisedialog.h"
#include "ui_expertisedialog.h"

ExpertiseDialog::ExpertiseDialog(Character& character, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExpertiseDialog),
    _character(character),
    _checkMap()
{
    ui->setupUi(this);

    _checkMap.insert(ui->chkAcrobatics, Combatant::Skills_acrobatics);
    _checkMap.insert(ui->chkArcana, Combatant::Skills_arcana);
    _checkMap.insert(ui->chkHistory, Combatant::Skills_history);
    _checkMap.insert(ui->chkPerception, Combatant::Skills_perception);
    _checkMap.insert(ui->chkInsight, Combatant::Skills_insight);
    _checkMap.insert(ui->chkInvestigation, Combatant::Skills_investigation);
    _checkMap.insert(ui->chkStealth, Combatant::Skills_stealth);
    _checkMap.insert(ui->chkPerformance, Combatant::Skills_performance);
    _checkMap.insert(ui->chkAnimalHandling, Combatant::Skills_animalHandling);
    _checkMap.insert(ui->chkReligion, Combatant::Skills_religion);
    _checkMap.insert(ui->chkIntimidation, Combatant::Skills_intimidation);
    _checkMap.insert(ui->chkNature, Combatant::Skills_nature);
    _checkMap.insert(ui->chkMedicine, Combatant::Skills_medicine);
    _checkMap.insert(ui->chkPersuasion, Combatant::Skills_persuasion);
    _checkMap.insert(ui->chkSurvival, Combatant::Skills_survival);
    _checkMap.insert(ui->chkDeception, Combatant::Skills_deception);
    _checkMap.insert(ui->chkSleightOfHand, Combatant::Skills_sleightOfHand);
    _checkMap.insert(ui->chkAthletics, Combatant::Skills_athletics);

    prepareCheckboxes();
}

ExpertiseDialog::~ExpertiseDialog()
{
    delete ui;
}

void ExpertiseDialog::prepareCheckboxes()
{
    QMap<QCheckBox*, Combatant::Skills>::const_iterator i = _checkMap.begin();
    while (i != _checkMap.end())
    {
        if(_character.getSkillValue(i.value()))
        {
            i.key()->setEnabled(true);
            i.key()->setChecked(_character.getSkillExpertise(i.value()));
            connect(i.key(), &QAbstractButton::clicked, this, &ExpertiseDialog::skillChecked);
        }
        else
        {
            i.key()->setEnabled(false);
        }
        ++i;
    }

    ui->chkJackOfAllTrades->setChecked(_character.getIntValue(Character::IntValue_jackofalltrades));
    connect(ui->chkJackOfAllTrades, &QAbstractButton::clicked, this, &ExpertiseDialog::jackChecked);
}

void ExpertiseDialog::skillChecked()
{
    QCheckBox* senderObject = dynamic_cast<QCheckBox*>(sender());
    if(!senderObject)
        return;

    Combatant::Skills skill = _checkMap.value(senderObject);
    _character.setSkillExpertise(skill, senderObject->isChecked());
}

void ExpertiseDialog::jackChecked()
{
    _character.setIntValue(Character::IntValue_jackofalltrades, ui->chkJackOfAllTrades->isChecked() ? 1 : 0);
}
