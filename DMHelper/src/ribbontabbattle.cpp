#include "ribbontabbattle.h"
#include "ui_ribbontabbattle.h"
#include "party.h"
#include <QMenu>
#include <QFileDialog>

RibbonTabBattle::RibbonTabBattle(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabBattle),
    _partyMenu(new QMenu(this))
{
    ui->setupUi(this);

    connect(ui->btnShowParty, SIGNAL(clicked(bool)), this, SIGNAL(showPartyClicked(bool)));
    ui->btnShowParty->setMenu(_partyMenu);
    RibbonTabBattle_PartyAction* defaultAction = new RibbonTabBattle_PartyAction(nullptr, RibbonTabBattle_PartyAction::PartyActionType_Default);
    defaultAction->setIcon(QIcon(":/img/data/icon_contentparty.png"));
    defaultAction->setText(QString("Default Icon"));
    _partyMenu->addAction(defaultAction);
    selectPartyAction(defaultAction);

    RibbonTabBattle_PartyAction* chooseAction = new RibbonTabBattle_PartyAction(nullptr, RibbonTabBattle_PartyAction::PartyActionType_Select);
    chooseAction->setText(QString("Choose icon..."));
    _partyMenu->addAction(chooseAction);

    _partyMenu->addSeparator();
    connect(_partyMenu, &QMenu::triggered, this, &RibbonTabBattle::selectPartyAction);

    connect(ui->btnShowMarkers, SIGNAL(clicked(bool)), this, SIGNAL(showMarkersClicked(bool)));
    connect(ui->btnAddMarker, SIGNAL(clicked()), this, SIGNAL(addMarkerClicked()));
    connect(ui->btnAddCharacter, SIGNAL(clicked(bool)), this, SIGNAL(addCharacterClicked()));
    connect(ui->btnAddMonsters, SIGNAL(clicked(bool)), this, SIGNAL(addMonsterClicked()));
    connect(ui->btnAddNPC, SIGNAL(clicked(bool)), this, SIGNAL(addNPCClicked()));
    connect(ui->btnAddObject, SIGNAL(clicked(bool)), this, SIGNAL(addObjectClicked()));
    connect(ui->btnCastSpell, SIGNAL(clicked(bool)), this, SIGNAL(castSpellClicked()));
    connect(ui->btnDuplicate, SIGNAL(clicked(bool)), this, SIGNAL(duplicateClicked()));
    connect(ui->btnShowLiving, SIGNAL(clicked(bool)), this, SIGNAL(showLivingClicked(bool)));
    connect(ui->btnShowDead, SIGNAL(clicked(bool)), this, SIGNAL(showDeadClicked(bool)));
    connect(ui->btnShowEffects, SIGNAL(clicked(bool)), this, SIGNAL(showEffectsClicked(bool)));
    connect(ui->btnShowMovement, SIGNAL(clicked(bool)), this, SIGNAL(showMovementClicked(bool)));
    connect(ui->btnLairActions, SIGNAL(clicked(bool)), this, SIGNAL(lairActionsClicked()));
    connect(ui->btnAddEvent, SIGNAL(clicked(bool)), this, SIGNAL(addEventClicked()));
    connect(ui->btnStatistics, SIGNAL(clicked(bool)), this, SIGNAL(statisticsClicked()));

    QMenu* effectMenu = new QMenu(this);
    QAction* actionAddRadius = new QAction(QIcon(":/img/data/icon_neweffectradius.png"), QString("Add Radius" ));
    effectMenu->addAction(actionAddRadius);
    connect(actionAddRadius, &QAction::triggered, this, &RibbonTabBattle::addEffectRadiusClicked);
    QAction* actionAddCube = new QAction(QIcon(":/img/data/icon_neweffectcube.png"), QString("Add Cube" ));
    effectMenu->addAction(actionAddCube);
    connect(actionAddCube, &QAction::triggered, this, &RibbonTabBattle::addEffectCubeClicked);
    QAction* actionAddCone = new QAction(QIcon(":/img/data/icon_neweffectcone.png"), QString("Add Cone" ));
    effectMenu->addAction(actionAddCone);
    connect(actionAddCone, &QAction::triggered, this, &RibbonTabBattle::addEffectConeClicked);
    QAction* actionAddLine = new QAction(QIcon(":/img/data/icon_neweffectline.png"), QString("Add Line" ));
    effectMenu->addAction(actionAddLine);
    connect(actionAddLine, &QAction::triggered, this, &RibbonTabBattle::addEffectLineClicked);
    effectMenu->addSeparator();
    QAction* actionAddSmoke = new QAction(QIcon(":/img/data/icon_neweffectsmoke.png"), QString("Add Smoke" ));
    effectMenu->addAction(actionAddSmoke);
    connect(actionAddSmoke, &QAction::triggered, this, &RibbonTabBattle::addEffectSmokeClicked);
    QAction* actionAddFire = new QAction(QIcon(":/img/data/icon_neweffectfire.png"), QString("Add Fire" ));
    effectMenu->addAction(actionAddFire);
    connect(actionAddFire, &QAction::triggered, this, &RibbonTabBattle::addEffectFireClicked);
    QAction* actionAddSparks = new QAction(QIcon(":/img/data/icon_neweffectsparks.png"), QString("Add Sparks" ));
    effectMenu->addAction(actionAddSparks);
    connect(actionAddSparks, &QAction::triggered, this, &RibbonTabBattle::addEffectSparksClicked);
    QAction* actionAddLight = new QAction(QIcon(":/img/data/icon_neweffectlight.png"), QString("Add Light" ));
    effectMenu->addAction(actionAddLight);
    connect(actionAddLight, &QAction::triggered, this, &RibbonTabBattle::addEffectLightClicked);

    connect(effectMenu, &QMenu::triggered, this, &RibbonTabBattle::selectEffectAction);
    ui->btnEffects->setPopupMode(QToolButton::MenuButtonPopup);
    ui->btnEffects->setMenu(effectMenu);
    
    selectEffectAction(actionAddRadius);
}

RibbonTabBattle::~RibbonTabBattle()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabBattle::getPublishRibbon()
{
    return ui->framePublish;
}

void RibbonTabBattle::setParty(Party* party)
{
    if(!party)
        return;

    QList<QAction*> actionList = _partyMenu->actions();
    for(QAction* action : actionList)
    {
        RibbonTabBattle_PartyAction* partyAction = dynamic_cast<RibbonTabBattle_PartyAction*>(action);
        if((partyAction) && (partyAction->getParty() == party))
        {
            if(!partyAction->icon().isNull())
                setPartyButtonIcon(partyAction->icon());
            return;
        }
    }
}

void RibbonTabBattle::setPartyIcon(const QString& partyIcon)
{
    if(!partyIcon.isEmpty())
        setPartyButtonIcon(QIcon(partyIcon));
}

void RibbonTabBattle::registerPartyIcon(Party* party)
{
    if((!party) || (!_partyMenu))
        return;

    QList<QAction*> actionList = _partyMenu->actions();
    for(QAction* action : actionList)
    {
        RibbonTabBattle_PartyAction* partyAction = dynamic_cast<RibbonTabBattle_PartyAction*>(action);
        if((partyAction) && (partyAction->getParty() == party))
        {
            partyAction->updateParty();
            return;
        }
    }

    QAction* newAction = new RibbonTabBattle_PartyAction(party);
    _partyMenu->addAction(newAction);
    if((_partyMenu->actions().count() <= 4) && (!newAction->icon().isNull()))
        setPartyButtonIcon(newAction->icon());
}

void RibbonTabBattle::removePartyIcon(Party* party)
{
    QList<QAction*> actionList = _partyMenu->actions();
    for(QAction* action : actionList)
    {
        RibbonTabBattle_PartyAction* partyAction = dynamic_cast<RibbonTabBattle_PartyAction*>(action);
        if((partyAction) && (partyAction->getParty() == party))
        {
            _partyMenu->removeAction(partyAction);
            return;
        }
    }
}

void RibbonTabBattle::clearPartyIcons()
{
    QList<QAction*> actionList = _partyMenu->actions();
    while(actionList.count() > 3)
        delete actionList.takeAt(3);
}

void RibbonTabBattle::setShowParty(bool showParty)
{
    if(ui->btnShowParty->isChecked() != showParty)
        ui->btnShowParty->setChecked(showParty);
}

void RibbonTabBattle::setShowMarkers(bool showMarkers)
{
    if(ui->btnShowMarkers->isChecked() != showMarkers)
        ui->btnShowMarkers->setChecked(showMarkers);
}

void RibbonTabBattle::setShowLiving(bool checked)
{
    ui->btnShowLiving->setChecked(checked);
}

void RibbonTabBattle::setShowDead(bool checked)
{
    ui->btnShowDead->setChecked(checked);
}

void RibbonTabBattle::setShowEffects(bool checked)
{
    ui->btnShowEffects->setChecked(checked);
}

void RibbonTabBattle::setShowMovement(bool checked)
{
    ui->btnShowMovement->setChecked(checked);
}

void RibbonTabBattle::setLairActionsVisible(bool visible)
{
    ui->btnLairActions->setVisible(visible);
    ui->lblLairActions->setVisible(visible);
}

void RibbonTabBattle::selectPartyAction(QAction* action)
{
    if(!action)
        return;

    if(!action->icon().isNull())
        setPartyButtonIcon(action->icon());

    RibbonTabBattle_PartyAction* partyAction = dynamic_cast<RibbonTabBattle_PartyAction*>(action);
    if(!partyAction)
        return;

    switch(partyAction->getPartyType())
    {
        case RibbonTabBattle_PartyAction::PartyActionType_Party:
            emit partySelected(partyAction->getParty());
            break;
        case RibbonTabBattle_PartyAction::PartyActionType_Default:
            emit partyIconSelected(QString(":/img/data/icon_contentparty.png"));
            break;
        case RibbonTabBattle_PartyAction::PartyActionType_Select:
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

void RibbonTabBattle::setPartyButtonIcon(const QIcon &icon)
{
    ui->btnShowParty->setIcon(icon);
}

void RibbonTabBattle::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();
    int buttonSize = frameHeight - getLabelHeight(*ui->lblEffects, frameHeight);

    setStandardButtonSize(*ui->lblAddCharacter, *ui->btnAddCharacter, frameHeight);
    setStandardButtonSize(*ui->lblAddMonsters, *ui->btnAddMonsters, frameHeight);
    setStandardButtonSize(*ui->lblAddNPC, *ui->btnAddNPC, frameHeight);
    setStandardButtonSize(*ui->lblAddObject, *ui->btnAddObject, frameHeight);
    setLineHeight(*ui->line_2, frameHeight);
    setStandardButtonSize(*ui->lblCastSpell, *ui->btnCastSpell, frameHeight);
    setLineHeight(*ui->line_5, frameHeight);
    setWidgetSize(*ui->btnEffects, buttonSize * 10 / 8, buttonSize);
    setStandardButtonSize(*ui->lblDuplicate, *ui->btnDuplicate, frameHeight);
    setLineHeight(*ui->line_4, frameHeight);
    setStandardButtonSize(*ui->lblShowLiving, *ui->btnShowLiving, frameHeight);
    setStandardButtonSize(*ui->lblShowDead, *ui->btnShowDead, frameHeight);
    setStandardButtonSize(*ui->lblShowEffects, *ui->btnShowEffects, frameHeight);
    setStandardButtonSize(*ui->lblShowMovement, *ui->btnShowMovement, frameHeight);
    setStandardButtonSize(*ui->lblLairActions, *ui->btnLairActions, frameHeight);
    setStandardButtonSize(*ui->lblAddEvent, *ui->btnAddEvent, frameHeight);
    setLineHeight(*ui->line_3, frameHeight);
    setStandardButtonSize(*ui->lblStatistics, *ui->btnStatistics, frameHeight);
}

void RibbonTabBattle::selectEffectAction(QAction* action)
{
    if(!action)
        return;

    ui->btnEffects->setDefaultAction(action);
    ui->btnEffects->setIcon(action->icon());
    ui->btnEffects->setToolTip(action->text());
}

RibbonTabBattle_PartyAction::RibbonTabBattle_PartyAction(Party* party, int partyType, QObject *parent) :
    QAction(parent),
    _party(party),
    _partyType(partyType)
{
    updateParty();
    if(_party)
    {
        connect(_party, &Party::dirty, this, &RibbonTabBattle_PartyAction::updateParty);
        connect(_party, &Party::CampaignObjectBase::campaignObjectDestroyed, this, &RibbonTabBattle_PartyAction::partyDestroyed);
    }
}

RibbonTabBattle_PartyAction::~RibbonTabBattle_PartyAction()
{
}

Party* RibbonTabBattle_PartyAction::getParty() const
{
    return _party;
}

int RibbonTabBattle_PartyAction::getPartyType() const
{
    return _partyType;
}

void RibbonTabBattle_PartyAction::updateParty()
{
    if(_party)
    {
        setIcon(QIcon(_party->getIconPixmap(DMHelper::PixmapSize_Battle)));
        setText(_party->getName());
    }
}

void RibbonTabBattle_PartyAction::partyDestroyed()
{
    _party = nullptr;
}
