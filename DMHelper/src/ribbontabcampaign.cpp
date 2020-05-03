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
    //connect(ui->btnNewNPC, SIGNAL(clicked(bool)), this, SIGNAL(newNPCClicked()));
    connect(ui->btnNewText, SIGNAL(clicked(bool)), this, SIGNAL(newTextClicked()));
    connect(ui->btnNewBattle, SIGNAL(clicked(bool)), this, SIGNAL(newBattleClicked()));
    connect(ui->btnNewScrollingText, SIGNAL(clicked(bool)), this, SIGNAL(newScrollingTextClicked()));
    connect(ui->btnExportItem, SIGNAL(clicked(bool)), this, SIGNAL(exportItemClicked()));
    connect(ui->btnImportItem, SIGNAL(clicked(bool)), this, SIGNAL(importItemClicked()));
    connect(ui->btnImportCharacter, SIGNAL(clicked(bool)), this, SIGNAL(importCharacterClicked()));
    //connect(ui->btnImportNPC, SIGNAL(clicked(bool)), this, SIGNAL(importNPCClicked()));

    connect(ui->btnUndo, SIGNAL(clicked(bool)), this, SIGNAL(undoClicked()));
    connect(ui->btnRedo, SIGNAL(clicked(bool)), this, SIGNAL(redoClicked()));
    //connect(ui->btnPlayersWindow, SIGNAL(clicked(bool)), this, SIGNAL(playersWindowClicked(bool)));
    //connect(ui->btnPreview, SIGNAL(clicked(bool)), this, SIGNAL(previewClicked()));

    ui->btnUndo->setEnabled(false);
    ui->btnRedo->setEnabled(false);
}

RibbonTabCampaign::~RibbonTabCampaign()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabCampaign::getPublishRibbon()
{
    return ui->framePublish;
}

/*
void RibbonTabCampaign::setPlayersWindow(bool checked)
{
    ui->btnPlayersWindow->setChecked(checked);
}
*/

void RibbonTabCampaign::setCampaignEnabled(bool enabled)
{
    ui->btnNewParty->setEnabled(enabled);
    ui->btnNewCharacter->setEnabled(enabled);
    ui->btnNewMap->setEnabled(enabled);
    //ui->btnNewNPC->setEnabled(enabled);
    ui->btnNewText->setEnabled(enabled);
    ui->btnNewBattle->setEnabled(enabled);
    ui->btnNewScrollingText->setEnabled(enabled);
    ui->btnExportItem->setEnabled(enabled);
    ui->btnImportItem->setEnabled(enabled);
    ui->btnImportCharacter->setEnabled(enabled);
    //ui->btnImportNPC->setEnabled(enabled);
}

void RibbonTabCampaign::setUndoEnabled(bool enabled)
{
    ui->btnUndo->setEnabled(enabled);
    ui->btnRedo->setEnabled(enabled);
}

void RibbonTabCampaign::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    setStandardButtonSize(*ui->lblNewText, *ui->btnNewText);
    setLineHeight(*ui->line_3);
    setStandardButtonSize(*ui->lblNewParty, *ui->btnNewParty);
    setStandardButtonSize(*ui->lblNewCharacter, *ui->btnNewCharacter);
    //setStandardButtonSize(*ui->lblNewNPC, *ui->btnNewNPC);
    setLineHeight(*ui->line_4);
    setStandardButtonSize(*ui->lblNewMap, *ui->btnNewMap);
    setStandardButtonSize(*ui->lblNewBattle, *ui->btnNewBattle);
    setStandardButtonSize(*ui->lblNewScrollingText, *ui->btnNewScrollingText);
    setLineHeight(*ui->line);
    setStandardButtonSize(*ui->lblExportItem, *ui->btnExportItem);
    setStandardButtonSize(*ui->lblImportItem, *ui->btnImportItem);
    setStandardButtonSize(*ui->lblImportCharacter, *ui->btnImportCharacter);
    //setStandardButtonSize(*ui->lblImportNPC, *ui->btnImportNPC);
    setLineHeight(*ui->line_2);
    setStandardButtonSize(*ui->lblUndo, *ui->btnUndo);
    setStandardButtonSize(*ui->lblRedo, *ui->btnRedo);
    //setLineHeight(*ui->line_4);
    //setStandardButtonSize(*ui->lblPlayersWindow, *ui->btnPlayersWindow);
    //setStandardButtonSize(*ui->lblPreview, *ui->btnPreview);
}

/*
 * <widget class="QMenu" name="menu_Campaign">
 <property name="title">
  <string>&amp;Campaign</string>
 </property>
 <widget class="QMenu" name="menuNew_Encounter">
  <property name="title">
   <string>New &amp;Encounter</string>
  </property>
  <addaction name="actionNew_Text_Encounter"/>
  <addaction name="actionNew_Battle_Encounter"/>
  <addaction name="actionNew_Scrolling_Text_Encounter"/>
 </widget>
 <addaction name="actionNew_Adventure"/>
 <addaction name="actionNew_Character"/>
 <addaction name="menuNew_Encounter"/>
 <addaction name="actionNew_Map"/>
 <addaction name="separator"/>
 <addaction name="actionExport_Item"/>
 <addaction name="actionImport_Item"/>
 <addaction name="separator"/>
 <addaction name="action_Import_Character"/>
 <addaction name="actionImport_NPC"/>
 <addaction name="actionPing_Discord"/>
 <addaction name="separator"/>
 <addaction name="actionOpen_Players_Window"/>
 <addaction name="actionBattle_Dialog"/>
 <addaction name="actionStart_Battle"/>
</widget>
*/
