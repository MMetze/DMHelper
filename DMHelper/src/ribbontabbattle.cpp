#include "ribbontabbattle.h"
#include "ui_ribbontabbattle.h"

RibbonTabBattle::RibbonTabBattle(QWidget *parent) :
    QFrame(parent),
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
}

RibbonTabBattle::~RibbonTabBattle()
{
    delete ui;
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
