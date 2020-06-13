#include "partyframecharacter.h"
#include "ui_partyframecharacter.h"
#include "character.h"
#include "dmconstants.h"

PartyFrameCharacter::PartyFrameCharacter(Character& character, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PartyFrameCharacter)
{
    ui->setupUi(this);

    ui->lblIcon->setPixmap(character.getIconPixmap(DMHelper::PixmapSize_Animate).scaled(55, 75, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lblIcon->setEnabled(character.getActive());
    ui->edtName->setText(character.getName());
    ui->edtRace->setText(character.getStringValue(Character::StringValue_race));
    ui->edtClass->setText(character.getStringValue(Character::StringValue_class));
    ui->edtHitPoints->setText(QString::number(character.getHitPoints()));
    ui->edtArmorClass->setText(QString::number(character.getArmorClass()));
    ui->edtInitiative->setText(QString::number(character.getInitiative()));
    ui->edtSpeed->setText(QString::number(character.getIntValue(Character::IntValue_speed)));
    ui->edtPassivePerception->setText(QString::number(character.getPassivePerception()));

    ui->edtStr->setText(QString::number(character.getStrength()) + " (" + Character::getAbilityModStr(character.getStrength()) + ")");
    ui->edtDex->setText(QString::number(character.getDexterity()) + " (" + Character::getAbilityModStr(character.getDexterity()) + ")");
    ui->edtCon->setText(QString::number(character.getConstitution()) + " (" + Character::getAbilityModStr(character.getConstitution()) + ")");
    ui->edtInt->setText(QString::number(character.getIntelligence()) + " (" + Character::getAbilityModStr(character.getIntelligence()) + ")");
    ui->edtWis->setText(QString::number(character.getWisdom()) + " (" + Character::getAbilityModStr(character.getWisdom()) + ")");
    ui->edtCha->setText(QString::number(character.getCharisma()) + " (" + Character::getAbilityModStr(character.getCharisma()) + ")");
}

PartyFrameCharacter::~PartyFrameCharacter()
{
    delete ui;
}
