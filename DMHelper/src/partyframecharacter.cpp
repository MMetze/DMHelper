#include "partyframecharacter.h"
#include "ui_partyframecharacter.h"
#include "characterv2.h"
#include "dmconstants.h"

PartyFrameCharacter::PartyFrameCharacter(Characterv2& character, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PartyFrameCharacter)
{
    ui->setupUi(this);

    ui->lblIcon->setPixmap(character.getIconPixmap(DMHelper::PixmapSize_Animate).scaled(55, 75, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lblIcon->setEnabled(character.getBoolValue(QString("active")));
    ui->edtName->setText(character.getName());
    ui->edtRace->setText(character.getStringValue("race"));
    ui->edtClass->setText(character.getStringValue("class"));
    ui->edtHitPoints->setText(QString::number(character.getHitPoints()));
    ui->edtArmorClass->setText(QString::number(character.getArmorClass()));
    ui->edtInitiative->setText(QString::number(character.getInitiative()));
    ui->edtSpeed->setText(QString::number(character.getSpeed()));
    ui->edtPassivePerception->setText(QString::number(character.getIntValue(QString("passiveperception"))));

    ui->edtStr->setText(QString::number(character.getStrength()) + " (" + Characterv2::getAbilityModStr(character.getStrength()) + ")");
    ui->edtDex->setText(QString::number(character.getDexterity()) + " (" + Characterv2::getAbilityModStr(character.getDexterity()) + ")");
    ui->edtCon->setText(QString::number(character.getConstitution()) + " (" + Characterv2::getAbilityModStr(character.getConstitution()) + ")");
    ui->edtInt->setText(QString::number(character.getIntelligence()) + " (" + Characterv2::getAbilityModStr(character.getIntelligence()) + ")");
    ui->edtWis->setText(QString::number(character.getWisdom()) + " (" + Characterv2::getAbilityModStr(character.getWisdom()) + ")");
    ui->edtCha->setText(QString::number(character.getCharisma()) + " (" + Characterv2::getAbilityModStr(character.getCharisma()) + ")");
}

PartyFrameCharacter::~PartyFrameCharacter()
{
    delete ui;
}
