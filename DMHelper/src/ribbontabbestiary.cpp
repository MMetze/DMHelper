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

    setStandardButtonSize(*ui->lblBestiary, *ui->btnBestiary);
    setStandardButtonSize(*ui->lblExportBestiary, *ui->btnExportBestiary);
    setStandardButtonSize(*ui->lblImportBestiary, *ui->btnImportBestiary);
    setLineHeight(*ui->line);
    setStandardButtonSize(*ui->lblScreen, *ui->btnScreen);
    setStandardButtonSize(*ui->lblTables, *ui->btnTables);
    setStandardButtonSize(*ui->lblQuickRef, *ui->btnQuickRef);
    setStandardButtonSize(*ui->lblSoundboard, *ui->btnSoundboard);
    setLineHeight(*ui->line_2);
    setStandardButtonSize(*ui->lblRollDice, *ui->btnRollDice);
    setStandardButtonSize(*ui->lblPublishText, *ui->btnPublishText);
    setStandardButtonSize(*ui->lblTranslateText, *ui->btnTranslateText);
    setStandardButtonSize(*ui->lblRandomMarket, *ui->btnRandomMarket);
    setStandardButtonSize(*ui->lblTimeDate, *ui->btnTimeDate);
    setStandardButtonSize(*ui->lblCountdown, *ui->btnCountdown);
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
