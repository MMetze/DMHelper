#include "partycharactergridframe.h"
#include "ui_partycharactergridframe.h"
#include "characterv2.h"
#include "character.h"
#include "dmconstants.h"
#include "characterimporter.h"

// TODO: make this scalable with screen size

PartyCharacterGridFrame::PartyCharacterGridFrame(Characterv2& character, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PartyCharacterGridFrame),
    _character(character)
{
    ui->setupUi(this);

    ui->edtName->installEventFilter(this);
    ui->edtRace->installEventFilter(this);
    ui->edtRace->viewport()->installEventFilter(this);

    connect(ui->btnUpdate, &QAbstractButton::clicked, this, &PartyCharacterGridFrame::syncDndBeyond);

    readCharacter();
}

PartyCharacterGridFrame::~PartyCharacterGridFrame()
{
    delete ui;
}

void PartyCharacterGridFrame::readCharacter()
{
    // HACK - should be a template
    ui->edtName->setText(_character.getName());
    ui->edtRace->setText(_character.getStringValue(QString("race")) + QString(" ") + _character.getStringValue(QString("class")));
    ui->edtHitPoints->setText(QString::number(_character.getHitPoints()));
    ui->edtArmorClass->setText(QString::number(_character.getArmorClass()));
    ui->edtInitiative->setText(QString::number(_character.getInitiative()));
    ui->edtSpeed->setText(QString::number(_character.getSpeed()));
    ui->edtPassivePerception->setText(QString::number(_character.getIntValue(QString("passiveperception"))));

    ui->edtStr->setText(QString::number(_character.getStrength()) + " (" + Character::getAbilityModStr(_character.getStrength()) + ")");
    ui->edtDex->setText(QString::number(_character.getDexterity()) + " (" + Character::getAbilityModStr(_character.getDexterity()) + ")");
    ui->edtCon->setText(QString::number(_character.getConstitution()) + " (" + Character::getAbilityModStr(_character.getConstitution()) + ")");
    ui->edtInt->setText(QString::number(_character.getIntelligence()) + " (" + Character::getAbilityModStr(_character.getIntelligence()) + ")");
    ui->edtWis->setText(QString::number(_character.getWisdom()) + " (" + Character::getAbilityModStr(_character.getWisdom()) + ")");
    ui->edtCha->setText(QString::number(_character.getCharisma()) + " (" + Character::getAbilityModStr(_character.getCharisma()) + ")");

    QPixmap iconPixmap = _character.getIconPixmap(DMHelper::PixmapSize_Animate);
    Combatant::drawConditions(&iconPixmap, _character.getConditions());
    ui->lblIcon->setPixmap(iconPixmap.scaled(88, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lblIcon->setEnabled(_character.getBoolValue(QString("active")));

    ui->btnUpdate->setVisible(_character.getDndBeyondID() > 0);
    ui->edtName->setCursorPosition(0);
    ui->edtRace->moveCursor(QTextCursor::Start);
    ui->edtRace->ensureCursorVisible();
}

QUuid PartyCharacterGridFrame::getId() const
{
    return _character.getID();
}

void PartyCharacterGridFrame::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit characterSelected(getId());
}

bool PartyCharacterGridFrame::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if((event) && (event->type() == QEvent::MouseButtonDblClick))
    {
        event->accept();
        emit characterSelected(getId());
        return true;
    }

    return QFrame::eventFilter(obj, event);
}

void PartyCharacterGridFrame::syncDndBeyond()
{
    // HACK - needs to be fixed
    /*
    CharacterImporter* importer = new CharacterImporter();
    connect(importer, &CharacterImporter::characterImported, this, &PartyCharacterGridFrame::readCharacter);
    importer->updateCharacter(&_character);
    */
}
