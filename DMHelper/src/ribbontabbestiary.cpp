#include "ribbontabbestiary.h"
#include "ui_ribbontabbestiary.h"

RibbonTabBestiary::RibbonTabBestiary(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabBestiary)
{
    ui->setupUi(this);

    connect(ui->btnBestiary, SIGNAL(clicked(bool)), this, SIGNAL(bestiaryClicked()));
    connect(ui->btnExportBestiary, SIGNAL(clicked(bool)), this, SIGNAL(exportBestiaryClicked()));
    connect(ui->btnImportBestiary, SIGNAL(clicked(bool)), this, SIGNAL(importBestiaryClicked()));

    connect(ui->btnSpellbook, SIGNAL(clicked(bool)), this, SIGNAL(spellbookClicked()));

    connect(ui->btnScreen, SIGNAL(clicked(bool)), this, SIGNAL(screenClicked()));
    connect(ui->btnTables, SIGNAL(clicked(bool)), this, SIGNAL(tablesClicked()));
    connect(ui->btnQuickRef, SIGNAL(clicked(bool)), this, SIGNAL(referenceClicked()));
    connect(ui->btnSoundboard, SIGNAL(clicked(bool)), this, SIGNAL(soundboardClicked()));

    connect(ui->btnRollDice, SIGNAL(clicked(bool)), this, SIGNAL(rollDiceClicked()));
    connect(ui->btnPublishText, SIGNAL(clicked(bool)), this, SIGNAL(publishTextClicked()));
    connect(ui->btnTranslateText, SIGNAL(clicked(bool)), this, SIGNAL(translateTextClicked()));
    connect(ui->btnRandomMarket, SIGNAL(clicked(bool)), this, SIGNAL(randomMarketClicked()));
    connect(ui->btnTimeDate, SIGNAL(clicked(bool)), this, SIGNAL(calendarClicked()));
    connect(ui->btnCountdown, SIGNAL(clicked(bool)), this, SIGNAL(countdownClicked()));
}

RibbonTabBestiary::~RibbonTabBestiary()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabBestiary::getPublishRibbon()
{
    return ui->framePublish;
}

void RibbonTabBestiary::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();

    setStandardButtonSize(*ui->lblBestiary, *ui->btnBestiary, frameHeight);
    setStandardButtonSize(*ui->lblExportBestiary, *ui->btnExportBestiary, frameHeight);
    setStandardButtonSize(*ui->lblImportBestiary, *ui->btnImportBestiary, frameHeight);
    setLineHeight(*ui->line, frameHeight);
    setStandardButtonSize(*ui->lblSpellbook, *ui->btnSpellbook, frameHeight);
    setLineHeight(*ui->line_3, frameHeight);
    setStandardButtonSize(*ui->lblSoundboard, *ui->btnSoundboard, frameHeight);
    setLineHeight(*ui->line_4, frameHeight);
    setStandardButtonSize(*ui->lblScreen, *ui->btnScreen, frameHeight);
    setStandardButtonSize(*ui->lblTables, *ui->btnTables, frameHeight);
    setStandardButtonSize(*ui->lblQuickRef, *ui->btnQuickRef, frameHeight);
    setLineHeight(*ui->line_2, frameHeight);
    setStandardButtonSize(*ui->lblRollDice, *ui->btnRollDice, frameHeight);
    setStandardButtonSize(*ui->lblPublishText, *ui->btnPublishText, frameHeight);
    setStandardButtonSize(*ui->lblTranslateText, *ui->btnTranslateText, frameHeight);
    setStandardButtonSize(*ui->lblRandomMarket, *ui->btnRandomMarket, frameHeight);
    setStandardButtonSize(*ui->lblTimeDate, *ui->btnTimeDate, frameHeight);
    setStandardButtonSize(*ui->lblCountdown, *ui->btnCountdown, frameHeight);
}


/*
 *
 *
 *         <widget class="QMenu" name="menu_Bestiary">
         <property name="title">
          <string>&amp;Tools</string>
         </property>
         <addaction name="action_Open_Bestiary"/>
         <addaction name="actionExport_Bestiary"/>
         <addaction name="actionImport_Bestiary"/>
         <addaction name="separator"/>
         <addaction name="actionDice"/>
         <addaction name="actionPublish_Text"/>
         <addaction name="actionTranslate_Text"/>
         <addaction name="actionRandom_Market"/>
         <addaction name="separator"/>
         <addaction name="actionOptions"/>
        </widget>


        */
