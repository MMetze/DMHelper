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
    connect(ui->btnShowLiving, SIGNAL(clicked(bool)), this, SIGNAL(showLivingClicked(bool)));
    connect(ui->btnShowDead, SIGNAL(clicked(bool)), this, SIGNAL(showDeadClicked(bool)));
    connect(ui->btnShowEffects, SIGNAL(clicked(bool)), this, SIGNAL(showEffectsClicked(bool)));
    connect(ui->btnShowMovement, SIGNAL(clicked(bool)), this, SIGNAL(showMovementClicked(bool)));
    connect(ui->btnLairActions, SIGNAL(clicked(bool)), this, SIGNAL(lairActionsClicked(bool)));
    connect(ui->btnNext, SIGNAL(clicked(bool)), this, SIGNAL(nextClicked()));
    connect(ui->btnSort, SIGNAL(clicked(bool)), this, SIGNAL(sortClicked()));
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

void RibbonTabBattle::setLairActions(bool checked)
{
    ui->btnLairActions->setChecked(checked);
}

void RibbonTabBattle::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    setStandardButtonSize(*ui->lblNewMap, *ui->btnNewMap);
    setStandardButtonSize(*ui->lblReloadMap, *ui->btnReloadMap);
    setLineHeight(*ui->line);
    setStandardButtonSize(*ui->lblAddCharacter, *ui->btnAddCharacter);
    setStandardButtonSize(*ui->lblAddMonsters, *ui->btnAddMonsters);
    setStandardButtonSize(*ui->lblAddNPC, *ui->btnAddNPC);
    setLineHeight(*ui->line_2);
    setStandardButtonSize(*ui->lblShowLiving, *ui->btnShowLiving);
    setStandardButtonSize(*ui->lblShowDead, *ui->btnShowDead);
    setStandardButtonSize(*ui->lblShowEffects, *ui->btnShowEffects);
    setStandardButtonSize(*ui->lblShowMovement, *ui->btnShowMovement);
    setStandardButtonSize(*ui->lblLairActions, *ui->btnLairActions);
    setLineHeight(*ui->line_3);
    setStandardButtonSize(*ui->lblNext, *ui->btnNext);
    setStandardButtonSize(*ui->lblSort, *ui->btnSort);
    setStandardButtonSize(*ui->lblStatistics, *ui->btnStatistics);
}