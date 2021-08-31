#include "characterframe.h"
#include "ui_characterframe.h"
#include "characterimporter.h"
#include "scaledpixmap.h"
#include "expertisedialog.h"
#include "conditionseditdialog.h"
#include "quickref.h"
#include <QCheckBox>
#include <QMouseEvent>
#include <QFileDialog>
#include <QIntValidator>
#include <QGridLayout>
#include <QDebug>

const int CONDITION_FRAME_SPACING = 8;

// TODO: automate character level, next level exp, proficiency bonus

CharacterFrame::CharacterFrame(QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::CharacterFrame),
    _character(nullptr),
    _mouseDown(false),
    _reading(false),
    _rotation(0),
    _conditionGrid(nullptr)
{
    ui->setupUi(this);

    ui->edtArmorClass->setValidator(new QIntValidator(0,100,this));
    ui->edtInitiative->setValidator(new QIntValidator(-10,100,this));
    ui->edtPassivePerception->setValidator(new QIntValidator(0,100,this));
    ui->edtStr->setValidator(new QIntValidator(0,100,this));
    ui->edtDex->setValidator(new QIntValidator(0,100,this));
    ui->edtCon->setValidator(new QIntValidator(0,100,this));
    ui->edtInt->setValidator(new QIntValidator(0,100,this));
    ui->edtWis->setValidator(new QIntValidator(0,100,this));
    ui->edtCha->setValidator(new QIntValidator(0,100,this));
    ui->edtExperience->setValidator(new QIntValidator(0,1000000,this));
    ui->edtSpeed->setValidator(new QIntValidator(0,1000,this));
    ui->edtProficiencyBonus->setValidator(new QIntValidator(-10,100,this));
    ui->edtLevel->setValidator(new QIntValidator(0,100,this));

    connect(ui->btnSync, &QAbstractButton::clicked, this, &CharacterFrame::syncDndBeyond);
    enableDndBeyondSync(false);

    connectChanged(true);

    connect(ui->btnExpertise,SIGNAL(clicked()),this,SLOT(openExpertiseDialog()));

    connect(ui->edtName,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtLevel,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtRace,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtExperience,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtSize,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtClass,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtHitPoints,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtHitPointsMax,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtArmorClass,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtInitiative,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtSpeed,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtAlignment,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtBackground,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtPassivePerception,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtStr,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtDex,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtCon,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtInt,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtWis,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtCha,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->edtProficiencyBonus,SIGNAL(editingFinished()),this,SLOT(writeCharacterData()));
    connect(ui->chkStrSave,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkAthletics,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkDexSave,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkStealth,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkAcrobatics,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkSleightOfHand,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkConSave,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkIntSave,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkInvestigation,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkArcana,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkNature,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkHistory,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkReligion,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkWisSave,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkMedicine,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkAnimalHandling,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkPerception,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkInsight,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkSurvival,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkChaSave,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkPerformance,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkDeception,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkPersuasion,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->chkIntimidation,SIGNAL(clicked()),this,SLOT(writeCharacterData()));
    connect(ui->edtFeatures,SIGNAL(textChanged()),this,SLOT(writeCharacterData()));
    connect(ui->edtEquipment,SIGNAL(textChanged()),this,SLOT(writeCharacterData()));
    connect(ui->edtSpells,SIGNAL(textChanged()),this,SLOT(writeCharacterData()));
    connect(ui->edtNotes,SIGNAL(textChanged()),this,SLOT(writeCharacterData()));

    connect(ui->btnEditConditions, &QAbstractButton::clicked, this, &CharacterFrame::editConditions);
}

CharacterFrame::~CharacterFrame()
{
    delete ui;
}

void CharacterFrame::activateObject(CampaignObjectBase* object)
{
    Character* character = dynamic_cast<Character*>(object);
    if(!character)
        return;

    if(_character != nullptr)
    {
        qDebug() << "[CharacterFrame] ERROR: New character object activated without deactivating the existing character object first!";
        deactivateObject();
    }

    setCharacter(character);
    connect(_character, &Character::nameChanged, this, &CharacterFrame::updateCharacterName);

    emit checkableChanged(false);
    emit setPublishEnabled(true);
}

void CharacterFrame::deactivateObject()
{
    if(!_character)
    {
        qDebug() << "[CharacterFrame] WARNING: Invalid (nullptr) character object deactivated!";
        return;
    }

    disconnect(_character, &Character::nameChanged, this, &CharacterFrame::updateCharacterName);
    writeCharacterData();
    setCharacter(nullptr);
}

void CharacterFrame::setCharacter(Character* character)
{
    if(_character == character)
        return;

    _character = character;
    readCharacterData();
    emit characterChanged();
}

void CharacterFrame::calculateMods()
{
    if(!_character)
        return;

    ui->lblStrMod->setText(Character::getAbilityModStr(ui->edtStr->text().toInt()));
    ui->lblDexMod->setText(Character::getAbilityModStr(ui->edtDex->text().toInt()));
    ui->lblConMod->setText(Character::getAbilityModStr(ui->edtCon->text().toInt()));
    ui->lblIntMod->setText(Character::getAbilityModStr(ui->edtInt->text().toInt()));
    ui->lblWisMod->setText(Character::getAbilityModStr(ui->edtWis->text().toInt()));
    ui->lblChaMod->setText(Character::getAbilityModStr(ui->edtCha->text().toInt()));

    ui->edtPassivePerception->setText(QString::number(_character->getPassivePerception()));

    updateCheckboxName(ui->chkStrSave, Combatant::Skills_strengthSave);
    updateCheckboxName(ui->chkAthletics, Combatant::Skills_athletics);
    updateCheckboxName(ui->chkDexSave, Combatant::Skills_dexteritySave);
    updateCheckboxName(ui->chkStealth, Combatant::Skills_stealth);
    updateCheckboxName(ui->chkAcrobatics, Combatant::Skills_acrobatics);
    updateCheckboxName(ui->chkSleightOfHand, Combatant::Skills_sleightOfHand);
    updateCheckboxName(ui->chkConSave, Combatant::Skills_constitutionSave);
    updateCheckboxName(ui->chkIntSave, Combatant::Skills_intelligenceSave);
    updateCheckboxName(ui->chkInvestigation, Combatant::Skills_investigation);
    updateCheckboxName(ui->chkArcana, Combatant::Skills_arcana);
    updateCheckboxName(ui->chkNature, Combatant::Skills_nature);
    updateCheckboxName(ui->chkHistory, Combatant::Skills_history);
    updateCheckboxName(ui->chkReligion, Combatant::Skills_religion);
    updateCheckboxName(ui->chkWisSave, Combatant::Skills_wisdomSave);
    updateCheckboxName(ui->chkMedicine, Combatant::Skills_medicine);
    updateCheckboxName(ui->chkAnimalHandling, Combatant::Skills_animalHandling);
    updateCheckboxName(ui->chkPerception, Combatant::Skills_perception);
    updateCheckboxName(ui->chkInsight, Combatant::Skills_insight);
    updateCheckboxName(ui->chkSurvival, Combatant::Skills_survival);
    updateCheckboxName(ui->chkChaSave, Combatant::Skills_charismaSave);
    updateCheckboxName(ui->chkPerformance, Combatant::Skills_performance);
    updateCheckboxName(ui->chkDeception, Combatant::Skills_deception);
    updateCheckboxName(ui->chkPersuasion, Combatant::Skills_persuasion);
    updateCheckboxName(ui->chkIntimidation, Combatant::Skills_intimidation);
}

void CharacterFrame::clear()
{
    _character = nullptr;

    QSignalBlocker blocker(this);
    _reading = true;

    ui->lblIcon->setPixmap(ScaledPixmap::defaultPixmap()->getPixmap(DMHelper::PixmapSize_Showcase));

    ui->edtName->setText(QString(""));
    ui->edtLevel->setText(QString(""));
    ui->edtRace->setText(QString(""));
    ui->edtExperience->setText(QString(""));
    ui->edtSize->setText(QString(""));
    ui->edtClass->setText(QString(""));
    ui->edtHitPoints->setText(QString(""));
    ui->edtHitPointsMax->setText(QString(""));
    ui->edtArmorClass->setText(QString(""));
    ui->edtInitiative->setText(QString(""));
    ui->edtSpeed->setText(QString(""));
    ui->edtAlignment->setText(QString(""));
    ui->edtBackground->setText(QString(""));

    ui->edtStr->setText(QString(""));
    ui->edtDex->setText(QString(""));
    ui->edtCon->setText(QString(""));
    ui->edtInt->setText(QString(""));
    ui->edtWis->setText(QString(""));
    ui->edtCha->setText(QString(""));

    ui->edtProficiencyBonus->setText(QString(""));
    ui->chkStrSave->setChecked(false);
    ui->chkAthletics->setChecked(false);
    ui->chkDexSave->setChecked(false);
    ui->chkStealth->setChecked(false);
    ui->chkAcrobatics->setChecked(false);
    ui->chkSleightOfHand->setChecked(false);
    ui->chkConSave->setChecked(false);
    ui->chkIntSave->setChecked(false);
    ui->chkInvestigation->setChecked(false);
    ui->chkArcana->setChecked(false);
    ui->chkNature->setChecked(false);
    ui->chkHistory->setChecked(false);
    ui->chkReligion->setChecked(false);
    ui->chkWisSave->setChecked(false);
    ui->chkMedicine->setChecked(false);
    ui->chkAnimalHandling->setChecked(false);
    ui->chkPerception->setChecked(false);
    ui->chkInsight->setChecked(false);
    ui->chkSurvival->setChecked(false);
    ui->chkChaSave->setChecked(false);
    ui->chkPerformance->setChecked(false);
    ui->chkDeception->setChecked(false);
    ui->chkPersuasion->setChecked(false);
    ui->chkIntimidation->setChecked(false);

    ui->edtFeatures->setText(QString(""));
    ui->edtEquipment->setText(QString(""));
    ui->edtSpells->setText(QString(""));
    ui->edtNotes->setText(QString(""));

    ui->lblStrMod->setText(QString(""));
    ui->lblDexMod->setText(QString(""));
    ui->lblConMod->setText(QString(""));
    ui->lblIntMod->setText(QString(""));
    ui->lblWisMod->setText(QString(""));
    ui->lblChaMod->setText(QString(""));

    ui->edtPassivePerception->setText(QString(""));

    ui->chkStrSave->setText(QString(""));
    ui->chkAthletics->setText(QString(""));
    ui->chkDexSave->setText(QString(""));
    ui->chkStealth->setText(QString(""));
    ui->chkAcrobatics->setText(QString(""));
    ui->chkSleightOfHand->setText(QString(""));
    ui->chkConSave->setText(QString(""));
    ui->chkIntSave->setText(QString(""));
    ui->chkInvestigation->setText(QString(""));
    ui->chkArcana->setText(QString(""));
    ui->chkNature->setText(QString(""));
    ui->chkHistory->setText(QString(""));
    ui->chkReligion->setText(QString(""));
    ui->chkWisSave->setText(QString(""));
    ui->chkMedicine->setText(QString(""));
    ui->chkAnimalHandling->setText(QString(""));
    ui->chkPerception->setText(QString(""));
    ui->chkInsight->setText(QString(""));
    ui->chkSurvival->setText(QString(""));
    ui->chkChaSave->setText(QString(""));
    ui->chkPerformance->setText(QString(""));
    ui->chkDeception->setText(QString(""));
    ui->chkPersuasion->setText(QString(""));
    ui->chkIntimidation->setText(QString(""));

    clearConditionGrid();

    enableDndBeyondSync(false);

    _reading = false;
}

void CharacterFrame::publishClicked(bool checked)
{
    Q_UNUSED(checked);
    handlePublishClicked();
}

void CharacterFrame::setRotation(int rotation)
{
    if(_rotation != rotation)
    {
        _rotation = rotation;
        emit rotationChanged(rotation);
    }
}

void CharacterFrame::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);
    ui->lblIcon->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _mouseDown = true;
}

void CharacterFrame::mouseReleaseEvent(QMouseEvent * event)
{
    if(!_mouseDown)
        return;

    ui->lblIcon->setFrameStyle(QFrame::Panel | QFrame::Raised);
    _mouseDown = false;

    if((!_character) || (!ui->lblIcon->frameGeometry().contains(event->pos())))
        return;

    QString filename = QFileDialog::getOpenFileName(this,QString("Select New Image..."));
    if(filename.isEmpty())
        return;

    _character->setIcon(filename);
    loadCharacterImage();
}

void CharacterFrame::readCharacterData()
{
    if(!_character)
        return;

    QSignalBlocker blocker(this);
    _reading = true;

    connectChanged(false);

    loadCharacterImage();

    ui->edtName->setText(_character->getName());
    ui->edtName->home(false);
    ui->edtLevel->setText(QString::number(_character->getIntValue(Character::IntValue_level)));
    ui->edtLevel->home(false);
    ui->edtRace->setText(_character->getStringValue(Character::StringValue_race));
    ui->edtRace->home(false);
    ui->edtSize->setText(_character->getStringValue(Character::StringValue_size));
    ui->edtSize->home(false);
    ui->edtExperience->setText(QString::number(_character->getIntValue(Character::IntValue_experience)));
    ui->edtExperience->home(false);
    ui->edtClass->setText(_character->getStringValue(Character::StringValue_class));
    ui->edtClass->home(false);
    ui->edtHitPoints->setText(QString::number(_character->getHitPoints()));
    ui->edtHitPoints->home(false);
    ui->edtHitPointsMax->setText(QString::number(_character->getIntValue(Character::IntValue_maximumHP)));
    ui->edtHitPointsMax->home(false);
    ui->edtArmorClass->setText(QString::number(_character->getArmorClass()));
    ui->edtArmorClass->home(false);
    ui->edtInitiative->setText(QString::number(_character->getInitiative()));
    ui->edtInitiative->home(false);
    ui->edtSpeed->setText(QString::number(_character->getIntValue(Character::IntValue_speed)));
    ui->edtSpeed->home(false);
    ui->edtAlignment->setText(_character->getStringValue(Character::StringValue_alignment));
    ui->edtAlignment->home(false);
    ui->edtBackground->setText(_character->getStringValue(Character::StringValue_background));
    ui->edtBackground->home(false);

    ui->edtStr->setText(QString::number(_character->getIntValue(Character::IntValue_strength)));
    ui->edtDex->setText(QString::number(_character->getIntValue(Character::IntValue_dexterity)));
    ui->edtCon->setText(QString::number(_character->getIntValue(Character::IntValue_constitution)));
    ui->edtInt->setText(QString::number(_character->getIntValue(Character::IntValue_intelligence)));
    ui->edtWis->setText(QString::number(_character->getIntValue(Character::IntValue_wisdom)));
    ui->edtCha->setText(QString::number(_character->getIntValue(Character::IntValue_charisma)));

    ui->edtProficiencyBonus->setText(QString::number(_character->getProficiencyBonus()));
    ui->edtProficiencyBonus->home(false);
    ui->chkStrSave->setChecked(_character->getSkillValue(Combatant::Skills_strengthSave));
    ui->chkAthletics->setChecked(_character->getSkillValue(Combatant::Skills_athletics));
    ui->chkDexSave->setChecked(_character->getSkillValue(Combatant::Skills_dexteritySave));
    ui->chkStealth->setChecked(_character->getSkillValue(Combatant::Skills_stealth));
    ui->chkAcrobatics->setChecked(_character->getSkillValue(Combatant::Skills_acrobatics));
    ui->chkSleightOfHand->setChecked(_character->getSkillValue(Combatant::Skills_sleightOfHand));
    ui->chkConSave->setChecked(_character->getSkillValue(Combatant::Skills_constitutionSave));
    ui->chkIntSave->setChecked(_character->getSkillValue(Combatant::Skills_intelligenceSave));
    ui->chkInvestigation->setChecked(_character->getSkillValue(Combatant::Skills_investigation));
    ui->chkArcana->setChecked(_character->getSkillValue(Combatant::Skills_arcana));
    ui->chkNature->setChecked(_character->getSkillValue(Combatant::Skills_nature));
    ui->chkHistory->setChecked(_character->getSkillValue(Combatant::Skills_history));
    ui->chkReligion->setChecked(_character->getSkillValue(Combatant::Skills_religion));
    ui->chkWisSave->setChecked(_character->getSkillValue(Combatant::Skills_wisdomSave));
    ui->chkMedicine->setChecked(_character->getSkillValue(Combatant::Skills_medicine));
    ui->chkAnimalHandling->setChecked(_character->getSkillValue(Combatant::Skills_animalHandling));
    ui->chkPerception->setChecked(_character->getSkillValue(Combatant::Skills_perception));
    ui->chkInsight->setChecked(_character->getSkillValue(Combatant::Skills_insight));
    ui->chkSurvival->setChecked(_character->getSkillValue(Combatant::Skills_survival));
    ui->chkChaSave->setChecked(_character->getSkillValue(Combatant::Skills_charismaSave));
    ui->chkPerformance->setChecked(_character->getSkillValue(Combatant::Skills_performance));
    ui->chkDeception->setChecked(_character->getSkillValue(Combatant::Skills_deception));
    ui->chkPersuasion->setChecked(_character->getSkillValue(Combatant::Skills_persuasion));
    ui->chkIntimidation->setChecked(_character->getSkillValue(Combatant::Skills_intimidation));

    ui->edtFeatures->setText(_character->getStringValue(Character::StringValue_proficiencies));
    ui->edtEquipment->setText(_character->getStringValue(Character::StringValue_equipment));
    ui->edtSpells->setText(_character->getStringValue(Character::StringValue_spells));
    ui->edtNotes->setText(_character->getStringValue(Character::StringValue_notes));

    updateConditionLayout();

    connectChanged(true);

    calculateMods();

    enableDndBeyondSync(_character->getDndBeyondID() != -1);

    _reading = false;

}

void CharacterFrame::writeCharacterData()
{
    if((_character) && (!_reading))
    {
        _character->beginBatchChanges();

        _character->setName(ui->edtName->text());
        _character->setIntValue(Character::IntValue_level, ui->edtLevel->text().toInt());
        _character->setStringValue(Character::StringValue_race, ui->edtRace->text());
        _character->setStringValue(Character::StringValue_size, ui->edtSize->text());
        _character->setIntValue(Character::IntValue_experience, ui->edtExperience->text().toInt());
        _character->setStringValue(Character::StringValue_class, ui->edtClass->text());
        _character->setHitPoints(ui->edtHitPoints->text().toInt());
        _character->setIntValue(Character::IntValue_maximumHP, ui->edtHitPointsMax->text().toInt());
        _character->setArmorClass(ui->edtArmorClass->text().toInt());
        _character->setInitiative(ui->edtInitiative->text().toInt());
        _character->setIntValue(Character::IntValue_speed, ui->edtSpeed->text().toInt());
        _character->setStringValue(Character::StringValue_alignment, ui->edtAlignment->text());
        _character->setStringValue(Character::StringValue_background, ui->edtBackground->text());

        _character->setIntValue(Character::IntValue_strength, ui->edtStr->text().toInt());
        _character->setIntValue(Character::IntValue_dexterity, ui->edtDex->text().toInt());
        _character->setIntValue(Character::IntValue_constitution, ui->edtCon->text().toInt());
        _character->setIntValue(Character::IntValue_intelligence, ui->edtInt->text().toInt());
        _character->setIntValue(Character::IntValue_wisdom, ui->edtWis->text().toInt());
        _character->setIntValue(Character::IntValue_charisma, ui->edtCha->text().toInt());

        _character->setSkillValue(Combatant::Skills_athletics, ui->chkAthletics->isChecked());
        _character->setSkillValue(Combatant::Skills_strengthSave, ui->chkStrSave->isChecked());
        _character->setSkillValue(Combatant::Skills_dexteritySave, ui->chkDexSave->isChecked());
        _character->setSkillValue(Combatant::Skills_stealth, ui->chkStealth->isChecked());
        _character->setSkillValue(Combatant::Skills_acrobatics, ui->chkAcrobatics->isChecked());
        _character->setSkillValue(Combatant::Skills_sleightOfHand, ui->chkSleightOfHand->isChecked());
        _character->setSkillValue(Combatant::Skills_constitutionSave, ui->chkConSave->isChecked());
        _character->setSkillValue(Combatant::Skills_intelligenceSave, ui->chkIntSave->isChecked());
        _character->setSkillValue(Combatant::Skills_investigation, ui->chkInvestigation->isChecked());
        _character->setSkillValue(Combatant::Skills_arcana, ui->chkArcana->isChecked());
        _character->setSkillValue(Combatant::Skills_nature, ui->chkNature->isChecked());
        _character->setSkillValue(Combatant::Skills_history, ui->chkHistory->isChecked());
        _character->setSkillValue(Combatant::Skills_religion, ui->chkReligion->isChecked());
        _character->setSkillValue(Combatant::Skills_wisdomSave, ui->chkWisSave->isChecked());
        _character->setSkillValue(Combatant::Skills_medicine, ui->chkMedicine->isChecked());
        _character->setSkillValue(Combatant::Skills_animalHandling, ui->chkAnimalHandling->isChecked());
        _character->setSkillValue(Combatant::Skills_perception, ui->chkPerception->isChecked());
        _character->setSkillValue(Combatant::Skills_insight, ui->chkInsight->isChecked());
        _character->setSkillValue(Combatant::Skills_survival, ui->chkSurvival->isChecked());
        _character->setSkillValue(Combatant::Skills_charismaSave, ui->chkChaSave->isChecked());
        _character->setSkillValue(Combatant::Skills_performance, ui->chkPerformance->isChecked());
        _character->setSkillValue(Combatant::Skills_deception, ui->chkDeception->isChecked());
        _character->setSkillValue(Combatant::Skills_persuasion, ui->chkPersuasion->isChecked());
        _character->setSkillValue(Combatant::Skills_intimidation, ui->chkIntimidation->isChecked());

        _character->setStringValue(Character::StringValue_proficiencies, ui->edtFeatures->toPlainText());
        _character->setStringValue(Character::StringValue_equipment, ui->edtEquipment->toPlainText());
        _character->setStringValue(Character::StringValue_spells, ui->edtSpells->toPlainText());
        _character->setStringValue(Character::StringValue_notes, ui->edtNotes->toPlainText());

        _character->endBatchChanges();

        calculateMods();
    }
}

void CharacterFrame::updateCharacterName()
{
    if(!_character)
        return;

    ui->edtName->setText(_character->getName());
}

void CharacterFrame::handlePublishClicked()
{
    if(!_character)
        return;

    QImage iconImg;
    QString iconFile = _character->getIcon();
    if(!iconImg.load(iconFile))
        iconImg = _character->getIconPixmap(DMHelper::PixmapSize_Full).toImage(); // .load(QString(":/img/data/portrait.png"));

    if(iconImg.isNull())
        return;

    if(_rotation != 0)
        iconImg = iconImg.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);

    emit publishCharacterImage(iconImg);
}

void CharacterFrame::syncDndBeyond()
{
    if(!_character)
        return;

    CharacterImporter* importer = new CharacterImporter();
    connect(importer, &CharacterImporter::characterImported, this, &CharacterFrame::readCharacterData);
    connect(this, &CharacterFrame::characterChanged, importer, &CharacterImporter::campaignChanged);
    importer->updateCharacter(_character);
}

void CharacterFrame::openExpertiseDialog()
{
    if(!_character)
        return;

    ExpertiseDialog dlg(*_character, this);
    dlg.exec();

    calculateMods();
    writeCharacterData();
}

void CharacterFrame::editConditions()
{
    if(!_character)
        return;

    ConditionsEditDialog dlg;
    dlg.setConditions(_character->getConditions());
    int result = dlg.exec();
    if(result == QDialog::Accepted)
    {
        if(dlg.getConditions() != _character->getConditions())
        {
            _character->setConditions(dlg.getConditions());
            updateConditionLayout();
        }
    }
}

void CharacterFrame::updateConditionLayout()
{
    clearConditionGrid();

    if(!_character)
        return;

    _conditionGrid = new QGridLayout;
    _conditionGrid->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    _conditionGrid->setContentsMargins(CONDITION_FRAME_SPACING, CONDITION_FRAME_SPACING, CONDITION_FRAME_SPACING, CONDITION_FRAME_SPACING);
    _conditionGrid->setSpacing(CONDITION_FRAME_SPACING);
    ui->scrollAreaWidgetContents->setLayout(_conditionGrid);

    int conditions = _character->getConditions();

    for(int i = 0; i < Combatant::getConditionCount(); ++i)
    {
        Combatant::Condition condition = Combatant::getConditionByIndex(i);
        if(conditions & condition)
            addCondition(condition);
    }

    int spacingColumn = _conditionGrid->columnCount();

    _conditionGrid->addItem(new QSpacerItem(20, 40, QSizePolicy::Expanding), 0, spacingColumn);

    for(int i = 0; i < spacingColumn; ++i)
        _conditionGrid->setColumnStretch(i, 1);

    _conditionGrid->setColumnStretch(spacingColumn, 10);

    update();
}

void CharacterFrame::clearConditionGrid()
{
    if(!_conditionGrid)
        return;

    qDebug() << "[BattleCombatantFrame] Clearing the condition grid";

    // Delete the grid entries
    QLayoutItem *child = nullptr;
    while((child = _conditionGrid->takeAt(0)) != nullptr)
    {
        delete child->widget();
        delete child;
    }

    delete _conditionGrid;
    _conditionGrid = nullptr;

    ui->scrollAreaWidgetContents->update();
}

void CharacterFrame::addCondition(Combatant::Condition condition)
{
    if(!_conditionGrid)
        return;

    QString resourceIcon = QString(":/img/data/img/") + Combatant::getConditionIcon(condition) + QString(".png");
    QLabel* conditionLabel = new QLabel(this);
    conditionLabel->setPixmap(QPixmap(resourceIcon).scaled(40, 40));

    QString conditionText = QString("<b>") + Combatant::getConditionDescription(condition) + QString("</b>");
    if(QuickRef::Instance())
    {
        QuickRefData* conditionData = QuickRef::Instance()->getData(QString("Condition"), 0, Combatant::getConditionTitle(condition));
        if(conditionData)
            conditionText += QString("<p>") + conditionData->getOverview();
    }
    conditionLabel->setToolTip(conditionText);

    int columnCount = (ui->scrollAreaWidgetContents->width() - CONDITION_FRAME_SPACING) / (40 + CONDITION_FRAME_SPACING);
    int row = _conditionGrid->count() / columnCount;
    int column = _conditionGrid->count() % columnCount;

    _conditionGrid->addWidget(conditionLabel, row, column);
}

void CharacterFrame::loadCharacterImage()
{
    if(_character)
        ui->lblIcon->setPixmap(_character->getIconPixmap(DMHelper::PixmapSize_Showcase));
}

void CharacterFrame::updateCheckboxName(QCheckBox* chk, Combatant::Skills skill)
{
    if(!_character)
        return;

    int skillBonus = _character->getSkillBonus(skill);

    QString chkName;
    if(skillBonus >= 0)
        chkName.append("+");
    chkName.append(QString::number(skillBonus));
    chk->setText(chkName);
}

void CharacterFrame::enableDndBeyondSync(bool enabled)
{
    ui->btnSync->setVisible(enabled);
    ui->lblDndBeyondLink->setVisible(enabled);

    if(_character)
    {
        QString characterUrl = QString("https://www.dndbeyond.com/characters/") + QString::number(_character->getDndBeyondID());
        QString fullLink = QString("<a href=\"") + characterUrl + QString("\">") + characterUrl + QString("</a>");
        qDebug() << "[CharacterFrame] Setting Dnd Beyond link for character to: " << fullLink;
        ui->lblDndBeyondLink->setText(fullLink);
    }
}

void CharacterFrame::connectChanged(bool makeConnection)
{
    if(makeConnection)
    {
        connect(ui->edtStr,SIGNAL(textChanged(QString)),this,SLOT(calculateMods()));
        connect(ui->edtDex,SIGNAL(textChanged(QString)),this,SLOT(calculateMods()));
        connect(ui->edtCon,SIGNAL(textChanged(QString)),this,SLOT(calculateMods()));
        connect(ui->edtInt,SIGNAL(textChanged(QString)),this,SLOT(calculateMods()));
        connect(ui->edtWis,SIGNAL(textChanged(QString)),this,SLOT(calculateMods()));
        connect(ui->edtCha,SIGNAL(textChanged(QString)),this,SLOT(calculateMods()));
        connect(ui->chkStrSave,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkAthletics,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkDexSave,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkStealth,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkAcrobatics,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkSleightOfHand,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkConSave,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkIntSave,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkInvestigation,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkArcana,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkNature,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkHistory,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkReligion,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkWisSave,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkMedicine,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkAnimalHandling,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkPerception,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkInsight,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkSurvival,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkChaSave,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkPerformance,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkDeception,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkPersuasion,SIGNAL(clicked()),this,SLOT(calculateMods()));
        connect(ui->chkIntimidation,SIGNAL(clicked()),this,SLOT(calculateMods()));
    }
    else
    {
        disconnect(ui->edtStr,SIGNAL(textChanged(QString)),this,SLOT(calculateMods()));
        disconnect(ui->edtDex,SIGNAL(textChanged(QString)),this,SLOT(calculateMods()));
        disconnect(ui->edtCon,SIGNAL(textChanged(QString)),this,SLOT(calculateMods()));
        disconnect(ui->edtInt,SIGNAL(textChanged(QString)),this,SLOT(calculateMods()));
        disconnect(ui->edtWis,SIGNAL(textChanged(QString)),this,SLOT(calculateMods()));
        disconnect(ui->edtCha,SIGNAL(textChanged(QString)),this,SLOT(calculateMods()));
        disconnect(ui->chkStrSave,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkAthletics,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkDexSave,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkStealth,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkAcrobatics,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkSleightOfHand,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkConSave,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkIntSave,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkInvestigation,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkArcana,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkNature,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkHistory,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkReligion,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkWisSave,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkMedicine,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkAnimalHandling,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkPerception,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkInsight,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkSurvival,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkChaSave,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkPerformance,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkDeception,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkPersuasion,SIGNAL(clicked()),this,SLOT(calculateMods()));
        disconnect(ui->chkIntimidation,SIGNAL(clicked()),this,SLOT(calculateMods()));
    }
}
