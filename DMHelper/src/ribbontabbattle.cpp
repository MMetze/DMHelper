#include "ribbontabbattle.h"
#include "ui_ribbontabbattle.h"
#include <QMenu>

RibbonTabBattle::RibbonTabBattle(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabBattle)
{
    ui->setupUi(this);

    connect(ui->btnAddCharacter, SIGNAL(clicked(bool)), this, SIGNAL(addCharacterClicked()));
    connect(ui->btnAddMonsters, SIGNAL(clicked(bool)), this, SIGNAL(addMonsterClicked()));
    connect(ui->btnAddNPC, SIGNAL(clicked(bool)), this, SIGNAL(addNPCClicked()));
    connect(ui->btnAddObject, SIGNAL(clicked(bool)), this, SIGNAL(addObjectClicked()));
    connect(ui->btnCastSpell, SIGNAL(clicked(bool)), this, SIGNAL(castSpellClicked()));
    connect(ui->btnAddRadius, SIGNAL(clicked(bool)), this, SIGNAL(addEffectRadiusClicked()));
    connect(ui->btnAddCube, SIGNAL(clicked(bool)), this, SIGNAL(addEffectCubeClicked()));
    connect(ui->btnAddCone, SIGNAL(clicked(bool)), this, SIGNAL(addEffectConeClicked()));
    connect(ui->btnAddLine, SIGNAL(clicked(bool)), this, SIGNAL(addEffectLineClicked()));
    connect(ui->btnDuplicate, SIGNAL(clicked(bool)), this, SIGNAL(duplicateClicked()));
    connect(ui->btnShowLiving, SIGNAL(clicked(bool)), this, SIGNAL(showLivingClicked(bool)));
    connect(ui->btnShowDead, SIGNAL(clicked(bool)), this, SIGNAL(showDeadClicked(bool)));
    connect(ui->btnShowEffects, SIGNAL(clicked(bool)), this, SIGNAL(showEffectsClicked(bool)));
    connect(ui->btnShowMovement, SIGNAL(clicked(bool)), this, SIGNAL(showMovementClicked(bool)));
    connect(ui->btnLairActions, SIGNAL(clicked(bool)), this, SIGNAL(lairActionsClicked(bool)));
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
    QAction* actionAddSmoke = new QAction(QIcon(":/img/data/icon_castspell.png"), QString("Add Smoke" ));
    effectMenu->addAction(actionAddSmoke);
    connect(actionAddSmoke, &QAction::triggered, this, &RibbonTabBattle::addEffectSmokeClicked);
    QAction* actionAddFire = new QAction(QIcon(":/img/data/icon_movement.png"), QString("Add Fire" ));
    effectMenu->addAction(actionAddFire);
    connect(actionAddFire, &QAction::triggered, this, &RibbonTabBattle::addEffectFireClicked);
    QAction* actionAddSparks = new QAction(QIcon(":/img/data/icon_living.png"), QString("Add Sparks" ));
    effectMenu->addAction(actionAddSparks);
    connect(actionAddSparks, &QAction::triggered, this, &RibbonTabBattle::addEffectSparksClicked);
    QAction* actionAddLight = new QAction(QIcon(":/img/data/icon_options.png"), QString("Add Light" ));
    effectMenu->addAction(actionAddLight);
    connect(actionAddLight, &QAction::triggered, this, &RibbonTabBattle::addEffectLightClicked);

    connect(effectMenu, &QMenu::triggered, this, &RibbonTabBattle::selectEffectAction);
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

void RibbonTabBattle::setLairActions(bool checked)
{
    ui->btnLairActions->setChecked(checked);
}

void RibbonTabBattle::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();

    setStandardButtonSize(*ui->lblAddCharacter, *ui->btnAddCharacter, frameHeight);
    setStandardButtonSize(*ui->lblAddMonsters, *ui->btnAddMonsters, frameHeight);
    setStandardButtonSize(*ui->lblAddNPC, *ui->btnAddNPC, frameHeight);
    setStandardButtonSize(*ui->lblAddObject, *ui->btnAddObject, frameHeight);
    setLineHeight(*ui->line_2, frameHeight);
    setStandardButtonSize(*ui->lblCastSpell, *ui->btnCastSpell, frameHeight);
    setLineHeight(*ui->line_5, frameHeight);
    setStandardButtonSize(*ui->lblAddRadius, *ui->btnAddRadius, frameHeight);
    setStandardButtonSize(*ui->lblAddCube, *ui->btnAddCube, frameHeight);
    setStandardButtonSize(*ui->lblAddCone, *ui->btnAddCone, frameHeight);
    setStandardButtonSize(*ui->lblAddLine, *ui->btnAddLine, frameHeight);
    setStandardButtonSize(*ui->lblDuplicate, *ui->btnDuplicate, frameHeight);
    setLineHeight(*ui->line_4, frameHeight);
    setStandardButtonSize(*ui->lblShowLiving, *ui->btnShowLiving, frameHeight);
    setStandardButtonSize(*ui->lblShowDead, *ui->btnShowDead, frameHeight);
    setStandardButtonSize(*ui->lblShowEffects, *ui->btnShowEffects, frameHeight);
    setStandardButtonSize(*ui->lblShowMovement, *ui->btnShowMovement, frameHeight);
    setStandardButtonSize(*ui->lblLairActions, *ui->btnLairActions, frameHeight);
    setLineHeight(*ui->line_3, frameHeight);
    setStandardButtonSize(*ui->lblStatistics, *ui->btnStatistics, frameHeight);
}

void RibbonTabBattle::selectEffectAction(QAction* action)
{
    if(!action)
        return;

    ui->btnEffects->setIcon(action->icon());
    ui->btnEffects->setToolTip(action->text());
}
