#include "ribbontabworldmap.h"
#include "ui_ribbontabworldmap.h"
#include "party.h"
#include <QMenu>
#include <QFileDialog>

RibbonTabWorldMap::RibbonTabWorldMap(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabWorldMap),
    _menu(new QMenu(this))
{
    ui->setupUi(this);

    connect(ui->btnShowParty, &QAbstractButton::toggled, this, &RibbonTabWorldMap::showPartyClicked);
    connect(ui->spinScale, SIGNAL(valueChanged(int)), this, SIGNAL(scaleChanged(int)));

    ui->btnShowParty->setMenu(_menu);

    RibbonTabWorldMap_PartyAction* defaultAction = new RibbonTabWorldMap_PartyAction(nullptr, RibbonTabWorldMap_PartyAction::PartyActionType_Default);
    defaultAction->setIcon(QIcon(":/img/data/icon_contentparty.png"));
    defaultAction->setText(QString("Default Icon"));
    _menu->addAction(defaultAction);
    selectAction(defaultAction);

    RibbonTabWorldMap_PartyAction* selectAction = new RibbonTabWorldMap_PartyAction(nullptr, RibbonTabWorldMap_PartyAction::PartyActionType_Select);
    selectAction->setText(QString("Choose icon..."));
    _menu->addAction(selectAction);

    _menu->addSeparator();
    connect(_menu, &QMenu::triggered, this, &RibbonTabWorldMap::selectAction);

    connect(ui->btnShowMarkers, &QAbstractButton::toggled, this, &RibbonTabWorldMap::showMarkersClicked);
    connect(ui->btnAddMarker, &QAbstractButton::clicked, this, &RibbonTabWorldMap::addMarkerClicked);
}

RibbonTabWorldMap::~RibbonTabWorldMap()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabWorldMap::getPublishRibbon()
{
    return ui->framePublish;
}

void RibbonTabWorldMap::setShowParty(bool showParty)
{
    if(ui->btnShowParty->isChecked() != showParty)
        ui->btnShowParty->setChecked(showParty);
}

void RibbonTabWorldMap::setParty(Party* party)
{
    if(!party)
        return;

    QList<QAction*> actionList = _menu->actions();
    for(QAction* action : actionList)
    {
        RibbonTabWorldMap_PartyAction* partyAction = dynamic_cast<RibbonTabWorldMap_PartyAction*>(action);
        if((partyAction) && (partyAction->getParty() == party))
        {
            selectAction(partyAction);
            return;
        }
    }
}

void RibbonTabWorldMap::setPartyIcon(const QString& partyIcon)
{
    setPartyButtonIcon(QIcon(partyIcon));
}

void RibbonTabWorldMap::setScale(int scale)
{
    if((scale != ui->spinScale->value()) &&
       (scale >= ui->spinScale->minimum()) &&
       (scale <= ui->spinScale->maximum()))
    {
        ui->spinScale->setValue(scale);
    }
}

void RibbonTabWorldMap::registerPartyIcon(Party* party)
{
    if((!party) || (!_menu))
        return;

    QList<QAction*> actionList = _menu->actions();
    for(QAction* action : actionList)
    {
        RibbonTabWorldMap_PartyAction* partyAction = dynamic_cast<RibbonTabWorldMap_PartyAction*>(action);
        if((partyAction) && (partyAction->getParty() == party))
        {
            partyAction->updateParty();
            return;
        }
    }

    QAction* newAction = new RibbonTabWorldMap_PartyAction(party);
    _menu->addAction(newAction);
    if(_menu->actions().count() <= 4)
        selectAction(newAction);
}

void RibbonTabWorldMap::removePartyIcon(Party* party)
{
    QList<QAction*> actionList = _menu->actions();
    for(QAction* action : actionList)
    {
        RibbonTabWorldMap_PartyAction* partyAction = dynamic_cast<RibbonTabWorldMap_PartyAction*>(action);
        if((partyAction) && (partyAction->getParty() == party))
        {
            _menu->removeAction(partyAction);
            return;
        }
    }
}

void RibbonTabWorldMap::clearPartyIcons()
{
    QList<QAction*> actionList = _menu->actions();
    while(actionList.count() > 3)
        delete actionList.takeAt(3);
}

void RibbonTabWorldMap::setShowMarkers(bool checked)
{
    if(ui->btnShowMarkers->isChecked() != checked)
        ui->btnShowMarkers->setChecked(checked);
}

void RibbonTabWorldMap::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();
    setStandardButtonSize(*ui->lblShowParty, *ui->btnShowParty, frameHeight);
    setWidgetSize(*ui->spinScale, ui->lblShowParty->width() * 9 / 10, ui->spinScale->height());
    setWidgetSize(*ui->lblScale, ui->lblShowParty->width(), ui->lblShowParty->height());
    setLineHeight(*ui->line_1, frameHeight);
    setStandardButtonSize(*ui->lblShowMarkers, *ui->btnShowMarkers, frameHeight);
    setStandardButtonSize(*ui->lblAddMarker, *ui->btnAddMarker, frameHeight);
}

void RibbonTabWorldMap::selectAction(QAction* action)
{
    if(!action)
        return;

    if(!action->icon().isNull())
        setPartyButtonIcon(action->icon());

    RibbonTabWorldMap_PartyAction* partyAction = dynamic_cast<RibbonTabWorldMap_PartyAction*>(action);
    if(!partyAction)
        return;

    switch(partyAction->getPartyType())
    {
        case RibbonTabWorldMap_PartyAction::PartyActionType_Party:
            emit partySelected(partyAction->getParty());
            break;
        case RibbonTabWorldMap_PartyAction::PartyActionType_Default:
            emit partyIconSelected(QString(":/img/data/icon_contentparty.png"));
            break;
        case RibbonTabWorldMap_PartyAction::PartyActionType_Select:
        {
            QString iconFile = QFileDialog::getOpenFileName(nullptr, QString("Select party token image file"));
            if(!iconFile.isEmpty())
                emit partyIconSelected(iconFile);
            break;
        }
        default:
            break;
    }
}

void RibbonTabWorldMap::setPartyButtonIcon(const QIcon &icon)
{
    ui->btnShowParty->setIcon(icon);
}





RibbonTabWorldMap_PartyAction::RibbonTabWorldMap_PartyAction(Party* party, int partyType, QObject *parent) :
    QAction(parent),
    _party(party),
    _partyType(partyType)
{
    updateParty();
    if(_party)
    {
        connect(_party, &Party::dirty, this, &RibbonTabWorldMap_PartyAction::updateParty);
        connect(_party, &Party::destroyed, this, &RibbonTabWorldMap_PartyAction::partyDestroyed);
    }
}

RibbonTabWorldMap_PartyAction::~RibbonTabWorldMap_PartyAction()
{
}

Party* RibbonTabWorldMap_PartyAction::getParty() const
{
    return _party;
}

int RibbonTabWorldMap_PartyAction::getPartyType() const
{
    return _partyType;
}

void RibbonTabWorldMap_PartyAction::updateParty()
{
    if(_party)
    {
        setIcon(QIcon(_party->getIconPixmap(DMHelper::PixmapSize_Battle)));
        setText(_party->getName());
    }
}

void RibbonTabWorldMap_PartyAction::partyDestroyed()
{
    _party = nullptr;
}
