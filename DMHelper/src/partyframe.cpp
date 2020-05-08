#include "partyframe.h"
#include "ui_partyframe.h"
#include "party.h"
#include "character.h"
#include "partyframecharacter.h"
#include <QFileDialog>
#include <QMouseEvent>
#include <QDebug>

PartyFrame::PartyFrame(QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::PartyFrame),
    _characterLayout(nullptr),
    _party(nullptr),
    _mouseDown(false),
    _rotation(0)
{
    ui->setupUi(this);

    _characterLayout = new QVBoxLayout;
    _characterLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    _characterLayout->setContentsMargins(3,3,3,3);
    ui->scrollAreaWidgetContents->setLayout(_characterLayout);

}

PartyFrame::~PartyFrame()
{
    qDebug() << "[PartyFrame] being destroyed: " << _characterLayout->count() << " layouts and " << _characterLayout->count() << " widgets";

    QLayoutItem *child;
    while ((child = _characterLayout->takeAt(0)) != nullptr) {
        delete child;
    }

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

void PartyFrame::loadCharacters()
{
    if(!_party)
        return;

    clearList();

    QList<Character*> allCharacters = _party->findChildren<Character*>();
    for(Character* character : allCharacters)
    {
        addCharacter(character);
    }
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

    QString filename = QFileDialog::getOpenFileName(this,QString("Select New Image..."));
    if(filename.isEmpty())
        return;

    _party->setIcon(filename);
    loadPartyImage();
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

void PartyFrame::clearList()
{
    // Delete the widget for the combatant
    QLayoutItem *child = nullptr;
    while ((child = _characterLayout->takeAt(0)) != nullptr)
    {
        child->widget()->deleteLater();
        delete child;
    }

}

void PartyFrame::addCharacter(Character* character)
{
    if(character)
        _characterLayout->addWidget(new PartyFrameCharacter(*character));
}

