#include "ribbontabcampaign.h"
#include "ui_ribbontabcampaign.h"

RibbonTabCampaign::RibbonTabCampaign(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabCampaign)
{
    ui->setupUi(this);

    connect(ui->btnNewParty, SIGNAL(clicked(bool)), this, SIGNAL(newPartyClicked()));
    connect(ui->btnNewCharacter, SIGNAL(clicked(bool)), this, SIGNAL(newCharacterClicked()));
    connect(ui->btnNewMap, SIGNAL(clicked(bool)), this, SIGNAL(newMapClicked()));
    connect(ui->btnNewText, SIGNAL(clicked(bool)), this, SIGNAL(newTextClicked()));
    connect(ui->btnNewBattle, SIGNAL(clicked(bool)), this, SIGNAL(newBattleClicked()));
    connect(ui->btnAddSound, SIGNAL(clicked(bool)), this, SIGNAL(newSoundClicked()));
    connect(ui->btnSyrinscape, SIGNAL(clicked(bool)), this, SIGNAL(newSyrinscapeClicked()));
    connect(ui->btnSyrinscapeOnline, SIGNAL(clicked(bool)), this, SIGNAL(newSyrinscapeOnlineClicked()));
    connect(ui->btnYouTube, SIGNAL(clicked(bool)), this, SIGNAL(newYoutubeClicked()));
    connect(ui->btnRemove, SIGNAL(clicked(bool)), this, SIGNAL(removeItemClicked()));
    connect(ui->btnNotes, SIGNAL(clicked(bool)), this, SIGNAL(showNotesClicked()));
    connect(ui->btnExportItem, SIGNAL(clicked(bool)), this, SIGNAL(exportItemClicked()));
    connect(ui->btnImportItem, SIGNAL(clicked(bool)), this, SIGNAL(importItemClicked()));
    connect(ui->btnImportCharacter, SIGNAL(clicked(bool)), this, SIGNAL(importCharacterClicked()));

    connect(ui->btnUndo, SIGNAL(clicked(bool)), this, SIGNAL(undoClicked()));
    connect(ui->btnRedo, SIGNAL(clicked(bool)), this, SIGNAL(redoClicked()));

    ui->btnUndo->setEnabled(false);
    ui->btnRedo->setEnabled(false);

    // Todo: add a proper undo/redo behavior
    ui->btnUndo->hide();
    ui->lblUndo->hide();
    ui->btnRedo->hide();
    ui->lblRedo->hide();
}

RibbonTabCampaign::~RibbonTabCampaign()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabCampaign::getPublishRibbon()
{
    return ui->framePublish;
}

void RibbonTabCampaign::setCampaignEnabled(bool enabled)
{
    ui->btnNewParty->setEnabled(enabled);
    ui->btnNewCharacter->setEnabled(enabled);
    ui->btnNewMap->setEnabled(enabled);
    ui->btnNewText->setEnabled(enabled);
    ui->btnNewBattle->setEnabled(enabled);
    ui->btnAddSound->setEnabled(enabled);
    ui->btnSyrinscape->setEnabled(enabled);
    ui->btnSyrinscapeOnline->setEnabled(enabled);
    ui->btnYouTube->setEnabled(enabled);
    ui->btnRemove->setEnabled(enabled);
    ui->btnNotes->setEnabled(enabled);
    ui->btnExportItem->setEnabled(enabled);
    ui->btnImportItem->setEnabled(enabled);
    ui->btnImportCharacter->setEnabled(enabled);
}

void RibbonTabCampaign::setUndoEnabled(bool enabled)
{
    ui->btnUndo->setEnabled(enabled);
    ui->btnRedo->setEnabled(enabled);
}

void RibbonTabCampaign::setAddPCButton(bool isPC)
{
    ui->lblNewCharacter->setText(isPC ? QString("PC") : QString("NPC"));
    ui->btnNewCharacter->setIcon(isPC ? QIcon(":/img/data/icon_newcharacter.png") : QIcon(":/img/data/icon_newnpc.png"));
}

void RibbonTabCampaign::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();

    setStandardButtonSize(*ui->lblNewText, *ui->btnNewText, frameHeight);
    setStandardButtonSize(*ui->lblNewParty, *ui->btnNewParty, frameHeight);
    setStandardButtonSize(*ui->lblNewCharacter, *ui->btnNewCharacter, frameHeight);
    setStandardButtonSize(*ui->lblNewMap, *ui->btnNewMap, frameHeight);
    setStandardButtonSize(*ui->lblNewBattle, *ui->btnNewBattle, frameHeight);
    setLineHeight(*ui->line_5, frameHeight);
    setStandardButtonSize(*ui->lblAddSound, *ui->btnAddSound, frameHeight);
    setStandardButtonSize(*ui->lblSyrinscape, *ui->btnSyrinscape, frameHeight);
    setStandardButtonSize(*ui->lblSyrinscapeOnline, *ui->btnSyrinscapeOnline, frameHeight);
    setStandardButtonSize(*ui->lblYouTube, *ui->btnYouTube, frameHeight);
    setLineHeight(*ui->line_6, frameHeight);
    setStandardButtonSize(*ui->lblRemove, *ui->btnRemove, frameHeight);
    setLineHeight(*ui->line, frameHeight);
    setStandardButtonSize(*ui->lblNotes, *ui->btnNotes, frameHeight);
    setLineHeight(*ui->line_3, frameHeight);
    setStandardButtonSize(*ui->lblExportItem, *ui->btnExportItem, frameHeight);
    setStandardButtonSize(*ui->lblImportItem, *ui->btnImportItem, frameHeight);
    setStandardButtonSize(*ui->lblImportCharacter, *ui->btnImportCharacter, frameHeight);
    setLineHeight(*ui->line_2, frameHeight);
    setStandardButtonSize(*ui->lblUndo, *ui->btnUndo, frameHeight);
    setStandardButtonSize(*ui->lblRedo, *ui->btnRedo, frameHeight);
}
