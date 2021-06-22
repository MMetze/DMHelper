#include "ribbontabbattle.h"
#include "ui_ribbontabbattle.h"

RibbonTabBattle::RibbonTabBattle(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabBattle)
{
    ui->setupUi(this);

    connect(ui->btnNewMap, SIGNAL(clicked(bool)), this, SIGNAL(newMapClicked()));
    connect(ui->btnReloadMap, SIGNAL(clicked(bool)), this, SIGNAL(reloadMapClicked()));
    connect(ui->btnAddCharacter, SIGNAL(clicked(bool)), this, SIGNAL(addCharacterClicked()));
    connect(ui->btnAddMonsters, SIGNAL(clicked(bool)), this, SIGNAL(addMonsterClicked()));
    connect(ui->btnAddNPC, SIGNAL(clicked(bool)), this, SIGNAL(addNPCClicked()));
    connect(ui->btnAddObject, SIGNAL(clicked(bool)), this, SIGNAL(addObjectClicked()));
    connect(ui->btnCastSpell, SIGNAL(clicked(bool)), this, SIGNAL(castSpellClicked()));
    connect(ui->btnAddRadius, SIGNAL(clicked(bool)), this, SIGNAL(addEffectRadiusClicked()));
    connect(ui->btnAddCube, SIGNAL(clicked(bool)), this, SIGNAL(addEffectCubeClicked()));
    connect(ui->btnAddCone, SIGNAL(clicked(bool)), this, SIGNAL(addEffectConeClicked()));
    connect(ui->btnAddLine, SIGNAL(clicked(bool)), this, SIGNAL(addEffectLineClicked()));
    connect(ui->btnShowLiving, SIGNAL(clicked(bool)), this, SIGNAL(showLivingClicked(bool)));
    connect(ui->btnShowDead, SIGNAL(clicked(bool)), this, SIGNAL(showDeadClicked(bool)));
    connect(ui->btnShowEffects, SIGNAL(clicked(bool)), this, SIGNAL(showEffectsClicked(bool)));
    connect(ui->btnShowMovement, SIGNAL(clicked(bool)), this, SIGNAL(showMovementClicked(bool)));
    connect(ui->btnFowMovement, SIGNAL(clicked(bool)), this, SIGNAL(fowMovementClicked(bool)));
    connect(ui->btnLairActions, SIGNAL(clicked(bool)), this, SIGNAL(lairActionsClicked(bool)));
    connect(ui->btnStatistics, SIGNAL(clicked(bool)), this, SIGNAL(statisticsClicked()));
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

void RibbonTabBattle::setFowMovement(bool checked)
{
    ui->btnFowMovement->setChecked(checked);
}

void RibbonTabBattle::setLairActions(bool checked)
{
    ui->btnLairActions->setChecked(checked);
}

void RibbonTabBattle::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();

    setStandardButtonSize(*ui->lblNewMap, *ui->btnNewMap, frameHeight);
    setStandardButtonSize(*ui->lblReloadMap, *ui->btnReloadMap, frameHeight);
    setLineHeight(*ui->line, frameHeight);
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
    setLineHeight(*ui->line_4, frameHeight);
    setStandardButtonSize(*ui->lblShowLiving, *ui->btnShowLiving, frameHeight);
    setStandardButtonSize(*ui->lblShowDead, *ui->btnShowDead, frameHeight);
    setStandardButtonSize(*ui->lblShowEffects, *ui->btnShowEffects, frameHeight);
    setStandardButtonSize(*ui->lblShowMovement, *ui->btnShowMovement, frameHeight);
    setStandardButtonSize(*ui->lblFowMovement, *ui->btnFowMovement, frameHeight);
    setStandardButtonSize(*ui->lblLairActions, *ui->btnLairActions, frameHeight);
    setLineHeight(*ui->line_3, frameHeight);
    setStandardButtonSize(*ui->lblStatistics, *ui->btnStatistics, frameHeight);
}
