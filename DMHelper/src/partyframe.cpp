#include "partyframe.h"
#include "ui_partyframe.h"
#include "party.h"
#include "character.h"
//#include "partyframecharacter.h"
#include "partycharactergridframe.h"
#include "characterimporter.h"
#include <QFileDialog>
#include <QMouseEvent>
#include <QDebug>

const int PARTY_FRAME_SPACING = 20;

PartyFrame::PartyFrame(QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::PartyFrame),
    //_characterLayout(nullptr),
    _characterGrid(nullptr),
    _characterFrames(),
    _party(nullptr),
    _mouseDown(false),
    _rotation(0),
    _layoutColumns(-1)
{
    ui->setupUi(this);

//    _characterLayout = new QVBoxLayout;
//    _characterLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
//    _characterLayout->setContentsMargins(3,3,3,3);
//    ui->scrollAreaWidgetContents->setLayout(_characterLayout);

    connect(ui->btnUpdateAll, &QAbstractButton::clicked, this, &PartyFrame::updateAll);

}

PartyFrame::~PartyFrame()
{
    //qDebug() << "[PartyFrame] being destroyed: " << _characterLayout->count() << " widgets";
    qDebug() << "[PartyFrame] being destroyed: " << (_characterGrid ? _characterGrid->count() : 0) << " widgets";

    /*
    QLayoutItem *child;
    //while ((child = _characterLayout->takeAt(0)) != nullptr)
    while ((child = _characterGrid->takeAt(0)) != nullptr)
    {
        delete child;
    }
    */

    clearList();

    delete ui;
}

void PartyFrame::activateObject(CampaignObjectBase* object)
{
    Party* party = dynamic_cast<Party*>(object);
    if(!party)
        return;

    if(_party != nullptr)
    {
        qDebug() << "[PartyFrame] ERROR: New party object activated without deactivating the existing party object first!";
        deactivateObject();
    }

    setParty(party);

    emit checkableChanged(false);
    emit setPublishEnabled(true);
}

void PartyFrame::deactivateObject()
{
    if(!_party)
    {
        qDebug() << "[PArtyFrame] WARNING: Invalid (nullptr) party object deactivated!";
        return;
    }

    setParty(nullptr);
}

void PartyFrame::setParty(Party* party)
{
    if(_party == party)
        return;

    _party = party;
    readPartyData();
    calculateThresholds();
    emit partyChanged();
}

void PartyFrame::publishClicked(bool checked)
{
    Q_UNUSED(checked);
    handlePublishClicked();
}

void PartyFrame::setRotation(int rotation)
{
    if(_rotation != rotation)
    {
        _rotation = rotation;
        emit rotationChanged(rotation);
    }
}

void PartyFrame::handleCharacterChanged(QUuid id)
{
    if(!_party)
        return;

    Character* character = dynamic_cast<Character*>(_party->searchChildrenById(id));
    if(!character)
        return;

    for(PartyCharacterGridFrame* frame : _characterFrames)
    {
        if((frame) && (frame->getId() == id))
            frame->readCharacter();
    }

    calculateThresholds();
}

void PartyFrame::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);

    if(!ui->lblIcon->frameGeometry().contains(event->pos()))
        return;

    ui->lblIcon->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _mouseDown = true;
}

void PartyFrame::mouseReleaseEvent(QMouseEvent * event)
{
    if(!_mouseDown)
        return;

    ui->lblIcon->setFrameStyle(QFrame::Panel | QFrame::Raised);
    _mouseDown = false;

    if((!_party) || (!ui->lblIcon->frameGeometry().contains(event->pos())))
        return;

    QString filename = QFileDialog::getOpenFileName(this,QString("Select New Image..."), QString(), QString("Image files (*.png *.jpg)"));
    if(filename.isEmpty())
        return;

    _party->setIcon(filename);
    loadPartyImage();
}

void PartyFrame::resizeEvent(QResizeEvent *event)
{
    int newColumnCount = getColumnCount();
    if(newColumnCount == -1)
        return;

    qDebug() << "[PartyFrame] Frame resized: " << event->oldSize() << " to " << event->size() << ", columns: " << _layoutColumns << " to " << newColumnCount;
    qDebug() << "[PartyFrame]         character list: " << ui->characterList->size() << ", contents: " << ui->scrollAreaWidgetContents->size();

    if((_layoutColumns == -1) ||(_layoutColumns != newColumnCount))
        updateLayout();
}

void PartyFrame::updateAll()
{
    if(!_party)
        return;

    QList<Character*> updateCharacters;
    QList<Character*> allCharacters = _party->findChildren<Character*>();
    for(Character* character : allCharacters)
    {
        if((character) &&(character->getDndBeyondID() != -1))
            updateCharacters.append(character);
    }

    if(updateCharacters.isEmpty())
        return;

    CharacterImporter* importer = new CharacterImporter();
    connect(importer, &CharacterImporter::characterImported, this, &PartyFrame::handleCharacterChanged);
    importer->updateCharacters(updateCharacters);
}

void PartyFrame::clearGrid()
{
    if(!_characterGrid)
        return;

    qDebug() << "[PartyFrame] Clearing the grid";

    _characterGrid->invalidate();

    // Delete the grid entries
    QLayoutItem *child = nullptr;
    while((child = _characterGrid->takeAt(0)) != nullptr)
    {
        delete child;
    }

    _layoutColumns = -1;
    delete _characterGrid;
    _characterGrid = nullptr;
}

void PartyFrame::clearList()
{
    // Make sure the grid is cleaned up before deleting the frames
    clearGrid();

    qDeleteAll(_characterFrames);
    _characterFrames.clear();
}

void PartyFrame::readPartyData()
{
    loadPartyImage();
    loadCharacters();
}

void PartyFrame::loadPartyImage()
{
    if(_party)
        ui->lblIcon->setPixmap(_party->getIconPixmap(DMHelper::PixmapSize_Battle).scaled(ui->lblIcon->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void PartyFrame::loadCharacters()
{
    if(!_party)
        return;

    clearList();

    bool canUpdate = false;
    QList<Character*> allCharacters = _party->findChildren<Character*>();
    for(Character* character : allCharacters)
    {
        if(character)
        {
            PartyCharacterGridFrame* newCharacterFrame = new PartyCharacterGridFrame(*character);
            connect(newCharacterFrame, &PartyCharacterGridFrame::characterSelected, this, &PartyFrame::characterSelected);
            _characterFrames.append(newCharacterFrame);
            if(character->getDndBeyondID() != -1)
                canUpdate = true;
        }
    }

    ui->btnUpdateAll->setEnabled(canUpdate);

    updateLayout();
}

void PartyFrame::updateLayout()
{
    int columnCount = getColumnCount();
    qDebug() << "[PartyFrame] Updating frame layout. Previous columns " << _layoutColumns << ", new columns: " << columnCount;

    clearGrid();

    _characterGrid = new QGridLayout;
    _characterGrid->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    _characterGrid->setContentsMargins(PARTY_FRAME_SPACING, PARTY_FRAME_SPACING, PARTY_FRAME_SPACING, PARTY_FRAME_SPACING);
    _characterGrid->setSpacing(PARTY_FRAME_SPACING);
    ui->scrollAreaWidgetContents->setLayout(_characterGrid);

    for(PartyCharacterGridFrame* frame : _characterFrames)
    {
        int row = _characterGrid->count() / columnCount;
        int column = _characterGrid->count() % columnCount;

        _characterGrid->addWidget(frame, row, column);
    }

    _layoutColumns = columnCount;
    int spacingColumn = _characterGrid->columnCount();

    _characterGrid->addItem(new QSpacerItem(20, 40, QSizePolicy::Expanding), 0, spacingColumn);

    for(int i = 0; i < spacingColumn; ++i)
        _characterGrid->setColumnStretch(i, 1);

    _characterGrid->setColumnStretch(spacingColumn, 10);
}

int PartyFrame::getColumnCount()
{
    if(_characterFrames.count() == 0)
        return -1;

    qDebug() << "[PartyFrame] character list width: " << ui->characterList->width() << ", viewport: " << ui->characterList->viewport()->width();
    qDebug() << "[PartyFrame] single grid count: " << _characterFrames.at(0)->width() << ", hint: " << _characterFrames.at(0)->sizeHint();
    qDebug() << "[PartyFrame] calculated columns: " << (ui->characterList->width() - PARTY_FRAME_SPACING) / (_characterFrames.at(0)->width() + PARTY_FRAME_SPACING);

    // (Width of the scroll area minus the left margin) / (width of a frame plus spacing between frames)
    //return (ui->characterList->width() - PARTY_FRAME_SPACING) / (_characterFrames.at(0)->width() + PARTY_FRAME_SPACING);
    return (ui->characterList->width() - PARTY_FRAME_SPACING) / (320 + PARTY_FRAME_SPACING);
}

void PartyFrame::calculateThresholds()
{
    if(!_party)
    {
        ui->lblEasy->setText(QString("---"));
        ui->lblMedium->setText(QString("---"));
        ui->lblHard->setText(QString("---"));
        ui->lblDeadly->setText(QString("---"));
        return;
    }

    int thresholds[4] = {0,0,0,0};

    QList<Character*> activeCharacters = _party->getActiveCharacters();
    for(int i = 0; i < activeCharacters.count(); ++i)
    {
        Character* character = activeCharacters.at(i);
        if(character)
        {
            for(int j = 0; j < 4; ++j)
            {
                thresholds[j] += character->getXPThreshold(j);
            }
        }
    }

    ui->lblEasy->setText(QString::number(thresholds[DMHelper::XPThreshold_Easy]));
    ui->lblMedium->setText(QString::number(thresholds[DMHelper::XPThreshold_Medium]));
    ui->lblHard->setText(QString::number(thresholds[DMHelper::XPThreshold_Hard]));
    ui->lblDeadly->setText(QString::number(thresholds[DMHelper::XPThreshold_Deadly]));
}

void PartyFrame::handlePublishClicked()
{
    if(!_party)
        return;

    QImage iconImg;
    QString iconFile = _party->getIcon();
    if(!iconImg.load(iconFile))
        iconImg = _party->getIconPixmap(DMHelper::PixmapSize_Full).toImage();

    if(iconImg.isNull())
        return;

    if(_rotation != 0)
        iconImg = iconImg.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);

    emit publishPartyImage(iconImg);
}
