#include "partycharactergridframe.h"
#include "ui_partycharactergridframe.h"
#include "character.h"
#include "dmconstants.h"
#include "characterimporter.h"
#include <QDebug>

// TODO: make this scalable with screen size

PartyCharacterGridFrame::PartyCharacterGridFrame(Character& character, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PartyCharacterGridFrame),
    _character(character)
{
    ui->setupUi(this);

    connect(ui->btnUpdate, &QAbstractButton::clicked, this, &PartyCharacterGridFrame::syncDndBeyond);

    readCharacter();
}

PartyCharacterGridFrame::~PartyCharacterGridFrame()
{
    delete ui;
}

void PartyCharacterGridFrame::readCharacter()
{
    ui->edtName->setText(_character.getName());
//    ui->edtRace->setText(character.getStringValue(Character::StringValue_race));
//    ui->edtClass->setText(character.getStringValue(Character::StringValue_class));
    ui->edtRace->setText(_character.getStringValue(Character::StringValue_race) + QString(" ") + _character.getStringValue(Character::StringValue_class));
    ui->edtHitPoints->setText(QString::number(_character.getHitPoints()));
    ui->edtArmorClass->setText(QString::number(_character.getArmorClass()));
    ui->edtInitiative->setText(QString::number(_character.getInitiative()));
    ui->edtSpeed->setText(QString::number(_character.getIntValue(Character::IntValue_speed)));
    ui->edtPassivePerception->setText(QString::number(_character.getPassivePerception()));

    ui->edtStr->setText(QString::number(_character.getStrength()) + " (" + Character::getAbilityModStr(_character.getStrength()) + ")");
    ui->edtDex->setText(QString::number(_character.getDexterity()) + " (" + Character::getAbilityModStr(_character.getDexterity()) + ")");
    ui->edtCon->setText(QString::number(_character.getConstitution()) + " (" + Character::getAbilityModStr(_character.getConstitution()) + ")");
    ui->edtInt->setText(QString::number(_character.getIntelligence()) + " (" + Character::getAbilityModStr(_character.getIntelligence()) + ")");
    ui->edtWis->setText(QString::number(_character.getWisdom()) + " (" + Character::getAbilityModStr(_character.getWisdom()) + ")");
    ui->edtCha->setText(QString::number(_character.getCharisma()) + " (" + Character::getAbilityModStr(_character.getCharisma()) + ")");

    ui->lblIcon->setPixmap(_character.getIconPixmap(DMHelper::PixmapSize_Animate).scaled(88, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lblIcon->setEnabled(_character.getActive());

    ui->btnUpdate->setVisible(_character.getDndBeyondID() > 0);
    ui->edtName->setCursorPosition(0);
    ui->edtRace->moveCursor(QTextCursor::Start);
    ui->edtRace->ensureCursorVisible();
}

QUuid PartyCharacterGridFrame::getId() const
{
    return _character.getID();
}

void PartyCharacterGridFrame::syncDndBeyond()
{
    CharacterImporter* importer = new CharacterImporter();
    connect(importer, &CharacterImporter::characterImported, this, &PartyCharacterGridFrame::readCharacter);
    importer->updateCharacter(&_character);
}
