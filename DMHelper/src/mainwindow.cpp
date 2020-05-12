#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "publishwindow.h"
#include "publishframe.h"
#include "dicerolldialog.h"
#include "dicerollframe.h"
#include "countdownframe.h"
#include "party.h"
#include "character.h"
#include "characterimporter.h"
#include "objectimporter.h"
#include "partyframe.h"
#include "characterframe.h"
#include "campaign.h"
#include "combatantfactory.h"
#include "map.h"
#include "mapfactory.h"
#include "mapframe.h"
#include "battleframemapdrawer.h"
#include "mruhandler.h"
#include "encounterfactory.h"
#include "emptycampaignframe.h"
#include "encountertextedit.h"
//#include "encounterbattleedit.h"
#include "encounterbattle.h"
#include "encounterscrollingtext.h"
#include "encounterscrollingtextedit.h"
#include "campaignobjectframe.h"
#include "combatant.h"
#include "campaigntreemodel.h"
#include "campaigntreeitem.h"
//#include "battledialogmanager.h"
#include "battleframe.h"
#include "audioplaybackframe.h"
#include "monster.h"
#include "monsterclass.h"
#include "bestiary.h"
#include "bestiaryexportdialog.h"
#include "equipmentserver.h"
#include "textpublishdialog.h"
#include "texttranslatedialog.h"
#include "randommarketdialog.h"
#include "combatantselectdialog.h"
#ifdef INCLUDE_CHASE_SUPPORT
    #include "chaserselectiondialog.h"
#endif
#include "optionsdialog.h"
#include "selectzoom.h"
#include "scrolltabwidget.h"
#include "quickrefframe.h"
#include "dmscreentabwidget.h"
#include "timeanddateframe.h"
#include "audiotrackedit.h"
#include "audioplayer.h"
#include "basicdateserver.h"
#ifdef INCLUDE_NETWORK_SUPPORT
    #include "networkcontroller.h"
#endif
#include "aboutdialog.h"
#include "campaignexporter.h"
#include "basicdateserver.h"
#include "welcomeframe.h"
#include "customtableframe.h"
#include "discordposter.h"
#include "legaldialog.h"
#include "updatechecker.h"
#include "ribbonmain.h"
#include "battledialogmodel.h"
#include "ribbontabfile.h"
#include "ribbontabcampaign.h"
#include "ribbontabbestiary.h"
//#include "ribbontabhelp.h"
#include "ribbontabmap.h"
#include "ribbontabbattle.h"
#include "ribbontabscrolling.h"
#include "ribbontabtext.h"
#include "mapeditframe.h"
#include "publishbuttonribbon.h"
#include <QResizeEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QDomElement>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>
#include <QImageReader>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QInputDialog>
#include <QScrollBar>
#include <QMenu>
#include <QMessageBox>
#include <QTime>
#include <QScrollArea>
#include <QDesktopServices>
#include <QDebug>
#include <QLibraryInfo>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QScreen>
#ifndef Q_OS_MAC
#include <QSplashScreen>
#endif


/*
 * TODO:
 * Add copyright notice to all files
 * Remove commented code
 * Refactor mainwindow, make it smaller
 * Avoid redrawing the tree every time a change is made
 * Add maps to Settings
 * Add NPCs - first just to the tree, then to battles, chases or party
 * Add non-characters to the party (monsters or NPCs) --> create a chracter from a monster class
 * Spell reference
 * More editing ability for UI elements (campaign, adventure, encounter, maps - rename, remap, etc)
 * Remove chase and other duplicate classes (dialogs instead of frames, old monster/character widgets)
 * Cross-populate preview screens
 * Image publisher
 * Items in the campaign
 * Drag and Drop to add maps and images
 * Check member variable naming convention
 * Maps - Tokens and DM layers, clickable?
 * different brushes for FOW clearing
 * Full character sheet
 */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    pubWindow(nullptr),
    previewTab(nullptr),
    previewFrame(nullptr),
    previewDlg(nullptr),
    dmScreenDlg(nullptr),
    tableDlg(nullptr),
    quickRefDlg(nullptr),
    soundDlg(nullptr),
    timeAndDateFrame(nullptr),
    calendarDlg(nullptr),
    countdownDlg(nullptr),
    encounterTextEdit(nullptr),
    _scrollingTextEdit(nullptr),
    treeModel(nullptr),
    treeIndexMap(),
    characterLayout(nullptr),
    campaign(nullptr),
    campaignFileName(),
    //currentCharacter(),
    _options(nullptr),
    bestiaryDlg(),
#ifdef INCLUDE_CHASE_SUPPORT
    chaseDlg(nullptr),
#endif
    _battleDlgMgr(nullptr),
    _audioPlayer(nullptr),
#ifdef INCLUDE_NETWORK_SUPPORT
    _networkController(nullptr),
#endif
    mouseDown(false),
    mouseDownPos(),
    undoAction(nullptr),
    redoAction(nullptr),
    initialized(false),
    dirty(false),
    _animationFrameCount(DMHelper::ANIMATION_TIMER_PREVIEW_FRAMES),
    _ribbon(nullptr),
    _ribbonTabFile(nullptr),
    _ribbonTabCampaign(nullptr),
    _ribbonTabTools(nullptr),
//    _ribbonTabHelp(nullptr),
    _ribbonTabMap(nullptr),
    _ribbonTabBattle(nullptr),
    _ribbonTabScrolling(nullptr),
    _ribbonTabText(nullptr),
    _ribbonTabMiniMap(nullptr)
{
    QScreen* screen = QGuiApplication::primaryScreen();
#ifndef Q_OS_MAC
    QPixmap pixmap(":/img/data/dmhelper_opaque.png");
    QSize screenSize = screen != nullptr ? screen->availableSize() : QSize(1000, 1000);
    QSplashScreen splash(pixmap.scaled(screenSize.width() / 2, screenSize.height() / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    splash.show();
    splash.showMessage(QString("Initializing DM Helper\n"),Qt::AlignBottom | Qt::AlignHCenter);
#endif

    qDebug() << "[MainWindow] Initializing Main";

    qDebug() << "[MainWindow] DMHelper version information";
    qDebug() << "[MainWindow]     DMHelper Version: " << QString::number(DMHelper::DMHELPER_MAJOR_VERSION) + "." + QString::number(DMHelper::DMHELPER_MINOR_VERSION) + "." + QString::number(DMHelper::DMHELPER_ENGINEERING_VERSION);
    qDebug() << "[MainWindow]     Expected Bestiary Version: " << QString::number(DMHelper::BESTIARY_MAJOR_VERSION) + "." + QString::number(DMHelper::BESTIARY_MINOR_VERSION);
    qDebug() << "[MainWindow]     Expected Campaign File Version: " << QString::number(DMHelper::CAMPAIGN_MAJOR_VERSION) + "." + QString::number(DMHelper::CAMPAIGN_MINOR_VERSION);
    qDebug() << "[MainWindow]     Build: " << __DATE__ << " " << __TIME__;
#ifdef Q_OS_MAC
    qDebug() << "[MainWindow]     OS: MacOS";
#else
    qDebug() << "[MainWindow]     OS: Windows";
#endif
    qDebug() << "[MainWindow]     Working Directory: " << QDir::currentPath();
    qDebug() << "[MainWindow]     Executable Directory: " << QCoreApplication::applicationDirPath();

    qDebug() << "[MainWindow] Qt Information";
    qDebug() << "[MainWindow]     Qt Version: " << QLibraryInfo::version().toString();
    qDebug() << "[MainWindow]     Is Debug? " << QLibraryInfo::isDebugBuild();
    qDebug() << "[MainWindow]     PrefixPath: " << QLibraryInfo::location(QLibraryInfo::PrefixPath);
    qDebug() << "[MainWindow]     DocumentationPath: " << QLibraryInfo::location(QLibraryInfo::DocumentationPath);
    qDebug() << "[MainWindow]     HeadersPath: " << QLibraryInfo::location(QLibraryInfo::HeadersPath);
    qDebug() << "[MainWindow]     LibrariesPath: " << QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    qDebug() << "[MainWindow]     LibraryExecutablesPath: " << QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath);
    qDebug() << "[MainWindow]     BinariesPath: " << QLibraryInfo::location(QLibraryInfo::BinariesPath);
    qDebug() << "[MainWindow]     PluginsPath: " << QLibraryInfo::location(QLibraryInfo::PluginsPath);
    qDebug() << "[MainWindow]     ImportsPath: " << QLibraryInfo::location(QLibraryInfo::ImportsPath);
    qDebug() << "[MainWindow]     Qml2ImportsPath: " << QLibraryInfo::location(QLibraryInfo::Qml2ImportsPath);
    qDebug() << "[MainWindow]     ArchDataPath: " << QLibraryInfo::location(QLibraryInfo::ArchDataPath);
    qDebug() << "[MainWindow]     DataPath: " << QLibraryInfo::location(QLibraryInfo::DataPath);
    qDebug() << "[MainWindow]     TranslationsPath: " << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    qDebug() << "[MainWindow]     ExamplesPath: " << QLibraryInfo::location(QLibraryInfo::ExamplesPath);
    qDebug() << "[MainWindow]     TestsPath: " << QLibraryInfo::location(QLibraryInfo::TestsPath);
    qDebug() << "[MainWindow]     SettingsPath: " << QLibraryInfo::location(QLibraryInfo::SettingsPath);

    qDebug() << "[MainWindow] Standard Path Information";
    qDebug() << "[MainWindow]     DocumentsLocation: " << (QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());
    qDebug() << "[MainWindow]     ApplicationsLocation: " << (QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).first());
    qDebug() << "[MainWindow]     RuntimeLocation: " << (QStandardPaths::standardLocations(QStandardPaths::RuntimeLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::RuntimeLocation).first());
    qDebug() << "[MainWindow]     ConfigLocation: " << (QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first());
    qDebug() << "[MainWindow]     AppDataLocation: " << (QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first());
    qDebug() << "[MainWindow]     AppLocalDataLocation: " << (QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).first());

    // TODO: cleanup this constructor and mainwindow in general
    ui->setupUi(this);
    if(screen)
    {
        resize(screen->availableSize().width() * 4 / 5, screen->availableSize().height() * 4 / 5);
    }
    setupRibbonBar();

    // TODO: find out why the following has no effect! There must be something in the frames that has "minimumexpanding" size policy
    ui->splitter->setStretchFactor(0,1);
    ui->splitter->setStretchFactor(1,4);

    qDebug() << "[MainWindow] Reading Settings";
    _options = new OptionsContainer(this);
    MRUHandler* mruHandler = new MRUHandler(_ribbonTabFile->getMRUMenu(), DEFAULT_MRU_FILE_COUNT, this);
    connect(mruHandler,SIGNAL(triggerMRU(QString)),this,SLOT(openFile(QString)));
    _options->setMRUHandler(mruHandler);
    _options->readSettings();
    connect(_options,SIGNAL(bestiaryFileNameChanged()),this,SLOT(readBestiary()));
    //connect(_options,SIGNAL(showAnimationsChanged(bool)),ui->scrollWidget,SLOT(setAnimatedTransitions(bool)));
    qDebug() << "[MainWindow] Settings Read";

    qDebug() << "[MainWindow] Initializing Bestiary";
    Bestiary::Initialize();
    qDebug() << "[MainWindow] Bestiary Initialized";

    qDebug() << "[MainWindow] Initializing BasicDateServer";
    BasicDateServer::Initialize(_options->getCalendarFileName());
    BasicDateServer* dateServer = BasicDateServer::Instance();
    connect(_options, &OptionsContainer::calendarFileNameChanged, dateServer, &BasicDateServer::readDateInformation);
    //connect(_options, SIGNAL(calendarFileNameChanged(const QString&)), dateServer, SLOT(readDateInformation(const QString&)));
    qDebug() << "[MainWindow] BasicDateServer Initialized";

    qDebug() << "[MainWindow] Initializing EquipmentServer";
    EquipmentServer::Initialize(_options->getEquipmentFileName());
    EquipmentServer* equipmentServer = EquipmentServer::Instance();
    connect(_options, &OptionsContainer::equipmentFileNameChanged, equipmentServer, &EquipmentServer::readEquipment);
    qDebug() << "[MainWindow] BasicDateServer Initialized";

    // File Menu
    /*
    connect(ui->action_NewCampaign,SIGNAL(triggered()),this,SLOT(newCampaign()));
    connect(ui->action_OpenCampaign,SIGNAL(triggered()),this,SLOT(openFileDialog()));
    connect(ui->action_SaveCampaign,SIGNAL(triggered()),this,SLOT(saveCampaign()));
    connect(ui->actionSave_Campaign_As,SIGNAL(triggered()),this,SLOT(saveCampaignAs()));
    connect(ui->actionClose_Campaign,SIGNAL(triggered()),this,SLOT(closeCampaign()));
    connect(ui->actionE_xit,SIGNAL(triggered()),this,SLOT(close()));
    */
    connect(_ribbonTabFile, SIGNAL(newClicked()), this, SLOT(newCampaign()));
    connect(_ribbonTabFile, SIGNAL(openClicked()), this, SLOT(openFileDialog()));
    connect(_ribbonTabFile, SIGNAL(saveClicked()), this, SLOT(saveCampaign()));
    connect(_ribbonTabFile, SIGNAL(saveAsClicked()), this, SLOT(saveCampaignAs()));
    connect(_ribbonTabFile, SIGNAL(optionsClicked()), _options, SLOT(editSettings()));
    connect(_ribbonTabFile, SIGNAL(closeClicked()), this, SLOT(closeCampaign()));

    // Campaign Menu
    connect(this,SIGNAL(campaignLoaded(Campaign*)),this,SLOT(handleCampaignLoaded(Campaign*)));
    connect(this,SIGNAL(campaignLoaded(Campaign*)),this,SLOT(clearDirty()));
    /*
    connect(ui->actionNew_Adventure,SIGNAL(triggered()),this,SLOT(newAdventure()));
    connect(ui->actionNew_Character,SIGNAL(triggered()),this,SLOT(newCharacter()));
    connect(ui->actionNew_Text_Encounter,SIGNAL(triggered()),this,SLOT(newTextEncounter()));
    connect(ui->actionNew_Battle_Encounter,SIGNAL(triggered()),this,SLOT(newBattleEncounter()));
    connect(ui->actionNew_Scrolling_Text_Encounter,SIGNAL(triggered()),this,SLOT(newScrollingTextEncounter()));
    connect(ui->actionNew_Map,SIGNAL(triggered()),this,SLOT(newMap()));
    connect(ui->actionExport_Item,SIGNAL(triggered()),this,SLOT(exportCurrentItem()));
    connect(ui->actionImport_Item,SIGNAL(triggered()),this,SLOT(importItem()));
    connect(ui->actionStart_Battle,SIGNAL(triggered()),this,SLOT(handleStartNewBattle()));
    connect(ui->action_Import_Character,SIGNAL(triggered()),this,SLOT(importCharacter()));
    connect(ui->actionImport_NPC,SIGNAL(triggered()),this,SLOT(importNPC()));
    connect(ui->actionOpen_Players_Window,SIGNAL(triggered()),this,SLOT(showPublishWindow()));
    */
    connect(_ribbonTabCampaign, SIGNAL(newPartyClicked()), this, SLOT(newParty()));
    connect(_ribbonTabCampaign, SIGNAL(newCharacterClicked()), this, SLOT(newCharacter()));
    connect(_ribbonTabCampaign, SIGNAL(newMapClicked()), this, SLOT(newMap()));
    //connect(_ribbonTabCampaign, SIGNAL(newNPCClicked()), this, SLOT(newNPC()));
    connect(_ribbonTabCampaign, SIGNAL(newTextClicked()), this, SLOT(newTextEncounter()));
    connect(_ribbonTabCampaign, SIGNAL(newBattleClicked()), this, SLOT(newBattleEncounter()));
    connect(_ribbonTabCampaign, SIGNAL(newScrollingTextClicked()), this, SLOT(newScrollingTextEncounter()));
    connect(_ribbonTabCampaign, SIGNAL(removeItemClicked()), this, SLOT(removeCurrentItem()));
    connect(_ribbonTabCampaign, SIGNAL(exportItemClicked()), this, SLOT(exportCurrentItem()));
    connect(_ribbonTabCampaign, SIGNAL(importItemClicked()), this, SLOT(importItem()));
    connect(_ribbonTabCampaign, SIGNAL(importCharacterClicked()), this, SLOT(importCharacter()));
    //connect(_ribbonTabCampaign, SIGNAL(importNPCClicked()), this, SLOT(importNPC()));
    //connect(_ribbonTabCampaign, SIGNAL(playersWindowClicked(bool)), this, SLOT(showPublishWindow(bool)));
    enableCampaignMenu();

    // Tools Menu
    /*
    connect(ui->action_Open_Bestiary,SIGNAL(triggered()),this,SLOT(openBestiary()));
    connect(ui->actionExport_Bestiary,SIGNAL(triggered()),this,SLOT(exportBestiary()));
    connect(ui->actionImport_Bestiary,SIGNAL(triggered()),this,SLOT(importBestiary()));
    connect(ui->action_New_Monster,SIGNAL(triggered()),&bestiaryDlg,SLOT(createNewMonster()));
    connect(ui->actionDice,SIGNAL(triggered()),this,SLOT(openDiceDialog()));
    connect(ui->actionPublish_Text,SIGNAL(triggered()),this,SLOT(openTextPublisher()));
    connect(ui->actionTranslate_Text,SIGNAL(triggered()),this,SLOT(openTextTranslator()));
    connect(ui->actionRandom_Market,SIGNAL(triggered()),this,SLOT(openRandomMarkets()));
#ifdef INCLUDE_CHASE_SUPPORT
    connect(ui->action_Chase_Dialog,SIGNAL(triggered()),this,SLOT(startChase()));
#endif
    connect(ui->actionOptions,SIGNAL(triggered()),_options,SLOT(editSettings()));
    */
    connect(_ribbonTabTools, SIGNAL(bestiaryClicked()), this, SLOT(openBestiary()));
    connect(_ribbonTabTools, SIGNAL(exportBestiaryClicked()), this, SLOT(exportBestiary()));
    connect(_ribbonTabTools, SIGNAL(importBestiaryClicked()), this, SLOT(importBestiary()));
    // TODO: connect(ui->action_New_Monster,SIGNAL(triggered()),&bestiaryDlg,SLOT(createNewMonster()));
    connect(_ribbonTabTools, SIGNAL(rollDiceClicked()), this, SLOT(openDiceDialog()));
    connect(_ribbonTabTools, SIGNAL(publishTextClicked()), this, SLOT(openTextPublisher()));
    connect(_ribbonTabTools, SIGNAL(translateTextClicked()), this, SLOT(openTextTranslator()));
    connect(_ribbonTabTools, SIGNAL(randomMarketClicked()), this, SLOT(openRandomMarkets()));

    // Help Menu
    /*
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(openAboutDialog()));
    connect(ui->actionCheck_For_Updates, SIGNAL(triggered()), this, SLOT(checkForUpdates()));
    */
    //connect(_ribbonTabHelp, SIGNAL(checkForUpdatesClicked()), this, SLOT(checkForUpdates()));
    //connect(_ribbonTabHelp, SIGNAL(aboutClicked()), this, SLOT(openAboutDialog()));
    connect(_ribbonTabFile, SIGNAL(checkForUpdatesClicked()), this, SLOT(checkForUpdates()));
    connect(_ribbonTabFile, SIGNAL(aboutClicked()), this, SLOT(openAboutDialog()));

    connect(ui->treeView,SIGNAL(expanded(QModelIndex)),this,SLOT(handleTreeItemExpanded(QModelIndex)));
    connect(ui->treeView,SIGNAL(collapsed(QModelIndex)),this,SLOT(handleTreeItemCollapsed(QModelIndex)));

    // Battle Menu
    // connections set up elsewhere

    // Map Menu
    connect(_options, SIGNAL(pointerFileNameChanged(const QString&)), _ribbonTabMap, SLOT(setPointerFile(const QString&)));
    _ribbonTabMap->setPointerFile(_options->getPointerFile());

    // Mini Map Menu
    // connections set up elsewhere

    // Text Menu
    // connections set up elsewhere


    qDebug() << "[MainWindow] Creating Player's Window";
    pubWindow = new PublishWindow(QString("DM Helper Player's Window"));
    pubWindow->setPointerFile(_options->getPointerFile());
    connect(_options, SIGNAL(pointerFileNameChanged(const QString&)), pubWindow, SLOT(setPointerFile(const QString&)));
    connect(pubWindow, SIGNAL(windowVisible(bool)), _ribbon->getPublishRibbon(), SLOT(setPlayersWindow(bool)));
    connect(_ribbon->getPublishRibbon(), SIGNAL(colorChanged(QColor)), pubWindow, SLOT(setBackgroundColor(QColor)));
    qDebug() << "[MainWindow] Player's Window Created";

    qDebug() << "[MainWindow] Creating Tree Model";
    ui->treeView->setHeaderHidden(true);
    treeModel = new CampaignTreeModel(ui->treeView);
    ui->treeView->setModel(treeModel);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(ui->treeView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(handleCustomContextMenu(QPoint)));
    connect(ui->treeView->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(handleTreeItemSelected(QModelIndex,QModelIndex)));
    connect(ui->treeView,SIGNAL(activated(QModelIndex)),this,SLOT(handleTreeItemDoubleClicked(QModelIndex)));
    connect(treeModel, &CampaignTreeModel::campaignChanged, ui->treeView, &CampaignTree::campaignChanged);
    connect(treeModel, &CampaignTreeModel::itemMoved, ui->treeView, &CampaignTree::handleItemMoved);
    connect(treeModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(handleTreeItemChanged(QStandardItem*)));
    qDebug() << "[MainWindow] Tree Model Created";

    connect(Bestiary::Instance(),SIGNAL(changed()),&bestiaryDlg,SLOT(dataChanged()));

    qDebug() << "[MainWindow] Loading Bestiary";
#ifndef Q_OS_MAC
    splash.showMessage(QString("Initializing Bestiary...\n"),Qt::AlignBottom | Qt::AlignHCenter);
#endif
    qApp->processEvents();
    readBestiary();
    qDebug() << "[MainWindow] Bestiary Loaded";

    connect(this, SIGNAL(dispatchPublishImage(QImage)), this, SLOT(showPublishWindow()));
    connect(this, SIGNAL(dispatchPublishImage(QImage, QColor)), this, SLOT(showPublishWindow()));
    connect(this, SIGNAL(dispatchPublishImage(QImage)), pubWindow, SLOT(setImage(QImage)));
    connect(this, SIGNAL(dispatchPublishImage(QImage, QColor)), pubWindow, SLOT(setImage(QImage, QColor)));
    connect(this, SIGNAL(dispatchAnimateImage(QImage)), pubWindow, SLOT(setImageNoScale(QImage)));
    connect(this, SIGNAL(dispatchAnimateImage(QImage)), this, SLOT(handleAnimationPreview(QImage)));

    connect(&bestiaryDlg,SIGNAL(publishMonsterImage(QImage, QColor)),this,SIGNAL(dispatchPublishImage(QImage, QColor)));

    // Add the encounter pages to the stacked widget - implicit mapping to EncounterType enum values
    // TODO: insert a MAP in between to be explicit about this mapping
    qDebug() << "[MainWindow] Creating Encounter Pages";
    // Empty Campaign Page
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_Base, new EmptyCampaignFrame);

    //ui->stackedWidgetEncounter->removeWidget(ui->stackedWidgetEncounter->widget(0));

    // EncounterType_Text
    encounterTextEdit = new EncounterTextEdit;
    connect(encounterTextEdit, SIGNAL(anchorClicked(QUrl)), this, SLOT(linkActivated(QUrl)));
    connect(_ribbonTabText, SIGNAL(colorChanged(QColor)), encounterTextEdit, SLOT(setColor(QColor)));
    connect(_ribbonTabText, SIGNAL(fontFamilyChanged(const QString&)), encounterTextEdit, SLOT(setFont(const QString&)));
    connect(_ribbonTabText, SIGNAL(fontSizeChanged(int)), encounterTextEdit, SLOT(setFontSize(int)));
    connect(_ribbonTabText, SIGNAL(fontBoldChanged(bool)), encounterTextEdit, SLOT(setBold(bool)));
    connect(_ribbonTabText, SIGNAL(fontItalicsChanged(bool)), encounterTextEdit, SLOT(setItalics(bool)));
    connect(_ribbonTabText, SIGNAL(alignmentChanged(Qt::Alignment)), encounterTextEdit, SLOT(setAlignment(Qt::Alignment)));
    connect(encounterTextEdit, SIGNAL(colorChanged(QColor)), _ribbonTabText, SLOT(setColor(QColor)));
    connect(encounterTextEdit, SIGNAL(fontFamilyChanged(const QString&)), _ribbonTabText, SLOT(setFontFamily(const QString&)));
    connect(encounterTextEdit, SIGNAL(fontSizeChanged(int)), _ribbonTabText, SLOT(setFontSize(int)));
    connect(encounterTextEdit, SIGNAL(fontBoldChanged(bool)), _ribbonTabText, SLOT(setFontBold(bool)));
    connect(encounterTextEdit, SIGNAL(fontItalicsChanged(bool)), _ribbonTabText, SLOT(setFontItalics(bool)));
    connect(encounterTextEdit, SIGNAL(alignmentChanged(Qt::Alignment)), _ribbonTabText, SLOT(setAlignment(Qt::Alignment)));
    ui->stackedWidgetEncounter->addFrames(QList<int>({DMHelper::CampaignType_Campaign,
                                                      DMHelper::CampaignType_Text,
                                                      DMHelper::CampaignType_Placeholder}), encounterTextEdit);
    qDebug() << "[MainWindow]     Adding Text Encounter widget as page #" << ui->stackedWidgetEncounter->count() - 1;

    // EncounterType_Battle
    /*
    EncounterBattleEdit* encounterBattleEdit = new EncounterBattleEdit;
    connect(encounterBattleEdit, SIGNAL(openMonster(QString)), this, SLOT(openMonster(QString)));
    ui->stackedWidgetEncounter->addWidget(encounterBattleEdit);
    */
    BattleFrame* battleFrame = new BattleFrame;
    battleFrame->setShowOnDeck(_options->getShowOnDeck());
    battleFrame->setShowCountdown(_options->getShowCountdown());
    battleFrame->setCountdownDuration(_options->getCountdownDuration());
    battleFrame->setPointerFile(_options->getPointerFile());
    connect(_options, SIGNAL(showOnDeckChanged(bool)), battleFrame, SLOT(setShowOnDeck(bool)));
    connect(_options, SIGNAL(showCountdownChanged(bool)), battleFrame, SLOT(setShowCountdown(bool)));
    connect(_options, SIGNAL(countdownDurationChanged(int)), battleFrame, SLOT(setCountdownDuration(int)));
    connect(_options, SIGNAL(pointerFileNameChanged(const QString&)), battleFrame, SLOT(setPointerFile(const QString&)));
    connect(pubWindow, SIGNAL(frameResized(QSize)), battleFrame, SLOT(setTargetSize(QSize)));
    connect(battleFrame, SIGNAL(characterSelected(QUuid)), this, SLOT(openCharacter(QUuid)));
    connect(battleFrame, SIGNAL(monsterSelected(QString)), this, SLOT(openMonster(QString)));
//    connect(battleFrame, SIGNAL(publishImage(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    connect(battleFrame, SIGNAL(animateImage(QImage)), this, SIGNAL(dispatchAnimateImage(QImage)));
    connect(battleFrame, SIGNAL(animateImage(QImage)), pubWindow, SLOT(setBackgroundColor()));
    connect(battleFrame, SIGNAL(animationStarted()), this, SLOT(handleAnimationStarted()));
    connect(battleFrame, SIGNAL(showPublishWindow()), this, SLOT(showPublishWindow()));
    connect(battleFrame, SIGNAL(modelChanged(BattleDialogModel*)), this, SLOT(battleModelChanged(BattleDialogModel*)));
    connect(_ribbonTabBattle, SIGNAL(newMapClicked()), battleFrame, SLOT(selectBattleMap()));
    connect(_ribbonTabBattle, SIGNAL(reloadMapClicked()), battleFrame, SLOT(reloadMap()));
    connect(_ribbonTabBattle, SIGNAL(addCharacterClicked()), battleFrame, SLOT(addCharacter()));
    connect(_ribbonTabBattle, SIGNAL(addMonsterClicked()), battleFrame, SLOT(addMonsters()));
    connect(_ribbonTabBattle, SIGNAL(addNPCClicked()), battleFrame, SLOT(addNPC()));
    //connect(_ribbonTabBattle, SIGNAL(showMovementClicked(bool)), battleFrame, SLOT(setShowMovement(bool)));
    //connect(_ribbonTabBattle, SIGNAL(lairActionsClicked(bool)), battleFrame, SLOT(setLairActions(bool)));
    connect(_ribbonTabBattle, SIGNAL(nextClicked()), battleFrame, SLOT(next()));
    connect(_ribbonTabBattle, SIGNAL(sortClicked()), battleFrame, SLOT(sort()));
    connect(_ribbonTabBattle, SIGNAL(statisticsClicked()), battleFrame, SLOT(showStatistics()));

    connect(_ribbonTabMap, SIGNAL(zoomInClicked()), battleFrame, SLOT(zoomIn()));
    connect(_ribbonTabMap, SIGNAL(zoomOutClicked()), battleFrame, SLOT(zoomOut()));
    connect(_ribbonTabMap, SIGNAL(zoomFullClicked()), battleFrame, SLOT(zoomFit()));
    connect(_ribbonTabMap, SIGNAL(zoomSelectClicked(bool)), battleFrame, SLOT(zoomSelect(bool)));
    connect(battleFrame, SIGNAL(zoomSelectToggled(bool)), _ribbonTabMap, SLOT(setZoomSelect(bool)));

    connect(_ribbonTabMap, SIGNAL(cameraCoupleClicked(bool)), battleFrame, SLOT(setCameraCouple(bool)));
    connect(_ribbonTabMap, SIGNAL(cameraZoomClicked()), battleFrame, SLOT(setCameraMap()));
    connect(_ribbonTabMap, SIGNAL(cameraZoomClicked()), battleFrame, SLOT(cancelCameraCouple()));
    connect(_ribbonTabMap, SIGNAL(cameraSelectClicked(bool)), battleFrame, SLOT(setCameraSelect(bool)));
    connect(_ribbonTabMap, SIGNAL(cameraSelectClicked(bool)), battleFrame, SLOT(cancelCameraCouple()));
    connect(battleFrame, SIGNAL(cameraSelectToggled(bool)), _ribbonTabMap, SLOT(setCameraSelect(bool)));
    connect(_ribbonTabMap, SIGNAL(cameraEditClicked(bool)), battleFrame, SLOT(setCameraEdit(bool)));
    connect(_ribbonTabMap, SIGNAL(cameraEditClicked(bool)), battleFrame, SLOT(cancelCameraCouple()));
    connect(battleFrame, SIGNAL(cameraEditToggled(bool)), _ribbonTabMap, SLOT(setCameraEdit(bool)));

    connect(_ribbonTabMap, SIGNAL(distanceClicked(bool)), battleFrame, SLOT(setDistance(bool)));
    connect(_ribbonTabMap, SIGNAL(heightChanged(bool, qreal)), battleFrame, SLOT(setDistanceHeight(bool, qreal)));
    connect(battleFrame, SIGNAL(distanceToggled(bool)), _ribbonTabMap, SLOT(setDistanceOn(bool)));
    connect(battleFrame, SIGNAL(distanceChanged(const QString&)), _ribbonTabMap, SLOT(setDistance(const QString&)));

    connect(_ribbonTabMap, SIGNAL(gridClicked(bool)), battleFrame, SLOT(setGridVisible(bool)));
    connect(_ribbonTabMap, SIGNAL(gridScaleChanged(int)), battleFrame, SLOT(setGridScale(int)));
    connect(_ribbonTabMap, SIGNAL(gridXOffsetChanged(int)), battleFrame, SLOT(setXOffset(int)));
    connect(_ribbonTabMap, SIGNAL(gridYOffsetChanged(int)), battleFrame, SLOT(setYOffset(int)));

    connect(_ribbonTabMap, SIGNAL(editFoWClicked(bool)), battleFrame, SLOT(setFoWEdit(bool)));
    connect(battleFrame, SIGNAL(foWEditToggled(bool)), _ribbonTabMap, SLOT(setEditFoW(bool)));
    connect(_ribbonTabMap, SIGNAL(selectFoWClicked(bool)), battleFrame, SLOT(setFoWSelect(bool)));
    connect(battleFrame, SIGNAL(foWSelectToggled(bool)), _ribbonTabMap, SLOT(setSelectFoW(bool)));
    BattleFrameMapDrawer* mapDrawer = battleFrame->getMapDrawer();
    connect(_ribbonTabMap, SIGNAL(drawEraseClicked(bool)), mapDrawer, SLOT(setErase(bool)));
    connect(_ribbonTabMap, SIGNAL(smoothClicked(bool)), mapDrawer, SLOT(setSmooth(bool)));
    connect(_ribbonTabMap, SIGNAL(brushSizeChanged(int)), mapDrawer, SLOT(setSize(int)));
    connect(_ribbonTabMap, SIGNAL(fillFoWClicked()), mapDrawer, SLOT(fillFoW()));
    connect(_ribbonTabMap, SIGNAL(brushModeChanged(int)), mapDrawer, SLOT(setBrushMode(int)));

    connect(_ribbonTabMap, SIGNAL(pointerClicked(bool)), battleFrame, SLOT(setPointerOn(bool)));
    connect(battleFrame, SIGNAL(pointerToggled(bool)), _ribbonTabMap, SLOT(setPointerOn(bool)));

    //PublishButtonRibbon* publishButtonRibbon = _ribbonTabMap->getPublishRibbon();
    //connect(publishButtonRibbon, SIGNAL(rotateCW()), battleFrame, SLOT(rotateCW()));
    //connect(publishButtonRibbon, SIGNAL(rotateCCW()), battleFrame, SLOT(rotateCCW()));
    //connect(publishButtonRibbon, SIGNAL(rotateChanged(int)), battleFrame, SLOT(setRotation(int)));
    //connect(publishButtonRibbon, SIGNAL(colorChanged(QColor)), battleFrame, SLOT(setBackgroundColor(QColor)));
    //connect(publishButtonRibbon, SIGNAL(toggled(bool)), battleFrame, SLOT(togglePublishing(bool)));
    //connect(publishButtonRibbon, SIGNAL(clicked(bool)), battleFrame, SLOT(publishClicked(bool)));
    //connect(battleFrame, SIGNAL(publishCancelled()), publishButtonRibbon, SLOT(cancelPublish()));
    //connect(battleFrame, SIGNAL(setPublishEnabled(bool)), publishButtonRibbon, SLOT(setEnabled(bool)));
    //connect(battleFrame, SIGNAL(setPublishColor(QColor)), publishButtonRibbon, SLOT(setColor(QColor))); backgroundColorChanged

    connect(this, SIGNAL(cancelSelect()), battleFrame, SLOT(cancelSelect()));

    ui->stackedWidgetEncounter->addFrames(QList<int>({DMHelper::CampaignType_Battle,
                                                      DMHelper::CampaignType_BattleContent}), battleFrame);
    qDebug() << "[MainWindow]     Adding Battle Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;

    // EncounterType_Character
    /*
    QScrollArea* scrollArea = new QScrollArea;
    CharacterFrame* charFrame = new CharacterFrame;
    scrollArea->setWidget(charFrame);
    ui->stackedWidgetEncounter->addFrame(scrollArea);
    connect(charFrame, SIGNAL(publishCharacterImage(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    */
    CharacterFrame* charFrame = new CharacterFrame;
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_Combatant, charFrame);
    qDebug() << "[MainWindow]     Adding Character Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;
    connect(charFrame, SIGNAL(publishCharacterImage(QImage)), this, SIGNAL(dispatchPublishImage(QImage)));

    PartyFrame* partyFrame = new PartyFrame;
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_Party, partyFrame);
    qDebug() << "[MainWindow]     Adding Party Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;
    connect(partyFrame, SIGNAL(publishPartyImage(QImage)), this, SIGNAL(dispatchPublishImage(QImage)));
    //connect(treeModel,SIGNAL(itemChanged(QStandardItem*)),partyFrame,SLOT(loadCharacters()));
    connect(this, SIGNAL(characterChanged(QUuid)), partyFrame, SLOT(handleCharacterChanged(QUuid)));

    // EncounterType_Map
    MapFrame* mapFrame = new MapFrame;
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_Map, mapFrame);
    qDebug() << "[MainWindow]     Adding Map Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;
    connect(mapFrame,SIGNAL(publishImage(QImage)),this,SIGNAL(dispatchPublishImage(QImage)));
    connect(mapFrame, SIGNAL(animateImage(QImage)), this, SIGNAL(dispatchAnimateImage(QImage)));
    //connect(mapFrame, SIGNAL(animationStarted(QColor)), this, SLOT(handleAnimationStarted(QColor)));
    connect(mapFrame, SIGNAL(animationStarted()), this, SLOT(handleAnimationStarted()));
    connect(mapFrame, SIGNAL(showPublishWindow()), this, SLOT(showPublishWindow()));
    connect(pubWindow, SIGNAL(frameResized(QSize)), mapFrame, SLOT(targetResized(QSize)));

    connect(_ribbonTabMiniMap, SIGNAL(zoomInClicked()), mapFrame, SLOT(zoomIn()));
    connect(_ribbonTabMiniMap, SIGNAL(zoomOutClicked()), mapFrame, SLOT(zoomOut()));
    connect(_ribbonTabMiniMap, SIGNAL(zoomOneClicked()), mapFrame, SLOT(zoomOne()));
    connect(_ribbonTabMiniMap, SIGNAL(zoomFullClicked()), mapFrame, SLOT(zoomFit()));
    connect(_ribbonTabMiniMap, SIGNAL(zoomSelectClicked(bool)), mapFrame, SLOT(zoomSelect(bool)));
    connect(mapFrame, SIGNAL(zoomSelectChanged(bool)), _ribbonTabMiniMap, SLOT(setZoomSelect(bool)));

    connect(_ribbonTabMiniMap, SIGNAL(drawEraseClicked(bool)), mapFrame, SLOT(setErase(bool)));
    connect(_ribbonTabMiniMap, SIGNAL(smoothClicked(bool)), mapFrame, SLOT(setSmooth(bool)));
    connect(_ribbonTabMiniMap, SIGNAL(brushSizeChanged(int)), mapFrame, SLOT(brushSizeChanged(int)));
    connect(_ribbonTabMiniMap, SIGNAL(fillFoWClicked()), mapFrame, SLOT(fillFoW()));
    connect(_ribbonTabMiniMap, SIGNAL(brushModeChanged(int)), mapFrame, SLOT(setBrushMode(int)));
    connect(mapFrame, SIGNAL(brushModeSet(int)), _ribbonTabMiniMap, SLOT(setBrushMode(int)));

    connect(_ribbonTabMiniMap, SIGNAL(publishZoomChanged(bool)), mapFrame, SLOT(setPublishZoom(bool)));
    connect(_ribbonTabMiniMap, SIGNAL(publishVisibleChanged(bool)), mapFrame, SLOT(setPublishVisible(bool)));

    //publishButtonRibbon = _ribbonTabMiniMap->getPublishRibbon();
    //connect(publishButtonRibbon, SIGNAL(rotationChanged(int)), mapFrame, SLOT(setRotation(int)));
    //connect(publishButtonRibbon, SIGNAL(colorChanged(QColor)), mapFrame, SLOT(setColor(QColor)));
    //connect(publishButtonRibbon, SIGNAL(clicked(bool)), mapFrame, SLOT(publishFoWImage(bool)));
    //connect(mapFrame, SIGNAL(publishCancelled()), publishButtonRibbon, SLOT(cancelPublish()));
    //connect(mapFrame, SIGNAL(publishCheckable(bool)), publishButtonRibbon, SLOT(setCheckable(bool)));
    //mapFrame->setRotation(publishButtonRibbon->getRotation());
    //mapFrame->setColor(publishButtonRibbon->getColor());

    connect(this, SIGNAL(cancelSelect()), battleFrame, SLOT(cancelSelect()));

    // EncounterType_ScrollingText
    _scrollingTextEdit = new EncounterScrollingTextEdit;
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_ScrollingText, _scrollingTextEdit);
    qDebug() << "[MainWindow]     Adding Scrolling Encounter widget as page #" << ui->stackedWidgetEncounter->count() - 1;
    connect(_scrollingTextEdit, SIGNAL(animateImage(QImage)), this, SIGNAL(dispatchAnimateImage(QImage)));
    connect(_scrollingTextEdit, SIGNAL(animationStarted()), this, SLOT(handleAnimationStarted()));
    connect(_scrollingTextEdit, SIGNAL(showPublishWindow()), this, SLOT(showPublishWindow()));
    connect(pubWindow, SIGNAL(frameResized(QSize)), _scrollingTextEdit, SLOT(targetResized(QSize)));
    connect(_ribbonTabScrolling, SIGNAL(backgroundClicked()), _scrollingTextEdit, SLOT(browseImageFile()));
    connect(_ribbonTabScrolling, SIGNAL(speedChanged(int)), _scrollingTextEdit, SLOT(setScrollSpeed(int)));
    connect(_ribbonTabScrolling, SIGNAL(widthChanged(int)), _scrollingTextEdit, SLOT(setImageWidth(int)));
    connect(_ribbonTabScrolling, SIGNAL(rewindClicked()), _scrollingTextEdit, SLOT(rewind()));
    connect(_scrollingTextEdit, SIGNAL(scrollSpeedChanged(int)), _ribbonTabScrolling, SLOT(setSpeed(int)));
    connect(_scrollingTextEdit, SIGNAL(imageWidthChanged(int)), _ribbonTabScrolling, SLOT(setWidth(int)));
    connect(_ribbonTabScrolling, SIGNAL(colorChanged(QColor)), _scrollingTextEdit, SLOT(setColor(QColor)));
    connect(_ribbonTabScrolling, SIGNAL(fontFamilyChanged(const QString&)), _scrollingTextEdit, SLOT(setFontFamily(const QString&)));
    connect(_ribbonTabScrolling, SIGNAL(fontSizeChanged(int)), _scrollingTextEdit, SLOT(setFontSize(int)));
    connect(_ribbonTabScrolling, SIGNAL(fontBoldChanged(bool)), _scrollingTextEdit, SLOT(setFontBold(bool)));
    connect(_ribbonTabScrolling, SIGNAL(fontItalicsChanged(bool)), _scrollingTextEdit, SLOT(setFontItalics(bool)));
    connect(_ribbonTabScrolling, SIGNAL(alignmentChanged(Qt::Alignment)), _scrollingTextEdit, SLOT(setAlignment(Qt::Alignment)));
    connect(_scrollingTextEdit, SIGNAL(colorChanged(QColor)), _ribbonTabScrolling, SLOT(setColor(QColor)));
    connect(_scrollingTextEdit, SIGNAL(fontFamilyChanged(const QString&)), _ribbonTabScrolling, SLOT(setFontFamily(const QString&)));
    connect(_scrollingTextEdit, SIGNAL(fontSizeChanged(int)), _ribbonTabScrolling, SLOT(setFontSize(int)));
    connect(_scrollingTextEdit, SIGNAL(fontBoldChanged(bool)), _ribbonTabScrolling, SLOT(setFontBold(bool)));
    connect(_scrollingTextEdit, SIGNAL(fontItalicsChanged(bool)), _ribbonTabScrolling, SLOT(setFontItalics(bool)));
    connect(_scrollingTextEdit, SIGNAL(alignmentChanged(Qt::Alignment)), _ribbonTabScrolling, SLOT(setAlignment(Qt::Alignment)));

    // EncounterType_AudioTrack
    AudioTrackEdit* audioTrackEdit = new AudioTrackEdit;
    connect(this, SIGNAL(campaignLoaded(Campaign*)), audioTrackEdit, SLOT(setCampaign(Campaign*)));
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_AudioTrack, audioTrackEdit);
    qDebug() << "[MainWindow]     Adding Audio Track widget as page #" << ui->stackedWidgetEncounter->count() - 1;

    // EncounterType_WelcomeScreen
    WelcomeFrame* welcomeFrame = new WelcomeFrame(mruHandler);
    connect(welcomeFrame, SIGNAL(openCampaignFile(QString)), this, SLOT(openFile(QString)));
    /*
    connect(ui->action_Users_Guide, SIGNAL(triggered()), welcomeFrame, SLOT(openUsersGuide()));
    connect(ui->action_Getting_Started, SIGNAL(triggered()), welcomeFrame, SLOT(openGettingStarted()));
    */
    //connect(_ribbonTabHelp, SIGNAL(userGuideClicked()), welcomeFrame, SLOT(openUsersGuide()));
    //connect(_ribbonTabHelp, SIGNAL(gettingStartedClicked()), welcomeFrame, SLOT(openGettingStarted()));
    connect(_ribbonTabFile, SIGNAL(userGuideClicked()), welcomeFrame, SLOT(openUsersGuide()));
    connect(_ribbonTabFile, SIGNAL(gettingStartedClicked()), welcomeFrame, SLOT(openGettingStarted()));
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_WelcomeScreen, welcomeFrame);
    qDebug() << "[MainWindow]     Adding Welcome Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;

    qDebug() << "[MainWindow] Encounter Pages Created";

    // Ensure publishing a single image stops any running animations
    //connect(this, SIGNAL(dispatchPublishImage(QImage,QColor)), battleFrame, SLOT(cancelPublish()));
    connect(this, SIGNAL(dispatchPublishImage(QImage,QColor)), _ribbon->getPublishRibbon(), SLOT(cancelPublish()));
    //connect(this, SIGNAL(dispatchPublishImage(QImage,QColor)), _scrollingTextEdit, SLOT(stopAnimation()));

    // Load the quick reference tabs
#ifndef Q_OS_MAC
    splash.showMessage(QString("Initializing Quick Reference tabs...\n"),Qt::AlignBottom | Qt::AlignHCenter);
#endif
    qApp->processEvents();
    qDebug() << "[MainWindow] Creating Reference Tabs";
    previewFrame = new PublishFrame(this);
    connect(previewFrame,SIGNAL(arrowVisibleChanged(bool)),pubWindow,SLOT(setArrowVisible(bool)));
    connect(previewFrame,SIGNAL(arrowVisibleChanged(bool)),previewFrame,SLOT(setArrowVisible(bool)));
    connect(previewFrame,SIGNAL(positionChanged(QPointF)),previewFrame,SLOT(setArrowPosition(QPointF)));
    connect(previewFrame,SIGNAL(positionChanged(QPointF)),pubWindow,SLOT(setArrowPosition(QPointF)));
    connect(this,SIGNAL(dispatchPublishImage(QImage,QColor)),previewFrame,SLOT(setImage(QImage)));
    connect(_options, SIGNAL(pointerFileNameChanged(const QString&)), previewFrame, SLOT(setPointerFile(const QString&)));
    previewFrame->setPointerFile(_options->getPointerFile());
    previewDlg = createDialog(previewFrame);
    connect(_ribbon->getPublishRibbon(), SIGNAL(previewClicked()), previewDlg, SLOT(exec()));

    /*
    // Add the preview tab
    //previewTab = new ScrollTabWidget(previewFrame, QSizeF(0.9, 0.9), this);
    //previewTab->setToolTip(QString("Preview"));
    //ui->scrollWidget->addTab(previewTab, QIcon(QPixmap(":/img/data/icon_preview.png")));
    connect(_ribbonTabCampaign, SIGNAL(previewClicked()), previewFrame, SLOT(show()));
    // Add the time/date tab
    */
    dmScreenDlg = createDialog(new DMScreenTabWidget(_options->getEquipmentFileName(), this));
    tableDlg = createDialog(new CustomTableFrame(_options->getTablesDirectory(), this));
    QuickRefFrame* quickRefFrame = new QuickRefFrame(_options->getQuickReferenceFileName(), this);
    connect(_options, &OptionsContainer::quickReferenceFileNameChanged, quickRefFrame, &QuickRefFrame::readQuickRef);
    quickRefDlg = createDialog(quickRefFrame);
    AudioPlaybackFrame* audioPlaybackFrame = new AudioPlaybackFrame(this);
    audioPlaybackFrame->setVolume(_options->getAudioVolume());
    connect(_audioPlayer, SIGNAL(positionChanged(qint64)), audioPlaybackFrame, SLOT(setPosition(qint64)));
    connect(_audioPlayer, SIGNAL(durationChanged(qint64)), audioPlaybackFrame, SLOT(setDuration(qint64)));
    connect(_audioPlayer, SIGNAL(trackChanged(AudioTrack*)), audioPlaybackFrame, SLOT(trackChanged(AudioTrack*)));
    connect(_audioPlayer, SIGNAL(stateChanged(AudioPlayer::State)), audioPlaybackFrame, SLOT(stateChanged(AudioPlayer::State)));
    connect(audioPlaybackFrame, SIGNAL(play()), _audioPlayer, SLOT(play()));
    connect(audioPlaybackFrame, SIGNAL(pause()), _audioPlayer, SLOT(pause()));
    connect(audioPlaybackFrame, SIGNAL(positionChanged(qint64)), _audioPlayer, SLOT(setPosition(qint64)));
    connect(audioPlaybackFrame, SIGNAL(volumeChanged(int)), _audioPlayer, SLOT(setVolume(int)));
    connect(audioPlaybackFrame, SIGNAL(volumeChanged(int)), _options, SLOT(setAudioVolume(int)));
    soundDlg = createDialog(audioPlaybackFrame);
    timeAndDateFrame = new TimeAndDateFrame(this);
    calendarDlg = createDialog(timeAndDateFrame);
    countdownDlg = createDialog(new CountdownFrame(this));

    connect(_ribbonTabTools, SIGNAL(screenClicked()), dmScreenDlg, SLOT(exec()));
    connect(_ribbonTabTools, SIGNAL(tablesClicked()), tableDlg, SLOT(show()));
    connect(_ribbonTabTools, SIGNAL(referenceClicked()), quickRefDlg, SLOT(exec()));
    connect(_ribbonTabTools, SIGNAL(soundboardClicked()), soundDlg, SLOT(exec()));
    connect(_ribbonTabTools, SIGNAL(calendarClicked()), calendarDlg, SLOT(exec()));
    connect(_ribbonTabTools, SIGNAL(countdownClicked()), countdownDlg, SLOT(exec()));

    /*
    timeAndDateFrame->setToolTip(QString("Time & Date"));
    ui->scrollWidget->addTab(new ScrollTabWidget(timeAndDateFrame, QSizeF(0,0), this), QIcon(QPixmap(":/img/data/icon_clock.png")));
    // Add the quick reference frame
    QuickRefFrame* quickRefFrame = new QuickRefFrame(_options->getQuickReferenceFileName(), this);
    connect(_options, &OptionsContainer::quickReferenceFileNameChanged, quickRefFrame, &QuickRefFrame::readQuickRef);
    ScrollTabWidget* w2 = new ScrollTabWidget(quickRefFrame, QSizeF(0,0), this);
    w2->setToolTip(QString("Quick Reference"));
    ui->scrollWidget->addTab(w2, QIcon(QPixmap(":/img/data/icon_reference.png")));
    // Add the DM screen widget
    DMScreenTabWidget* dmScreen = new DMScreenTabWidget(_options->getEquipmentFileName(), this);
    dmScreen->setToolTip(QString("DM reference"));
    ui->scrollWidget->addTab(new ScrollTabWidget(dmScreen, QSizeF(0,0), this), QIcon(QPixmap(":/img/data/icon_screen.png")));
    // Add the dice frame
    DiceRollFrame* diceFrame = new DiceRollFrame(this);
    diceFrame->setToolTip(QString("Dice Roller"));
    ui->scrollWidget->addTab(new ScrollTabWidget(diceFrame, QSizeF(0,0), this), QIcon(QPixmap(":/img/data/icon_dice.png")));
    // Add the countdown timer frame
    CountdownFrame* countdownFrame = new CountdownFrame(this);
    countdownFrame->setToolTip(QString("Countdown Timer"));
    ui->scrollWidget->addTab(new ScrollTabWidget(countdownFrame, QSizeF(0,0), this), QIcon(QPixmap(":/img/data/icon_countdown.png")));
    // Add the custom tableframe
    CustomTableFrame* customTableFrame = new CustomTableFrame(_options->getTablesDirectory(), this);
    customTableFrame->setToolTip(QString("Used-defined Tables"));
    ui->scrollWidget->addTab(new ScrollTabWidget(customTableFrame, QSizeF(0,0), this), QIcon(QPixmap(":/img/data/icon_table.png")));
    // Add the audio playback frame
    AudioPlaybackFrame* audioPlaybackFrame = new AudioPlaybackFrame(this);
    audioPlaybackFrame->setToolTip(QString("Audio Playback"));
    ui->scrollWidget->addTab(new ScrollTabWidget(audioPlaybackFrame, QSizeF(0,0), this), QIcon(QPixmap(":/img/data/icon_music.png")));
    //Initialize animation settings
    ui->scrollWidget->setAnimatedTransitions(_options->getShowAnimations());
    */
    qDebug() << "[MainWindow] Reference Tabs Created";

#ifndef Q_OS_MAC
    splash.showMessage(QString("Preparing DM Helper\n"),Qt::AlignBottom | Qt::AlignHCenter);
#endif
    qApp->processEvents();

    /*
    qDebug() << "[MainWindow] Initializing Battle Dialog Manager";
    _battleDlgMgr = new BattleDialogManager(this);
    connect(ui->actionBattle_Dialog, SIGNAL(triggered()), _battleDlgMgr, SLOT(showBattleDialog()));
    connect(_battleDlgMgr, SIGNAL(battleActive(bool)), ui->actionBattle_Dialog, SLOT(setEnabled(bool)));
    DONE connect(_battleDlgMgr, SIGNAL(characterSelected(QUuid)), this, SLOT(openCharacter(QUuid)));
    DONE connect(_battleDlgMgr, SIGNAL(monsterSelected(QString)), this, SLOT(openMonster(QString)));
    DONE connect(_battleDlgMgr, SIGNAL(publishImage(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    DONE connect(_battleDlgMgr, SIGNAL(animateImage(QImage)), this, SIGNAL(dispatchAnimateImage(QImage)));
    DONE connect(_battleDlgMgr, SIGNAL(animationStarted(QColor)), this, SLOT(handleAnimationStarted(QColor)));
    DONE connect(_battleDlgMgr, SIGNAL(showPublishWindow()), this, SLOT(showPublishWindow()));
    connect(_battleDlgMgr, SIGNAL(dirty()), this, SLOT(setDirty()));
    DONE connect(pubWindow, SIGNAL(frameResized(QSize)), _battleDlgMgr, SLOT(targetResized(QSize)));
    connect(this, SIGNAL(campaignLoaded(Campaign*)), _battleDlgMgr, SLOT(setCampaign(Campaign*)));
    connect(this, SIGNAL(dispatchPublishImage(QImage,QColor)), _battleDlgMgr, SLOT(cancelPublish()));
    DONE _battleDlgMgr->setShowOnDeck(_options->getShowOnDeck());
    DONE _battleDlgMgr->setShowCountdown(_options->getShowCountdown());
    DONE _battleDlgMgr->setCountdownDuration(_options->getCountdownDuration());
    DONE connect(_options, SIGNAL(showOnDeckChanged(bool)), _battleDlgMgr, SLOT(setShowOnDeck(bool)));
    DONE connect(_options, SIGNAL(showCountdownChanged(bool)), _battleDlgMgr, SLOT(setShowCountdown(bool)));
    DONE connect(_options, SIGNAL(countdownDurationChanged(int)), _battleDlgMgr, SLOT(setCountdownDuration(int)));
    connect(encounterBattleEdit, SIGNAL(startBattle(EncounterBattle*)), _battleDlgMgr, SLOT(startNewBattle(EncounterBattle*)));
    connect(encounterBattleEdit, SIGNAL(loadBattle(EncounterBattle*)), _battleDlgMgr, SLOT(loadBattle(EncounterBattle*)));
    connect(encounterBattleEdit, SIGNAL(deleteBattle(EncounterBattle*)), _battleDlgMgr, SLOT(deleteBattle(EncounterBattle*)));
    qDebug() << "[MainWindow] Battle Dialog Manager Initialized.";
    */

    _audioPlayer = new AudioPlayer(this);
    _audioPlayer->setVolume(_options->getAudioVolume());
    //audioPlaybackFrame->setVolume(_options->getAudioVolume());
    connect(audioTrackEdit, SIGNAL(trackSelected(AudioTrack*)), _audioPlayer, SLOT(playTrack(AudioTrack*)));
    //connect(_audioPlayer, SIGNAL(positionChanged(qint64)), audioPlaybackFrame, SLOT(setPosition(qint64)));
    //connect(_audioPlayer, SIGNAL(durationChanged(qint64)), audioPlaybackFrame, SLOT(setDuration(qint64)));
    //connect(_audioPlayer, SIGNAL(trackChanged(AudioTrack*)), audioPlaybackFrame, SLOT(trackChanged(AudioTrack*)));
    //connect(_audioPlayer, SIGNAL(stateChanged(AudioPlayer::State)), audioPlaybackFrame, SLOT(stateChanged(AudioPlayer::State)));
    //connect(audioPlaybackFrame, SIGNAL(play()), _audioPlayer, SLOT(play()));
    //connect(audioPlaybackFrame, SIGNAL(pause()), _audioPlayer, SLOT(pause()));
    //connect(audioPlaybackFrame, SIGNAL(positionChanged(qint64)), _audioPlayer, SLOT(setPosition(qint64)));
    //connect(audioPlaybackFrame, SIGNAL(volumeChanged(int)), _audioPlayer, SLOT(setVolume(int)));
    //connect(audioPlaybackFrame, SIGNAL(volumeChanged(int)), _options, SLOT(setAudioVolume(int)));
    connect(mapFrame, SIGNAL(startTrack(AudioTrack*)), _audioPlayer, SLOT(playTrack(AudioTrack*)));
    //connect(encounterBattleEdit, SIGNAL(startTrack(AudioTrack*)), _audioPlayer, SLOT(playTrack(AudioTrack*)));

#ifdef INCLUDE_NETWORK_SUPPORT
    _networkController = new NetworkController(this);
    _networkController->setNetworkLogin(_options->getURLString(), _options->getUserName(), _options->getPassword(), _options->getSessionID(), QString());
    _networkController->enableNetworkController(_options->getNetworkEnabled());
    connect(this, SIGNAL(dispatchPublishImage(QImage)), _networkController, SLOT(uploadImage(QImage)));
    connect(_audioPlayer, SIGNAL(trackChanged(AudioTrack*)), _networkController, SLOT(uploadTrack(AudioTrack*)));
    connect(_options, SIGNAL(networkEnabledChanged(bool)), _networkController, SLOT(enableNetworkController(bool)));
    connect(_options, SIGNAL(networkSettingsChanged(QString,QString,QString,QString,QString)), _networkController, SLOT(setNetworkLogin(QString,QString,QString,QString,QString)));
    _battleDlgMgr->setNetworkManager(_networkController);
#endif

    emit campaignLoaded(nullptr);

#ifndef Q_OS_MAC
    splash.finish(this);
#endif

    qDebug() << "[MainWindow] Main Initialization complete";
}

MainWindow::~MainWindow()
{
    //delete _battleDlgMgr;

    deleteCampaign();

    delete pubWindow;
    delete ui;

    Bestiary::Shutdown();
    ScaledPixmap::cleanupDefaultPixmap();
}

void MainWindow::newCampaign()
{
    if( !closeCampaign() )
        return;

    bool ok;
    QString campaignName = QInputDialog::getText(this, QString("Enter New Campaign Name"),QString("Campaign"),QLineEdit::Normal,QString(),&ok);
    if(ok)
    {
        if(campaignName.isEmpty())
            campaignName = QString("Campaign");

        campaign = new Campaign(campaignName);
        campaign->addObject(EncounterFactory().createObject(DMHelper::CampaignType_Text, -1, QString("Notes"), false));
        campaign->addObject(EncounterFactory().createObject(DMHelper::CampaignType_Party, -1, QString("Party"), false));
        campaign->addObject(EncounterFactory().createObject(DMHelper::CampaignType_Text, -1, QString("Adventures"), false));
        campaign->addObject(EncounterFactory().createObject(DMHelper::CampaignType_Text, -1, QString("World"), false));
        qDebug() << "[MainWindow] Campaign created: " << campaignName;
        selectItem(DMHelper::TreeType_Campaign, QUuid());
        emit campaignLoaded(campaign);
        setDirty();
    }
}

bool MainWindow::saveCampaign()
{
    if(!campaign)
        return true;

    campaign->validateCampaignIds();
    if(!campaign->isValid())
    {
        QMessageBox::StandardButton result = QMessageBox::critical(this,
                                                                   QString("Invalid Campaign"),
                                                                   QString("An invalid structure has been detected in the open campaign. Saving may corrupt the file and lead to data loss.\n\nIt is strongly recommended to back up your campaign file before saving!\n\nDo you wish to save now?"),
                                                                   QMessageBox::Yes | QMessageBox::No );
        if(result == QMessageBox::No)
        {
            qDebug() << "[MainWindow] Invalid campaign not saved";
            return false;
        }
        else
        {
            qDebug() << "[MainWindow] Invalid campaign saved despite warning!";
        }
    }

    if(campaignFileName.isEmpty())
    {
        campaignFileName = QFileDialog::getSaveFileName(this,QString("Save Campaign"),QString(),QString("XML files (*.xml)"));
        if(campaignFileName.isEmpty())
            return false;
    }

    qDebug() << "[MainWindow] Saving Campaign: " << campaignFileName;

    QDomDocument doc( "DMHelperXML" );

    QDomElement root = doc.createElement( "root" );
    doc.appendChild( root );

    QFileInfo fileInfo(campaignFileName);
    QDir targetDir(fileInfo.absoluteDir());
    campaign->outputXML(doc, root, targetDir, false);

    QFile file(campaignFileName);
    if( !file.open( QIODevice::WriteOnly ) )
    {
        qDebug() << "[MainWindow] Unable to open campaign file for writing: " << campaignFileName;
        qDebug() << "       Error " << file.error() << ": " << file.errorString();
        QFileInfo info(file);
        qDebug() << "       Full filename: " << info.absoluteFilePath();

        campaignFileName.clear();
        return false;
    }

    QTextStream ts( &file );
    ts.setCodec("UTF-8");
    ts << doc.toString();

    file.close();

    clearDirty();

    qDebug() << "[MainWindow] Campaign saved: " << campaignFileName;

    if(_options->getMRUHandler())
        _options->getMRUHandler()->addMRUFile(campaignFileName);

    return true;
}

void MainWindow::saveCampaignAs()
{
    QString previousCampaignFileName = campaignFileName;

    campaignFileName.clear();

    if(saveCampaign() == false)
    {
        campaignFileName = previousCampaignFileName;
    }
}

void MainWindow::openFileDialog()
{
    QString filename = QFileDialog::getOpenFileName(this,QString("Select Campaign"));
    if( (!filename.isNull()) && (!filename.isEmpty()) && (QFile::exists(filename)) )
        openFile(filename);
}

bool MainWindow::closeCampaign()
{
    qDebug() << "[MainWindow] Closing Campaign: " << campaignFileName;

    if(dirty)
    {
        QMessageBox::StandardButton result = QMessageBox::question(this,
                                                                   QString("Save Campaign"),
                                                                   QString("Would you like to save the current campaign before proceeding? Unsaved changes will be lost."),
                                                                   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
        if(result == QMessageBox::Cancel)
        {
            qDebug() << "[MainWindow] Closíng Campaign cancelled";
            return false;
        }

        if(result == QMessageBox::Yes)
        {
            saveCampaign();
        }
        else
        {
            qDebug() << "[MainWindow] User decided not to save Campaign: " << campaignFileName;
        }
    }

    deleteCampaign();
    clearDirty();

    qDebug() << "[MainWindow] Campaign closed";

    return true;
}

void MainWindow::openDiceDialog()
{
    DiceRollDialog *drDlg = new DiceRollDialog(this);
    drDlg->exec();
}

void MainWindow::openCharacter(QUuid id)
{
    selectItem(id);
//    if((!campaign) || (id.isNull()))

//    if((!treeModel) || (id.isNull()))
//        return;

    /*
    Character* selectedChar = campaign->getCharacterById(id);
    if(!selectedChar)
    {
        selectedChar = campaign->getNPCById(id);
        if(!selectedChar)
            return;
    }

    if(treeIndexMap.contains(selectedChar->getName()))
    {
        QModelIndex index = treeIndexMap.value(selectedChar->getName());
        ui->treeView->setCurrentIndex(index);
    }
    */

//    QModelIndex index = treeModel->getObject(id);
//    if(index.isValid())
//        ui->treeView->setCurrentIndex(index);

//    activateWindow();
}

void MainWindow::openMonster(const QString& monsterClass)
{
    if(!monsterClass.isEmpty() && Bestiary::Instance()->exists(monsterClass))
    {
        bestiaryDlg.setMonster(monsterClass);
        openBestiary();
    }
}

void MainWindow::newCharacter()
{
    if(!campaign)
        return;

    bool ok;
    QString characterName = QInputDialog::getText(this,
                                                  QString("New Character"),
                                                  QString("Characters added without a party are treated as NPCs. To add a PC, either create the character directly in a party or drag an NPC into a party.") + QChar::LineFeed + QChar::LineFeed + QString("Enter character name:"),
                                                  QLineEdit::Normal,
                                                  QString(),
                                                  &ok);
    if((!ok) || (characterName.isEmpty()))
        return;

    Character* character = dynamic_cast<Character*>(CombatantFactory().createObject(DMHelper::CampaignType_Combatant, DMHelper::CombatantType_Character, characterName, false));
    //newCharacter->setName(characterName);
    //campaign->addCharacter(newCharacter);
    //campaign->addObject(newCharacter);
    CampaignObjectBase* currentObject = ui->treeView->currentCampaignObject();
    if(!currentObject)
        currentObject = campaign;

    currentObject->setExpanded(true);
    currentObject->addObject(character);

    updateCampaignTree();

    openCharacter(character->getID());
}

void MainWindow::importCharacter()
{
    if(!campaign)
        return;

    CharacterImporter* importer = new CharacterImporter();
    connect(importer, &CharacterImporter::characterImported, this, &MainWindow::openCharacter);
    connect(this, &MainWindow::campaignLoaded, importer, &CharacterImporter::campaignChanged);
    importer->importCharacter(campaign, true);
}

/*
void MainWindow::TEST_DISCORD()
{
    DiscordPoster* poster = new DiscordPoster();
    poster->post();
}
*/

void MainWindow::importItem()
{
    if(!campaign)
        return;

    ObjectImporter importer;
    importer.importObject(*campaign);
}

/*
void MainWindow::newNPC()
{
    bool ok;
    QString npcName = QInputDialog::getText(this, QString("Enter New NPC Name"),QString("NPC"),QLineEdit::Normal,QString(),&ok);
    if((!ok) || (npcName.isEmpty()))
        return;

    Character* newNPC = dynamic_cast<Character*>(CombatantFactory().createObject(DMHelper::CampaignType_Combatant, DMHelper::CombatantType_Character, npcName, false));
//    newNPC->setName(npcName);
    //campaign->addNPC(newNPC);
    campaign->addObject(newNPC);
    openCharacter(newNPC->getID());
}

void MainWindow::importNPC()
{
    if(!campaign)
        return;

    CharacterImporter* importer = new CharacterImporter();
    connect(importer, &CharacterImporter::characterImported, this, &MainWindow::openCharacter);
    connect(this, &MainWindow::campaignLoaded, importer, &CharacterImporter::campaignChanged);
    importer->importCharacter(campaign, false);
}
*/

/*
void MainWindow::removeCurrentCharacter()
{
    if(!campaign)
        return;

    Character* removeCharacter = campaign->getCharacterById(currentCharacter);
    if(!removeCharacter)
        return;

    if(QMessageBox::question(this,QString("Confirm Delete Character"),QString("Are you sure you would like to delete the character ") + removeCharacter->getName() + QString("?")) == QMessageBox::Yes)
    {
        delete campaign->removeCharacter(currentCharacter);
    }
}
*/

void MainWindow::newParty()
{
    newEncounter(DMHelper::CampaignType_Party, QString("New Party"), QString("Enter new party name:"));
}

void MainWindow::newTextEncounter()
{
    //newEncounter(DMHelper::EncounterType_Text);
    newEncounter(DMHelper::CampaignType_Text, QString("New Text Entry"), QString("Enter new entry name:"));
}

void MainWindow::newBattleEncounter()
{
    //newEncounter(DMHelper::EncounterType_Battle);
    CampaignObjectBase* encounter = newEncounter(DMHelper::CampaignType_Battle, QString("New Battle"), QString("Enter new battle name:"));
    if(!encounter)
        return;

    EncounterBattle* battle = dynamic_cast<EncounterBattle*>(encounter);
    if(battle)
        battle->createBattleDialogModel();
}

void MainWindow::newScrollingTextEncounter()
{
    //newEncounter(DMHelper::EncounterType_ScrollingText);
    newEncounter(DMHelper::CampaignType_ScrollingText, QString("New Scrolling Text"), QString("Enter new scrolling text entry name:"));
}

void MainWindow::newMap()
{
    // TODO: throw a message box when it doesn't work.
    if(!campaign)
        return;

    /*
    QStandardItem* parentItem = findParentbyType(treeModel->itemFromIndex(ui->treeView->currentIndex()), DMHelper::TreeType_Adventure);
    if(!parentItem)
    {
        parentItem = findParentbyType(treeModel->itemFromIndex(ui->treeView->currentIndex()), DMHelper::TreeType_Settings_Title);
        if(!parentItem)
        {
            return;
        }
    }
    */

    /*
    Character* character = dynamic_cast<Character*>(CombatantFactory().createObject(DMHelper::CampaignType_Combatant, DMHelper::CombatantType_Character, characterName, false));
    CampaignObjectBase* currentObject = ui->treeView->currentCampaignObject();
    if(!currentObject)
        currentObject = campaign;

    currentObject->setExpanded(true);
    currentObject->addObject(character);

    updateCampaignTree();

    openCharacter(character->getID());

     */
    bool ok = false;
    QString mapName = QInputDialog::getText(this, QString("Enter Map Name"), QString("New Map"), QLineEdit::Normal, QString(), &ok);
    if(!ok)
        return;

    QString filename = QFileDialog::getOpenFileName(this, QString("Select Map Image..."));
    if(filename.isEmpty())
        return;

    CampaignObjectBase* currentObject = ui->treeView->currentCampaignObject();
    if(!currentObject)
        currentObject = campaign;

    //Map* newMap = new Map(mapName, filename);
    Map* map = dynamic_cast<Map*>(MapFactory().createObject(DMHelper::CampaignType_Map, -1, mapName, false));
    if(!map)
        return;

    map->setFileName(filename);

    /*
    Adventure* adventure = campaign->getAdventureById(id);
    if(adventure)
    {
        adventure->addMap(newMap);
    }
    else
    {
        campaign->addSetting(newMap);
    }
    */
    currentObject->setExpanded(true);
    currentObject->addObject(map);

    updateCampaignTree();

    selectItem(map->getID());
}

void MainWindow::editCurrentMap()
{
    //TODO: describe why returning
    if((!campaign)||(!treeModel))
        return;

    QStandardItem* mapItem = treeModel->itemFromIndex(ui->treeView->currentIndex());
    int type = mapItem->data(DMHelper::TreeItemData_Type).toInt();
    if(type == DMHelper::TreeType_Map)
    {
        Map* map = dynamic_cast<Map*>(campaign->getObjectById(QUuid(mapItem->data(DMHelper::TreeItemData_ID).toString())));
        /*
        Adventure* adventure = nullptr;
        QStandardItem* adventureItem = findParentbyType(mapItem, DMHelper::TreeType_Adventure);
        if(adventureItem)
        {
            adventure = campaign->getAdventureById(QUuid(adventureItem->data(DMHelper::TreeItemData_ID).toString()));
        }

        Map* map = nullptr;
        if(adventure)
        {
            map = adventure->getMapById(QUuid(mapItem->data(DMHelper::TreeItemData_ID).toString()));
        }
        else
        {
            map = campaign->getSettingById(QUuid(mapItem->data(DMHelper::TreeItemData_ID).toString()));
        }
        */

        if(!map)
            return;

        QString filename = QFileDialog::getOpenFileName(this, QString("Select Map Image..."));
        if(!filename.isEmpty())
        {
            map->setFileName(filename);
            statusBar()->showMessage(map->getFileName()); // TODO: Can this not automatically happen?
        }
    }
}

void MainWindow::removeCurrentItem()
{
    if((!campaign)||(!treeModel))
        return;

//    QStandardItem* removeItem = treeModel->itemFromIndex(ui->treeView->currentIndex());
//    int type = removeItem->data(DMHelper::TreeItemData_Type).toInt();

//    CampaignObjectBase* removeObject = campaign->getObjectById(QUuid(removeItem->data(DMHelper::TreeItemData_ID).toString()));
    CampaignObjectBase* removeObject = ui->treeView->currentCampaignObject();
    if(!removeObject)
    {
        qDebug() << "[MainWindow] ERROR: cannot remove object because not able to find current campaign object.";
        return;
    }

    CampaignObjectBase* parentObject = dynamic_cast<CampaignObjectBase*>(removeObject->parent());
    if(!parentObject)
    {
        qDebug() << "[MainWindow] ERROR: cannot remove object because not able to find current object's parent. Current object: " << removeObject->getName() << ", ID: " << removeObject->getID();
        return;
    }

    if(QMessageBox::question(this,
                             QString("Confirm Delete"),
                             QString("Are you sure you would like to delete the entry ") + removeObject->getName() + QString("?")) != QMessageBox::Yes)
        return;

    QUuid nextObjectId;
    if(parentObject->getObjectType() != DMHelper::CampaignType_Campaign)
    {
        nextObjectId = parentObject->getID();
    }
    else
    {
        const QList<CampaignObjectBase*> campaignChildren = campaign->getChildObjects();
        int i = 0;
        while((nextObjectId.isNull()) && (i < campaignChildren.count()))
        {
            if((campaignChildren.at(i)) && (campaignChildren.at(i)->getID() != removeObject->getID()))
                nextObjectId = campaignChildren.at(i)->getID();
            ++i;
        }
    }

    if(!nextObjectId.isNull())
        selectItem(nextObjectId);
    else
        ui->stackedWidgetEncounter->setCurrentFrame(DMHelper::CampaignType_Base); //ui->stackedWidgetEncounter->setCurrentIndex(0);

    delete campaign->removeObject(removeObject->getID());
    updateCampaignTree();

    /*
    QUuid parentId;
    if(removeObject->parent())
            parentId = QUuid(removeItem->parent()->data(DMHelper::TreeItemData_ID).toString());
    campaign->removeObject(QUuid(removeItem->data(DMHelper::TreeItemData_ID).toString()));
    if(!parentId.isNull())
    {
        QStandardItem* parentItem = findItem(treeModel->invisibleRootItem(), parentId);
        if(parentItem)
            ui->treeView->setCurrentIndex(parentItem->index());
    }
    */

    /*
    QStandardItem* adventureItem = findParentbyType(removeItem, DMHelper::TreeType_Adventure);
    if(adventureItem)
    {
        Adventure* adventure = campaign->getAdventureById(QUuid(adventureItem->data(DMHelper::TreeItemData_ID).toString()));
        if(!adventure)
            return;

        if(type == DMHelper::TreeType_Adventure)
        {
            if(QMessageBox::question(this,QString("Confirm Delete Adventure"),QString("Are you sure you would like to delete the adventure ") + adventure->getName() + QString("?")) == QMessageBox::Yes)
            {
                delete campaign->removeAdventure(QUuid(removeItem->data(DMHelper::TreeItemData_ID).toString()));
            }
        }
        else if(type == DMHelper::TreeType_Encounter)
        {
            Encounter* encounter = adventure->getEncounterById(QUuid(removeItem->data(DMHelper::TreeItemData_ID).toString()));
            if(QMessageBox::question(this,QString("Confirm Delete Encounter"),QString("Are you sure you would like to delete the encounter ") + encounter->getName() + QString("?")) == QMessageBox::Yes)
            {
                encounter->widgetDeactivated(Hiui->stackedWidgetEncounter->currentWidget());
                delete adventure->removeEncounter(QUuid(removeItem->data(DMHelper::TreeItemData_ID).toString()));
            }
        }
        else if(type == DMHelper::TreeType_Map)
        {
            Map* map = adventure->getMapById(QUuid(removeItem->data(DMHelper::TreeItemData_ID).toString()));
            if(QMessageBox::question(this,QString("Confirm Delete Map"),QString("Are you sure you would like to delete the map ") + map->getName() + QString("?")) == QMessageBox::Yes)
            {
                //TODO: is it right that this isn't a delete?
                adventure->removeMap(QUuid(removeItem->data(DMHelper::TreeItemData_ID).toString()));
            }
        }
    }
    else
    {
        if(type == DMHelper::TreeType_Character)
        {
            Character* character = campaign->getCharacterById(QUuid(removeItem->data(DMHelper::TreeItemData_ID).toString()));
            if(character)
            {
                if(QMessageBox::question(this,QString("Confirm Delete Character"),QString("Are you sure you would like to delete the character ") + character->getName() + QString("?")) == QMessageBox::Yes)
                {
                    delete campaign->removeCharacter(character->getID());
                }
            }
            else
            {
                character = campaign->getNPCById(QUuid(removeItem->data(DMHelper::TreeItemData_ID).toString()));
                if(character)
                {
                    if(QMessageBox::question(this,QString("Confirm Delete NPC"),QString("Are you sure you would like to delete the npc ") + character->getName() + QString("?")) == QMessageBox::Yes)
                    {
                        delete campaign->removeNPC(character->getID());
                    }
                }
            }
        }
        else if(type == DMHelper::TreeType_Map)
        {
            Map* map = campaign->getSettingById(QUuid(removeItem->data(DMHelper::TreeItemData_ID).toString()));
            if(QMessageBox::question(this,QString("Confirm Delete Setting"),QString("Are you sure you would like to delete the setting ") + map->getName() + QString("?")) == QMessageBox::Yes)
            {
                //TODO: is it right that this isn't a delete?
                campaign->removeSetting(QUuid(removeItem->data(DMHelper::TreeItemData_ID).toString()));
            }
        }
    }
    */
}

void MainWindow::editCurrentItem()
{
    if((!campaign)||(!treeModel))
        return;

    QModelIndex index = ui->treeView->currentIndex();
    QStandardItem* editItem = treeModel->itemFromIndex(index);
    int type = editItem->data(DMHelper::TreeItemData_Type).toInt();

    if(editItem->isEditable())
    {
        ui->treeView->edit(index);
    }
    else if(type == DMHelper::TreeType_Map)
    {
        Map* map = dynamic_cast<Map*>(campaign->getObjectById(QUuid(editItem->data(DMHelper::TreeItemData_ID).toString())));
        /*
        QStandardItem* adventureItem = findParentbyType(editItem, DMHelper::TreeType_Adventure);
        if(adventureItem)
        {
            Adventure* adventure = campaign->getAdventureById(QUuid(adventureItem->data(DMHelper::TreeItemData_ID).toString()));
            if(adventure)
            {
                map = adventure->getMapById(QUuid(editItem->data(DMHelper::TreeItemData_ID).toString()));
            }
        }

        if(!map)
        {
            map = campaign->getSettingById(QUuid(editItem->data(DMHelper::TreeItemData_ID).toString()));
        }
        */

        if(map)
        {
            bool ok = false;
            QString mapName = QInputDialog::getText(this, QString("Enter Map Name"), QString("New Map"), QLineEdit::Normal, QString(), &ok);
            if(ok)
            //{
                map->setName(mapName); // TODO: Why doesn't this emit changed? Fixed, should now be ok
                //updateCampaignTree();
            //}
        }
    }
}

void MainWindow::exportCurrentItem()
{
    if((!campaign)||(!treeModel))
        return;

    QModelIndex index = ui->treeView->currentIndex();
    QStandardItem* exportItem = treeModel->itemFromIndex(index);
    if(!exportItem)
        return;

    QString exportFileName = QFileDialog::getSaveFileName(this,QString("Export Object"),QString(),QString("XML files (*.xml)"));
    if(exportFileName.isEmpty())
        return;

    QFileInfo fileInfo(exportFileName);
    QDir targetDir(fileInfo.absoluteDir());

    QUuid exportId(exportItem->data(DMHelper::TreeItemData_ID).toString());

    qDebug() << "[MainWindow] Exporting object with ID " << exportId << " to " << exportFileName;

    CampaignExporter exporter(*campaign, exportId, targetDir);
    if(!exporter.isValid())
    {
        qDebug() << "[MainWindow] Error - invalid export created!";
        return;
    }

    QString exportString = exporter.getExportDocument().toString();
    if(exportString.isEmpty())
    {
        qDebug() << "[MainWindow] Error - export null string found, no export created!";
        return;
    }

    QFile file(exportFileName);
    if( !file.open( QIODevice::WriteOnly ) )
    {
        qDebug() << "[MainWindow] Not able to open export file " << exportFileName;
        return;
    }

    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    ts << exportString;
    file.close();

    qDebug() << "[MainWindow] Export complete";
}

void MainWindow::clearDirty()
{
    dirty = false;
    setWindowModified(dirty);
}

void MainWindow::setDirty()
{
    dirty = true;
    setWindowModified(dirty);
}

void MainWindow::checkForUpdates(bool silentUpdate)
{
    UpdateChecker* checker = new UpdateChecker(*_options, silentUpdate, true, this);
    checker->checkForUpdates();
}

void MainWindow::showPublishWindow(bool visible)
{
    if(visible)
    {
        if(!pubWindow->isVisible())
        {
            pubWindow->show();
        }
        pubWindow->activateWindow();
    }
    else
    {
        pubWindow->hide();
    }
}

void MainWindow::linkActivated(const QUrl & link)
{
    QString path = link.path();
    if(path.startsWith(QString("DMHelper@")))
    {
        QString linkName = path.remove(0, 9);
        if(treeIndexMap.contains(linkName))
        {
            QModelIndex index = treeIndexMap.value(linkName);
            ui->treeView->setCurrentIndex(index);
        }
    }
}

void MainWindow::readBestiary()
{
    qDebug() << "[MainWindow] Requested to read Bestiary.";

    if(!Bestiary::Instance())
    {
        qDebug() << "[MainWindow] Bestiary instance not found, reading stopped";
        return;
    }

    QString bestiaryFileName = _options->getBestiaryFileName();
    qDebug() << "[MainWindow] Bestiary file from options: " << bestiaryFileName;

    if(bestiaryFileName.isEmpty())
    {
        qDebug() << "[MainWindow] ERROR! No known bestiary found, attempting to load default bestiary";
#ifdef Q_OS_MAC
        QDir fileDirPath(QCoreApplication::applicationDirPath());
        fileDirPath.cdUp();
        fileDirPath.cdUp();
        fileDirPath.cdUp();
        bestiaryFileName = fileDirPath.path() + QString("/bestiary/DMHelperBestiary.xml");
#else
        QDir fileDirPath(QCoreApplication::applicationDirPath());
        bestiaryFileName = fileDirPath.path() + QString("/bestiary/DMHelperBestiary.xml");
#endif
    }

    qDebug() << "[MainWindow] Reading bestiary: " << bestiaryFileName;

    QDomDocument doc( "DMHelperBestiaryXML" );
    QFile file( bestiaryFileName );
    if( !file.open( QIODevice::ReadOnly ) )
    {
        qDebug() << "[MainWindow] Reading bestiary file open failed.";
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString errMsg;
    int errRow;
    int errColumn;
    bool contentResult = doc.setContent( in.readAll(), &errMsg, &errRow, &errColumn);

    file.close();

    if( contentResult == false )
    {
        qDebug() << "[MainWindow] Reading bestiary reading XML content.";
        qDebug() << errMsg << errRow << errColumn;
        return;
    }

    QDomElement root = doc.documentElement();
    if( (root.isNull()) || (root.tagName() != "root") )
    {
        qDebug() << "[MainWindow] Bestiary file missing root item";
        return;
    }

    QFileInfo fileInfo(bestiaryFileName);
    Bestiary::Instance()->setDirectory(fileInfo.absoluteDir());
    Bestiary::Instance()->inputXML(root, false);

    if(!_options->getLastMonster().isEmpty() && Bestiary::Instance()->exists(_options->getLastMonster()))
        bestiaryDlg.setMonster(_options->getLastMonster());
    else
        bestiaryDlg.setMonster(Bestiary::Instance()->getFirstMonsterClass());

    qDebug() << "[MainWindow] Bestiary reading complete.";

}

void MainWindow::showEvent(QShowEvent * event)
{
    qDebug() << "[MainWindow] Main window Show event.";
    if(!initialized)
    {
        // Implement any one-time initialization here
        if((_options) && (!_options->doDataSettingsExist()))
        {
            LegalDialog dlg;
            dlg.exec();
            _options->setUpdatesEnabled(dlg.isUpdatesEnabled());
            _options->setStatisticsAccepted(dlg.isStatisticsAccepted());
        }

        if(_options)
        {
            checkForUpdates(true);
        }

        initialized = true;
    }

    //ui->scrollWidget->resizeTabs();

    QMainWindow::showEvent(event);
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    Q_UNUSED(event);

    qDebug() << "[MainWindow] Close event received.";

    if(!closeCampaign())
    {
        event->ignore();
        return;
    }

    writeBestiary();

    _options->setLastMonster(bestiaryDlg.getMonster() ? bestiaryDlg.getMonster()->getName() : "");
    _options->writeSettings();

    qApp->quit();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    //ui->scrollWidget->resizeTabs();
}

void MainWindow::mousePressEvent(QMouseEvent * event)
{
    mouseDownPos = event->pos();
    mouseDown = true;

    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent * event)
{
    mouseDown = false;
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent * event)
{
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Escape:
            emit cancelSelect();
            return;
        default:
            break;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData* data = event->mimeData();
    if( (data->hasUrls()) &&
        (data->urls().count() == 1) &&
        (data->urls().first().isLocalFile()) )
    {
        // TODO: fix
        QImageReader reader(data->urls().first().toLocalFile());
        if(reader.canRead())
        {
            event->acceptProposedAction();
            return;
        }
    }

    event->ignore();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    const QMimeData* data = event->mimeData();
    if( (data->hasUrls()) &&
        (data->urls().count() == 1) &&
        (data->urls().first().isLocalFile()) )
    {
        // TODO: enhance to include non-images
        QImageReader reader(data->urls().first().toLocalFile());
        if(reader.canRead())
        {
            event->acceptProposedAction();
            return;
        }
    }

    event->ignore();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData* data = event->mimeData();
    if( (data->hasUrls()) &&
        (data->urls().count() == 1) &&
        (data->urls().first().isLocalFile()) )
    {
        QString filename = data->urls().first().toLocalFile();
        openFile(filename);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::setupRibbonBar()
{
    _ribbon = new RibbonMain(this);

    _ribbonTabFile = new RibbonTabFile(this);
    _ribbon->enableTab(_ribbonTabFile);
    _ribbonTabCampaign = new RibbonTabCampaign(this);
    _ribbon->enableTab(_ribbonTabCampaign);
    _ribbonTabTools = new RibbonTabBestiary(this);
    _ribbon->enableTab(_ribbonTabTools);
//    _ribbonTabHelp = new RibbonTabHelp(this);
//    _ribbon->enableTab(_ribbonTabHelp);

    _ribbonTabMap = new RibbonTabMap(this);
    _ribbonTabMap->hide();
    _ribbonTabBattle = new RibbonTabBattle(this);
    _ribbonTabBattle->hide();
    _ribbonTabScrolling = new RibbonTabScrolling(this);
    _ribbonTabScrolling->hide();
    _ribbonTabText = new RibbonTabText(this);
    _ribbonTabText->hide();
    _ribbonTabMiniMap = new MapEditFrame(this);
    _ribbonTabMiniMap->hide();

    connect(_ribbon->getPublishRibbon(), SIGNAL(playersWindowClicked(bool)), this, SLOT(showPublishWindow(bool)));

    _ribbon->setCurrentIndex(0);
    setMenuWidget(_ribbon);
}

void MainWindow::deleteCampaign()
{
    if(treeModel)
    {
        // Deselect the current object
        handleTreeItemSelected(QModelIndex(), ui->treeView->currentIndex());
        // Clear the tree model
        treeModel->clear();
    }

    if(campaign)
    {
        Campaign* oldCampaign = campaign;
        campaign = nullptr;
        emit campaignLoaded(campaign);

        // Clear the campaign itself
        delete oldCampaign;
    }

    // Ensure the file name is removed
    campaignFileName.clear();
}

void MainWindow::enableCampaignMenu()
{
    //ui->menu_Campaign->setEnabled(campaign != nullptr);
    _ribbonTabCampaign->setCampaignEnabled(campaign != nullptr);

    /*
    if(campaign)
    {
        bool active = false;
        if(treeModel)
        {
            QStandardItem* selectedItem = treeModel->itemFromIndex(ui->treeView->currentIndex());
            if(selectedItem)
            {
                int type = selectedItem->data(DMHelper::TreeItemData_Type).toInt();
                if( ( type == DMHelper::TreeType_Adventure ) ||
                    ( type == DMHelper::TreeType_Encounter_Title ) ||
                    ( type == DMHelper::TreeType_Encounter ) ||
                    ( type == DMHelper::TreeType_Map_Title ) ||
                    ( type == DMHelper::TreeType_Map ) )
                    active = true;
            }
        }

        ui->menuNew_Encounter->setEnabled(active);
        ui->actionNew_Map->setEnabled(active);
    }
    */
}


Encounter* MainWindow::notesFromIndex(const QModelIndex & index)
{
    // TODO: Remove completely
    return nullptr;
    /*
    if( (!treeModel) || (!index.isValid()) )
        return nullptr;

    QStandardItem* item = treeModel->itemFromIndex(index);
    if(!item)
        return nullptr;

    int type = item->data(DMHelper::TreeItemData_Type).toInt();
    if(type != DMHelper::TreeType_Notes)
        return nullptr;

    return campaign->getNotes();
    */
}

Character* MainWindow::characterFromIndex(const QModelIndex & index)
{
    if( (!treeModel) || (!index.isValid()) )
        return nullptr;

    QStandardItem* item = treeModel->itemFromIndex(index);
    if(!item)
        return nullptr;

    int type = item->data(DMHelper::TreeItemData_Type).toInt();
    if(type != DMHelper::TreeType_Character)
        return nullptr;

    Character* character = campaign->getCharacterById(QUuid(item->data(DMHelper::TreeItemData_ID).toString()));
    if(!character)
        character = campaign->getNPCById(QUuid(item->data(DMHelper::TreeItemData_ID).toString()));

    return character;
}

Adventure* MainWindow::adventureFromIndex(const QModelIndex & index)
{
    // TODO: remove completely
    return nullptr;
    /*
    // TODO: much repetition in these FromIndex functions - must be able to be simplified and generalized
    if( (!treeModel) || (!index.isValid()) )
        return nullptr;

    QStandardItem* item = treeModel->itemFromIndex(index);
    while((item) && (item->data(DMHelper::TreeItemData_Type).toInt() != DMHelper::TreeType_Adventure))
        item = item->parent();

    if(!item)
        return nullptr;
    else
        return campaign->getAdventureById(QUuid(item->data(DMHelper::TreeItemData_ID).toString()));
        */
}

Encounter* MainWindow::encounterFromIndex(const QModelIndex & index)
{
    // TODO: remove completely
    return nullptr;
    /*
    if( (!treeModel) || (!index.isValid()) )
        return nullptr;

    QStandardItem* item = treeModel->itemFromIndex(index);
    if(!item)
        return nullptr;

    int type = item->data(DMHelper::TreeItemData_Type).toInt();
    if(type != DMHelper::TreeType_Encounter)
        return nullptr;

    Adventure* adventure = adventureFromIndex(index);
    if(!adventure)
        return nullptr;

    return adventure->getEncounterById(QUuid(item->data(DMHelper::TreeItemData_ID).toString()));
    */
}

Map* MainWindow::mapFromIndex(const QModelIndex & index)
{
    if((!campaign) || (!treeModel) || (!index.isValid()))
        return nullptr;

    QStandardItem* item = treeModel->itemFromIndex(index);
    if(!item)
        return nullptr;

    return dynamic_cast<Map*>(campaign->getObjectById(QUuid(item->data(DMHelper::TreeItemData_ID).toString())));

    /*
    int type = item->data(DMHelper::TreeItemData_Type).toInt();
    if(type != DMHelper::TreeType_Map)
        return nullptr;

    QStandardItem* parent = item->parent();
    if(!parent)
        return nullptr;

    QStandardItem* adventureParent = parent->parent();
    if(!adventureParent)
        return nullptr;

    int parentType = adventureParent->data(DMHelper::TreeItemData_Type).toInt();
    if(parentType == DMHelper::TreeType_Adventure)
    {
        Adventure* adventure = campaign->getAdventureById(QUuid(adventureParent->data(DMHelper::TreeItemData_ID).toString()));
        if(!adventure)
            return nullptr;

        return adventure->getMapById(QUuid(item->data(DMHelper::TreeItemData_ID).toString()));
    }
    else if(parentType == DMHelper::TreeType_World)
    {
        return campaign->getSettingById(QUuid(item->data(DMHelper::TreeItemData_ID).toString()));
    }
    else
    {
        return nullptr;
    }
    */
}

bool MainWindow::selectItem(QUuid itemId)
{
    if((treeModel) && (!itemId.isNull()))
    {
        QModelIndex index = treeModel->getObject(itemId);
        if(index.isValid())
        {
            ui->treeView->setCurrentIndex(index);
            activateWindow();
            return true;
        }
    }

    return false;
}

bool MainWindow::selectItem(int itemType, QUuid itemId)
{
    Q_UNUSED(itemType);
    return selectItem(itemId);
    /*
    if((campaign)&&(treeModel))
    {
        QStandardItem* item = findItem(treeModel->invisibleRootItem(), itemType, itemId);
        if(item != nullptr)
        {
            ui->treeView->setCurrentIndex(item->index());
            return true;
        }
    }

    return false;
    */
}

bool MainWindow::selectItem(int itemType, QUuid itemId, QUuid adventureId)
{
    Q_UNUSED(itemType);
    Q_UNUSED(adventureId);

    return selectItem(itemId);
    /*
    if((campaign)&&(treeModel))
    {
        QStandardItem* adventureItem = findItem(treeModel->invisibleRootItem(), DMHelper::TreeType_Adventure, adventureId);
        if(adventureItem)
        {
            QStandardItem* item = findItem(adventureItem, itemType, itemId);
            if(item)
            {
                ui->treeView->setCurrentIndex(item->index());
                return true;
            }
        }
    }

    return false;
    */
}

QStandardItem* MainWindow::findItem(QStandardItem* parent, int itemType, QUuid itemId)
{
    if(!parent)
        return nullptr;

    for(int i = 0; i < parent->rowCount(); ++i)
    {
        QStandardItem* child = parent->child(i);
        if( ( child->data(DMHelper::TreeItemData_Type).toInt() == itemType ) && ( QUuid(child->data(DMHelper::TreeItemData_ID).toString()) == itemId ) )
        {
            return child;
        }

        QStandardItem* childResult = findItem(child, itemType, itemId);
        if(childResult != nullptr)
            return childResult;
    }

    return nullptr;
}

QStandardItem* MainWindow::findItem(QStandardItem* parent, QUuid itemId)
{
    if(!parent)
        return nullptr;

    for(int i = 0; i < parent->rowCount(); ++i)
    {
        QStandardItem* child = parent->child(i);
        if(QUuid(child->data(DMHelper::TreeItemData_ID).toString()) == itemId)
        {
            return child;
        }

        QStandardItem* childResult = findItem(child, itemId);
        if(childResult != nullptr)
            return childResult;
    }

    return nullptr;
}

QStandardItem* MainWindow::findParentbyType(QStandardItem* child, int parentType)
{
    if(!child)
        return nullptr;

    if( child->data(DMHelper::TreeItemData_Type).toInt() == parentType )
        return child;

    return findParentbyType(child->parent(), parentType);
}

void MainWindow::setIndexExpanded(bool expanded, const QModelIndex& index)
{
    if(expanded)
    {
        ui->treeView->expand(index);
    }
    else
    {
        ui->treeView->collapse(index);
    }

}

void MainWindow::writeBestiary()
{
    qDebug() << "[MainWindow] Bestiary now to be written to file";

    if(!Bestiary::Instance())
    {
        qDebug() << "[MainWindow] Bestiary instance not found, no file written.";
        return;
    }

    if(Bestiary::Instance()->count() <= 0)
    {
        qDebug() << "[MainWindow] Bestiary is empty, no file will be written";
        return;
    }

    QString bestiaryFileName = _options->getBestiaryFileName();
    qDebug() << "[MainWindow] Writing Bestiary to " << bestiaryFileName;

    if(bestiaryFileName.isEmpty())
    {
        bestiaryFileName = QFileDialog::getSaveFileName(this,QString("Save Bestiary"),QString(),QString("XML files (*.xml)"));
        if(bestiaryFileName.isEmpty())
            return;

        _options->setBestiaryFileName(bestiaryFileName);
    }

    QDomDocument doc( "DMHelperBestiaryXML" );

    QDomElement root = doc.createElement( "root" );
    doc.appendChild( root );

    QFileInfo fileInfo(bestiaryFileName);
    QDir targetDirectory(fileInfo.absoluteDir());
    if( Bestiary::Instance()->outputXML(doc, root, targetDirectory, false) <= 0)
    {
        qDebug() << "[MainWindow] Bestiary output did not find any monsters. Aborting writing to file";
        return;
    }

    QString xmlString = doc.toString();

    QFile file(bestiaryFileName);
    if( !file.open( QIODevice::WriteOnly ) )
    {
        qDebug() << "[MainWindow] Unable to open Bestiary file for writing: " << bestiaryFileName;
        qDebug() << "       Error " << file.error() << ": " << file.errorString();
        QFileInfo info(file);
        qDebug() << "       Full filename: " << info.absoluteFilePath();
        bestiaryFileName.clear();
        return;
    }

    QTextStream ts( &file );
    ts.setCodec("UTF-8");
    ts << xmlString;

    file.close();

    qDebug() << "[MainWindow] Bestiary file writing complete: " << bestiaryFileName;
}

CampaignObjectBase* MainWindow::newEncounter(int encounterType, const QString& dialogTitle, const QString& dialogText)
{
    if((!campaign)||(!treeModel))
        return nullptr;

    CampaignObjectBase* currentObject = ui->treeView->currentCampaignObject();
    if(!currentObject)
        currentObject = campaign;

    bool ok;
    QString encounterName = QInputDialog::getText(this, QString("Enter Encounter Name"), QString("New Encounter"), QLineEdit::Normal, QString(), &ok);
    if(!ok)
        return nullptr;

    //Encounter* encounter = EncounterFactory::createEncounter(encounterType, encounterName, adventure);
    CampaignObjectBase* encounter = EncounterFactory().createObject(encounterType, -1, encounterName, false);
    if(!encounter)
        return nullptr;

    //adventure->addEncounter(encounter);
    currentObject->setExpanded(true);
    currentObject->addObject(encounter);
//    selectItem(DMHelper::TreeType_Encounter, encounter->getID(), id);

    updateCampaignTree();

    selectItem(encounter->getID());

    return encounter;

    /*
    QStandardItem* adventureItem = findParentbyType(treeModel->itemFromIndex(ui->treeView->currentIndex()), DMHelper::TreeType_Adventure);
    if(!adventureItem)
        return;

    QUuid id = QUuid(adventureItem->data(DMHelper::TreeItemData_ID).toString());
    bool ok;
    QString encounterName = QInputDialog::getText(this, QString("Enter Encounter Name"),QString("New Encounter"),QLineEdit::Normal,QString(),&ok);
    if(ok)
    {
        Adventure* adventure = campaign->getAdventureById(id);
        if(adventure)
        {
            Encounter* encounter = EncounterFactory::createEncounter(encounterType, encounterName, adventure);
            if(encounter)
            {
                adventure->addEncounter(encounter);
                selectItem(DMHelper::TreeType_Encounter, encounter->getID(), id);
            }
        }
    }
    */
}

void MainWindow::openFile(const QString& filename)
{
    if(!closeCampaign())
        return;

    qDebug() << "[MainWindow] Loading Campaign: " << filename;

    QDomDocument doc("DMHelperXML");
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, QString("Campaign file open failed"),
                              QString("Unable to open the campaign file: ") + filename);
        qDebug() << "[MainWindow] Loading Failed: Unable to open campaign file";
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString contentError;
    int contentErrorLine = 0;
    int contentErrorColumn = 0;
    bool contentResult = doc.setContent(in.readAll(), &contentError, &contentErrorLine, &contentErrorColumn);

    file.close();

    if(contentResult == false)
    {
        QMessageBox::critical(this, QString("Campaign file open failed"),
                              QString("Error reading the campaign file: (line ") + QString::number(contentErrorLine) + QString(", column ") + QString::number(contentErrorColumn) + QString("): ") + contentError);
        qDebug() << "[MainWindow] Loading Failed: Error reading XML (line " << contentErrorLine << ", column " << contentErrorColumn << "): " << contentError;
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "root"))
    {
        QMessageBox::critical(this, QString("Campaign file open failed"),
                              QString("Unable to find the root entry in the campaign file: ") + filename);
        qDebug() << "[MainWindow] Loading Failed: Error reading XML - unable to find root entry";
        return;
    }

    QDomElement campaignElement = root.firstChildElement(QString("campaign"));
    if(campaignElement.isNull())
    {
        QMessageBox::critical(this, QString("Campaign file open failed"),
                              QString("Unable to find the campaign entry in the campaign file: ") + filename);
        qDebug() << "[MainWindow] Loading Failed: Error reading XML - unable to find campaign entry";
        return;
    }

    campaignFileName = filename;
    QFileInfo fileInfo(campaignFileName);
    QDir::setCurrent(fileInfo.absolutePath());
    campaign = new Campaign();
    campaign->inputXML(campaignElement, false);
    campaign->postProcessXML(campaignElement, false);
    if(!campaign->isValid())
    {
        QMessageBox::StandardButton result = QMessageBox::critical(this,
                                                                   QString("Invalid Campaign"),
                                                                   QString("The loaded campaign has an invalid structure: there is a high risk of data loss and/or program malfunction! Would you like to continue?"),
                                                                   QMessageBox::Yes | QMessageBox::No );
        if(result == QMessageBox::No)
        {
            QMessageBox::information(this,
                                     QString("Invalid Campaign"),
                                     QString("The campaign has not been opened."));
            qDebug() << "[MainWindow] Invalid campaign discarded";
            delete campaign;
            campaign = nullptr;
            return;
        }
    }

    selectItem(DMHelper::TreeType_Campaign, QUuid());
    emit campaignLoaded(campaign);

    if(_options->getMRUHandler())
        _options->getMRUHandler()->addMRUFile(filename);
}

void MainWindow::handleCampaignLoaded(Campaign* campaign)
{
    qDebug() << "[MainWindow] Campaign Loaded: " << campaignFileName;

    updateCampaignTree();
    updateMapFiles();
    updateClock();

    treeModel->setCampaign(campaign);

    if(campaign)
    {
        QModelIndex firstIndex = treeModel->index(0,0);
        if(firstIndex.isValid())
            ui->treeView->setCurrentIndex(firstIndex); // Activate the first entry in the tree
        else
            ui->stackedWidgetEncounter->setCurrentFrame(DMHelper::CampaignType_Base); // ui->stackedWidgetEncounter->setCurrentIndex(0);
        connect(campaign,SIGNAL(dirty()),this,SLOT(setDirty()));
        //connect(campaign,SIGNAL(changed()),this,SLOT(updateCampaignTree()));
        setWindowTitle(QString("DM Helper - ") + campaign->getName() + QString("[*]"));
        _ribbon->setCurrentIndex(1); // Shift to the Campaign tab
    }
    else
    {
        setWindowTitle(QString("DM Helper [*]"));
        ui->stackedWidgetEncounter->setEnabled(true);
        // Deactivate the currently selected object
        deactivateObject();
        activateWidget(DMHelper::CampaignType_WelcomeScreen);// ui->stackedWidgetEncounter->setCurrentIndex(DMHelper::EncounterType_WelcomeScreen);
        setRibbonToType(DMHelper::CampaignType_WelcomeScreen);
        _ribbon->setCurrentIndex(0); // Shift to the File tab
        _ribbonTabCampaign->setAddPCButton(false);
    }

    enableCampaignMenu();
}

void MainWindow::updateCampaignTree()
{
    qDebug() << "[MainWindow] Updating Campaign Tree";
    //enableCampaignMenu();
    if(treeModel)
        treeModel->refresh();

    /*

    if(treeModel)
    {
        // Clear the tree model
        treeModel->clear();
        treeIndexMap.clear();
    }

    if(campaign)
    {
        QStandardItem* campaignItem = new QStandardItem(campaign->getName());
        campaignItem->setEditable(true);
        campaignItem->setData(QVariant(DMHelper::TreeType_Campaign),DMHelper::TreeItemData_Type);
        campaignItem->setData(QVariant(QString()),DMHelper::TreeItemData_ID);
        treeModel->appendRow(campaignItem);
        ui->treeView->expand(campaignItem->index());

        QStandardItem* generalNotesItem = new QStandardItem(QString("Notes"));
        generalNotesItem->setData(QVariant(DMHelper::TreeType_Notes),DMHelper::TreeItemData_Type);
        generalNotesItem->setData(QVariant(QUuid().toString()),DMHelper::TreeItemData_ID);
        campaignItem->appendRow(generalNotesItem);

        QStandardItem* partyTitleItem = new QStandardItem(QString("Party"));
        partyTitleItem->setData(QVariant(DMHelper::TreeType_Party_Title),DMHelper::TreeItemData_Type);
        partyTitleItem->setData(QVariant(QUuid().toString()),DMHelper::TreeItemData_ID);
        campaignItem->appendRow(partyTitleItem);
        setIndexExpanded(campaign->getPartyExpanded(), partyTitleItem->index());

        QStandardItem* adventuresTitleItem = new QStandardItem(QString("Adventures"));
        adventuresTitleItem->setData(QVariant(DMHelper::TreeType_Adventure_Title),DMHelper::TreeItemData_Type);
        adventuresTitleItem->setData(QVariant(QUuid().toString()),DMHelper::TreeItemData_ID);
        campaignItem->appendRow(adventuresTitleItem);
        setIndexExpanded(campaign->getAdventuresExpanded(), adventuresTitleItem->index());

        QStandardItem* worldTitleItem = new QStandardItem(QString("World"));
        worldTitleItem->setData(QVariant(DMHelper::TreeType_World),DMHelper::TreeItemData_Type);
        worldTitleItem->setData(QVariant(QUuid().toString()),DMHelper::TreeItemData_ID);
        campaignItem->appendRow(worldTitleItem);
        setIndexExpanded(campaign->getWorldExpanded(), worldTitleItem->index());

        QStandardItem* audioTitleItem = new QStandardItem(QString("Audio Tracks"));
        audioTitleItem->setData(QVariant(DMHelper::TreeType_AudioTrack),DMHelper::TreeItemData_Type);
        audioTitleItem->setData(QVariant(QUuid().toString()),DMHelper::TreeItemData_ID);
        campaignItem->appendRow(audioTitleItem);

        // Set up the party
        for(int c = 0; c < campaign->getCharacterCount(); ++c)
        {
            Character* character = campaign->getCharacterByIndex(c);
            if(character)
            {
                QStandardItem* characterItem = new QStandardItem(character->getName());
                characterItem->setData(QVariant(DMHelper::TreeType_Character),DMHelper::TreeItemData_Type);
                characterItem->setData(QVariant(character->getID().toString()),DMHelper::TreeItemData_ID);
                characterItem->setEditable(false);
                characterItem->setCheckable(true);
                characterItem->setCheckState(character->getActive() ? Qt::Checked : Qt::Unchecked);
                partyTitleItem->appendRow(characterItem);
                treeIndexMap.insert(characterItem->text(),characterItem->index());
            }
        }

        // Set up the adventures
        for(int a = 0; a < campaign->getAdventureCount(); ++a)
        {
            Adventure* adventure = campaign->getAdventureByIndex(a);
            if(adventure)
            {
                QStandardItem* adventureItem = new QStandardItem(adventure->getName());
                adventureItem->setData(QVariant(DMHelper::TreeType_Adventure),DMHelper::TreeItemData_Type);
                adventureItem->setData(QVariant(adventure->getID().toString()),DMHelper::TreeItemData_ID);
                adventureItem->setEditable(true);
                adventuresTitleItem->appendRow(adventureItem);
                treeIndexMap.insert(adventureItem->text(),adventureItem->index());

                QStandardItem* encountersTitleItem = new QStandardItem(QString("Encounters"));
                encountersTitleItem->setEditable(false);
                encountersTitleItem->setData(QVariant(DMHelper::TreeType_Encounter_Title),DMHelper::TreeItemData_Type);
                encountersTitleItem->setData(QVariant(adventure->getID().toString()),DMHelper::TreeItemData_ID);
                adventureItem->appendRow(encountersTitleItem);
                treeIndexMap.insert(encountersTitleItem->text(),encountersTitleItem->index());

                for( int e = 0; e < adventure->getEncounterCount(); ++e )
                {
                    Encounter* encounter = adventure->getEncounterByIndex(e);
                    if(encounter)
                    {
                        QString encounterName = encounter->getName();
                        if(encounter->hasData())
                            encounterName += " (*)";
                        QStandardItem* encounterItem = new QStandardItem(encounterName);
                        encounterItem->setData(QVariant(DMHelper::TreeType_Encounter),DMHelper::TreeItemData_Type);
                        encounterItem->setData(QVariant(encounter->getID().toString()),DMHelper::TreeItemData_ID);
                        encounterItem->setEditable(true);
                        encountersTitleItem->appendRow(encounterItem);
                        treeIndexMap.insert(encounterItem->text(),encounterItem->index());
                    }
                }

                QStandardItem* mapsTitleItem = new QStandardItem(QString("Maps"));
                mapsTitleItem->setEditable(false);
                mapsTitleItem->setData(QVariant(DMHelper::TreeType_Map_Title),DMHelper::TreeItemData_Type);
                mapsTitleItem->setData(QVariant(adventure->getID().toString()),DMHelper::TreeItemData_ID);
                adventureItem->appendRow(mapsTitleItem);

                for( int m = 0; m < adventure->getMapCount(); ++m )
                {
                    Map* map = adventure->getMapByIndex(m);
                    if(map)
                    {
                        QStandardItem* mapItem = new QStandardItem(map->getName());
                        mapItem->setData(QVariant(DMHelper::TreeType_Map),DMHelper::TreeItemData_Type);
                        mapItem->setData(QVariant(map->getID().toString()),DMHelper::TreeItemData_ID);
                        mapItem->setEditable(false);
                        mapsTitleItem->appendRow(mapItem);
                        treeIndexMap.insert(mapItem->text(),mapItem->index());
                    }
                }

                setIndexExpanded(adventure->getExpanded(), adventureItem->index());
                setIndexExpanded(adventure->getExpanded(), encountersTitleItem->index());
                setIndexExpanded(adventure->getExpanded(), mapsTitleItem->index());
            }
        }

        // Set up the world
        QStandardItem* settingTitleItem = new QStandardItem(QString("Settings"));
        settingTitleItem->setData(QVariant(DMHelper::TreeType_Settings_Title),DMHelper::TreeItemData_Type);
        settingTitleItem->setData(QVariant(QUuid().toString()),DMHelper::TreeItemData_ID);
        worldTitleItem->appendRow(settingTitleItem);
        setIndexExpanded(campaign->getWorldSettingsExpanded(), settingTitleItem->index());

        for( int m = 0; m < campaign->getSettingCount(); ++m )
        {
            Map* map = campaign->getSettingByIndex(m);
            if(map)
            {
                QStandardItem* mapItem = new QStandardItem(map->getName());
                mapItem->setData(QVariant(DMHelper::TreeType_Map),DMHelper::TreeItemData_Type);
                mapItem->setData(QVariant(map->getID().toString()),DMHelper::TreeItemData_ID);
                mapItem->setEditable(false);
                settingTitleItem->appendRow(mapItem);
                treeIndexMap.insert(mapItem->text(),mapItem->index());
            }
        }

        QStandardItem* peopleTitleItem = new QStandardItem(QString("People"));
        peopleTitleItem->setData(QVariant(DMHelper::TreeType_People_Title),DMHelper::TreeItemData_Type);
        peopleTitleItem->setData(QVariant(QUuid().toString()),DMHelper::TreeItemData_ID);
        worldTitleItem->appendRow(peopleTitleItem);
        setIndexExpanded(campaign->getWorldNPCsExpanded(), peopleTitleItem->index());

        for(int c = 0; c < campaign->getNPCCount(); ++c)
        {
            Character* character = campaign->getNPCByIndex(c);
            if(character)
            {
                QStandardItem* characterItem = new QStandardItem(character->getName());
                characterItem->setData(QVariant(DMHelper::TreeType_Character),DMHelper::TreeItemData_Type);
                characterItem->setData(QVariant(character->getID().toString()),DMHelper::TreeItemData_ID);
                characterItem->setEditable(false);
                peopleTitleItem->appendRow(characterItem);
                treeIndexMap.insert(characterItem->text(),characterItem->index());
            }
        }
    }

    encounterTextEdit->setKeys(treeIndexMap.uniqueKeys());
    */
}

void MainWindow::updateMapFiles()
{
    if(!campaign)
        return;

    QFileInfo fileInfo(campaignFileName);
    QDir sourceDir(fileInfo.absoluteDir());

    QList<Map*> mapList = campaign->findChildren<Map*>();
    for(Map* map : mapList)
    {
        if(map)
            map->setFileName(QDir::cleanPath(sourceDir.absoluteFilePath(map->getFileName())));
    }

    /*
    for(int i = 0; i < campaign->getAdventureCount(); ++i)
    {
        Adventure* adventure = campaign->getAdventureByIndex(i);
        if(adventure)
        {
            for( int m = 0; m < adventure->getMapCount(); ++m )
            {
                Map* map = adventure->getMapByIndex(m);
                if(map)
                {
                    map->setFileName(QDir::cleanPath(sourceDir.absoluteFilePath(map->getFileName())));
                }
            }
        }
    }
    */
}

void MainWindow::updateClock()
{
    if(campaign)
    {
        connect(timeAndDateFrame,SIGNAL(dateChanged(BasicDate)),campaign,SLOT(setDate(BasicDate)));
        connect(campaign,SIGNAL(dateChanged(BasicDate)),timeAndDateFrame,SLOT(setDate(BasicDate)));
        connect(timeAndDateFrame,SIGNAL(timeChanged(QTime)),campaign,SLOT(setTime(QTime)));
        connect(campaign,SIGNAL(timeChanged(QTime)),timeAndDateFrame,SLOT(setTime(QTime)));

        timeAndDateFrame->setDate(campaign->getDate());
        timeAndDateFrame->setTime(campaign->getTime());
    }
    else
    {
        timeAndDateFrame->setDate(BasicDate(1,1,0));
        timeAndDateFrame->setTime(QTime(0,0));
    }
}

void MainWindow::handleCustomContextMenu(const QPoint& point)
{
    // TODO: PROPERLY!
    if(!treeModel)
        return;

    QModelIndex index = ui->treeView->indexAt(point);
    if(!index.isValid())
        return;

    //QStandardItem* item = treeModel->itemFromIndex(index);
    CampaignTreeItem* campaignItem = treeModel->campaignItemFromIndex(index);
    if(!campaignItem)
        return;

    CampaignObjectBase* campaignObject = campaignItem->getCampaignItemObject();
    if(!campaignObject)
        return;

    QMenu* contextMenu = new QMenu(ui->treeView);

    // New text entry
    QAction* addTextEntry = new QAction(QIcon(":/img/data/icon_newtextencounter.png"), QString("New Text Entry"), contextMenu);
    connect(addTextEntry, SIGNAL(triggered()), this, SLOT(newTextEncounter()));
    contextMenu->addAction(addTextEntry);

    contextMenu->addSeparator();

    // New party
    QAction* addParty = new QAction(QIcon(":/img/data/icon_newparty.png"), QString("New Party"), contextMenu);
    connect(addParty, SIGNAL(triggered()), this, SLOT(newParty()));
    contextMenu->addAction(addParty);

    // Add a new character
    QAction* addNewCharacter = nullptr;
    if((campaignObject->getObjectType() == DMHelper::CampaignType_Party) || (campaignObject->getParentByType(DMHelper::CampaignType_Party) != nullptr))
        addNewCharacter = new QAction(QIcon(":/img/data/icon_newcharacter.png"), QString("New PC"), contextMenu);
    else
        addNewCharacter = new QAction(QIcon(":/img/data/icon_newnpc.png"), QString("New NPC"), contextMenu);
    connect(addNewCharacter, SIGNAL(triggered()), this, SLOT(newCharacter()));
    contextMenu->addAction(addNewCharacter);

    contextMenu->addSeparator();

    // New map
    QAction* addMap = new QAction(QIcon(":/img/data/icon_newmap.png"), QString("New Map"), contextMenu);
    connect(addMap, SIGNAL(triggered()), this, SLOT(newMap()));
    contextMenu->addAction(addMap);

    // New battle
    QAction* addBattle = new QAction(QIcon(":/img/data/icon_newbattle.png"), QString("New Battle"), contextMenu);
    connect(addBattle, SIGNAL(triggered()), this, SLOT(newBattle()));
    contextMenu->addAction(addBattle);

    // New scroll text
    QAction* addScrollingText = new QAction(QIcon(":/img/data/icon_newscrollingtext.png"), QString("New Scrolling Text"), contextMenu);
    connect(addScrollingText, SIGNAL(triggered()), this, SLOT(newScrollingTextEncounter()));
    contextMenu->addAction(addScrollingText);

    // Remove item
    QAction* removeItem = new QAction(QIcon(":/img/data/icon_remove.png"), QString("Remove Item"), contextMenu);
    connect(removeItem, SIGNAL(triggered()), this, SLOT(removeCurrentItem()));
    contextMenu->addAction(removeItem);

    contextMenu->addSeparator();

    /*
    // TODO: do we want a separate "Edit Map File" option or is the way to just remove the map and add another one?
    if(type == DMHelper::TreeType_Map)
    {
        QAction* editMapItem = new QAction(QString("Edit Map File"), contextMenu);
        connect(editMapItem,SIGNAL(triggered()),this,SLOT(editCurrentMap()));
        contextMenu->addAction(editMapItem);
    }
    */

    if(campaignItem->isEditable())
    {
        QAction* editItem = new QAction(QIcon(":/img/data/icon_edit.png"), QString("Edit Item"), contextMenu);
        connect(editItem,SIGNAL(triggered()),this,SLOT(editCurrentItem()));
        contextMenu->addAction(editItem);

        contextMenu->addSeparator();
    }

    QAction* exportItem = new QAction(QIcon(":/img/data/icon_exportitem.png"), QString("Export Item..."));
    connect(exportItem, SIGNAL(triggered()), this, SLOT(exportCurrentItem()));
    contextMenu->addAction(exportItem);

    if(contextMenu->actions().count() > 0)
        contextMenu->exec(ui->treeView->mapToGlobal(point));

    delete contextMenu;
}

void MainWindow::handleTreeItemChanged(QStandardItem * item)
{
    if((!item) || (!campaign))
        return;

    qDebug() << "[MainWindow] Tree Item Changed: " << item;

    if(item->data(DMHelper::TreeItemData_Type).toInt() == DMHelper::TreeType_Character)
    {
        emit characterChanged(QUuid(item->data(DMHelper::TreeItemData_ID).toString()));
    }

    /*
    campaign->beginBatchChanges();

    int type = item->data(DMHelper::TreeItemData_Type).toInt();
    QUuid id = QUuid(item->data(DMHelper::TreeItemData_ID).toString());
    if(type == DMHelper::TreeType_Adventure)
    {
        Adventure* adventure = campaign->getAdventureById(id);
        if(adventure)
        {
            adventure->setName(item->text());
        }
    }
    else if(type == DMHelper::TreeType_Encounter)
    {
        Encounter* encounter = dynamic_cast<Encounter*>(campaign->getObjectbyId(id));
        if(encounter)
        {
            encounter->setName(item->text());

            Adventure* oldAdventure = encounter->getAdventure();
            if((item->parent()) && (item->parent()->parent()) && (item->parent()->parent()->data(DMHelper::TreeItemData_Type).toInt() == DMHelper::TreeType_Adventure))
            {
                QUuid newAdventureID = QUuid(item->parent()->parent()->data(DMHelper::TreeItemData_ID).toString());
                if(oldAdventure->getID() == newAdventureID)
                {
                    oldAdventure->moveEncounterTo(id,item->row());
                }
                else
                {
                    Adventure* newAdventure = campaign->getAdventureById(newAdventureID);
                    if(newAdventure)
                    {
                        oldAdventure->removeEncounter(id);
                        newAdventure->addEncounter(encounter,item->row());
                    }
                }
            }
        }
    }
    else if(type == DMHelper::TreeType_Map)
    {
        Map* map = dynamic_cast<Map*>(campaign->getObjectbyId(id));
        if(map)
        {
            map->setName(item->text());

            Adventure* oldAdventure = map->getAdventure();
            if((item->parent()) && (item->parent()->parent()) && (item->parent()->parent()->data(DMHelper::TreeItemData_Type).toInt() == DMHelper::TreeType_Adventure))
            {
                QUuid newAdventureID = QUuid(item->parent()->parent()->data(DMHelper::TreeItemData_ID).toString());
                if(oldAdventure->getID() == newAdventureID)
                {
                    oldAdventure->moveMapTo(id,item->row());
                }
                else
                {
                    Adventure* newAdventure = campaign->getAdventureById(newAdventureID);
                    if(newAdventure)
                    {
                        oldAdventure->removeMap(id);
                        newAdventure->addMap(map,item->row());
                    }
                }
            }
        }
    }
    else if(type == DMHelper::TreeType_Character)
    {
        Character* character = dynamic_cast<Character*>(campaign->getObjectbyId(id));
        if(character)
        {
            character->setActive(item->checkState() == Qt::Checked);
            //return;

            // Todo: what if the name of the character is changed?
        }
    }

    campaign->endBatchChanges();
    */
}

void MainWindow::handleTreeItemSelected(const QModelIndex & current, const QModelIndex & previous)
{
    // TODO: refactor and abstract (make a deselect and select)
    Q_UNUSED(previous);

    qDebug() << "[MainWindow] Tree Item Selected. Current: " << current << " Previous: " << previous;

    // Deactivate the currently selected object
    deactivateObject();

    // Look for the next object to activate it
    /*
    QStandardItem* item = treeModel->itemFromIndex(current);
    if(!item)
        return;

//    CampaignObjectBase* itemObject = static_cast<CampaignObjectBase*>(item->data(DMHelper::TreeItemData_Object).value<void*>());
    CampaignObjectBase* itemObject = reinterpret_cast<CampaignObjectBase*>(item->data(DMHelper::TreeItemData_Object).value<quint64>());
    if(!itemObject)
        return;
        */

    CampaignTreeItem* item = treeModel->campaignItemFromIndex(current);
    CampaignObjectBase* itemObject = nullptr;

    if(item)
        itemObject = item->getCampaignItemObject();

    if(itemObject)
    {
        activateObject(itemObject);
        _ribbonTabCampaign->setAddPCButton(((itemObject->getObjectType() == DMHelper::CampaignType_Party) || (itemObject->getParentByType(DMHelper::CampaignType_Party) != nullptr)));
    }
    else
    {
        _ribbonTabCampaign->setAddPCButton(false);
    }


    /*
    // If the previous index is an encounter disable the widget
    // TODO: this isn't relevant any more with the frame, can be removed???
    if(ui->stackedWidgetEncounter->isEnabled() == true)
    {
        Encounter* encounter = encounterFromIndex(previous);
        if(!encounter)
        {
            encounter = notesFromIndex(previous);
        }

        if(encounter)
        {
            encounter->widgetDeactivated(ui->stackedWidgetEncounter->currentWidget());
            if(encounter->getType() == DMHelper::EncounterType_Battle)
            {
                _ribbon->disableTab(_ribbonTabMap);
                _ribbon->disableTab(_ribbonTabBattle);
            }
        }

        ui->stackedWidgetEncounter->setEnabled(false);
    }

    // If the previous index is an map...
    Map* map = mapFromIndex(previous);
    if(map)
    {
        ui->stackedWidgetEncounter->setEnabled(false);
        MapFrame* mapFrame = dynamic_cast<MapFrame*>(ui->stackedWidgetEncounter->widget(DMHelper::EncounterType_Map));
        mapFrame->setMap(nullptr);
        map->unregisterWindow(mapFrame);
        delete undoAction; undoAction = nullptr;
        delete redoAction; redoAction = nullptr;
        _ribbonTabCampaign->setUndoEnabled(false);
        _ribbon->disableTab(_ribbonTabMiniMap);
    }

    enableCampaignMenu();
    statusBar()->clearMessage();

    if(!current.isValid())
        return;

    // If the selected index is a note...
    Encounter* encounter;
    encounter = notesFromIndex(current);
    if(encounter)
    {
        ui->stackedWidgetEncounter->setEnabled(true);
        ui->stackedWidgetEncounter->setCurrentIndex(encounter->getType());
        encounter->widgetActivated(ui->stackedWidgetEncounter->currentWidget());
        EncounterTextEdit* textEdit = dynamic_cast<EncounterTextEdit*>(ui->stackedWidgetEncounter->currentWidget());
        if(textEdit)
            connect(campaign,SIGNAL(destroyed(QObject*)),textEdit,SLOT(clear()));

        return;
    }

    // If the selected index is a character...
    Character* character = characterFromIndex(current);
    if(character)
    {
        ui->stackedWidgetEncounter->setEnabled(true);
        ui->stackedWidgetEncounter->setCurrentIndex(DMHelper::EncounterType_Character);
//        QScrollArea* scrollArea = dynamic_cast<QScrollArea*>(ui->stackedWidgetEncounter->currentWidget());
//        if(scrollArea)
//        {
//            CharacterFrame* characterFrame = dynamic_cast<CharacterFrame*>(scrollArea->widget());
            CharacterFrame* characterFrame = dynamic_cast<CharacterFrame*>(ui->stackedWidgetEncounter->currentWidget());
            if(characterFrame)
            {
                characterFrame->setCharacter(character);
                connect(character,SIGNAL(destroyed(QObject*)),characterFrame,SLOT(clear()));
            }
//        }
        return;
    }

    // If the selected index is an encounter...
    encounter = encounterFromIndex(current);
    if(encounter)
    {
        ui->stackedWidgetEncounter->setEnabled(true);
        ui->stackedWidgetEncounter->setCurrentIndex(encounter->getType());
        encounter->widgetActivated(ui->stackedWidgetEncounter->currentWidget());
        if(encounter->getType() == DMHelper::EncounterType_Text)
        {
            EncounterTextEdit* textEdit = dynamic_cast<EncounterTextEdit*>(ui->stackedWidgetEncounter->currentWidget());
            if(textEdit)
                connect(encounter,SIGNAL(destroyed(QObject*)),textEdit,SLOT(clear()));
        }
        else if(encounter->getType() == DMHelper::EncounterType_Battle)
        {
            _ribbon->enableTab(_ribbonTabMap);
            _ribbon->enableTab(_ribbonTabBattle);


            //EncounterBattleEdit* battleEdit = dynamic_cast<EncounterBattleEdit*>(ui->stackedWidgetEncounter->currentWidget());
            //if(battleEdit)
            //    connect(encounter,SIGNAL(destroyed(QObject*)),battleEdit,SLOT(clear()));

            BattleFrame* battleFrame = dynamic_cast<BattleFrame*>(ui->stackedWidgetEncounter->currentWidget());
            if(battleFrame)
                connect(encounter,SIGNAL(destroyed(QObject*)),battleFrame,SLOT(clear()));
        }
        return;
    }

    // If the selected index is an map...
    map = mapFromIndex(current);
    if(map)
    {
        ui->stackedWidgetEncounter->setEnabled(true);
        ui->stackedWidgetEncounter->setCurrentIndex(DMHelper::EncounterType_Map);
        MapFrame* mapFrame = dynamic_cast<MapFrame*>(ui->stackedWidgetEncounter->currentWidget());
        connect(mapFrame,SIGNAL(windowClosed(MapFrame*)),map,SLOT(unregisterWindow(MapFrame*)));
        map->registerWindow(mapFrame);
        statusBar()->showMessage(map->getFileName());
        mapFrame->setMap(map);

        undoAction = mapFrame->getUndoAction(this);
        undoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
        //ui->menu_Edit->addAction(undoAction);
        connect(_ribbonTabCampaign, SIGNAL(undoClicked()), undoAction, SLOT(trigger()));

        redoAction = mapFrame->getRedoAction(this);
        redoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y));
        //ui->menu_Edit->addAction(redoAction);
        connect(_ribbonTabCampaign, SIGNAL(redoClicked()), redoAction, SLOT(trigger()));

        _ribbonTabCampaign->setUndoEnabled(true);
        _ribbon->enableTab(_ribbonTabMiniMap);

        connect(map,SIGNAL(destroyed(QObject*)), mapFrame,SLOT(clear()));
        return;
    }

    // If the selected index is the audio tracks...
    QStandardItem* item = treeModel->itemFromIndex(current);
    if((item) && (item->data(DMHelper::TreeItemData_Type).toInt() == DMHelper::TreeType_AudioTrack))
    {
        ui->stackedWidgetEncounter->setEnabled(true);
        ui->stackedWidgetEncounter->setCurrentIndex(DMHelper::EncounterType_AudioTrack);
        return;
    }
    */
}

void MainWindow::handleTreeItemDoubleClicked(const QModelIndex & index)
{
    if(!index.isValid())
        return;
}

void MainWindow::handleTreeItemExpanded(const QModelIndex & index)
{
    handleTreeStateChanged(index, true);
}

void MainWindow::handleTreeItemCollapsed(const QModelIndex & index)
{
    handleTreeStateChanged(index, false);
}

void MainWindow::handleTreeStateChanged(const QModelIndex & index, bool expanded)
{
    CampaignTreeItem* item = treeModel->campaignItemFromIndex(index);
    if(!item)
        return;

    CampaignObjectBase* object = item->getCampaignItemObject();
    if(!object)
        return;

    object->setExpanded(expanded);
    //int itemType = item->data(DMHelper::TreeItemData_Type).toInt();
    //QUuid itemId = QUuid(item->data(DMHelper::TreeItemData_ID).toString());

    /*
    switch(itemType)
    {
        case DMHelper::TreeType_Party_Title:
            campaign->setPartyExpanded(expanded);
            break;
        case DMHelper::TreeType_Adventure_Title:
            campaign->setAdventuresExpanded(expanded);
            break;
        case DMHelper::TreeType_World:
            campaign->setWorldExpanded(expanded);
            break;
        case DMHelper::TreeType_Settings_Title:
            campaign->setWorldSettingsExpanded(expanded);
            break;
        case DMHelper::TreeType_People_Title:
            campaign->setWorldNPCsExpanded(expanded);
            break;
        case DMHelper::TreeType_Adventure:
            {
                Adventure* adventure = campaign->getAdventureById(itemId);
                adventure->setExpanded(expanded);
            }
            break;
        case DMHelper::TreeType_Encounter_Title:
        case DMHelper::TreeType_Map_Title:
        default:
            break; // do nothing
    }
    */
}

void MainWindow::handleEncounterTextChanged()
{
    //TODO: remove
    if(ui->stackedWidgetEncounter->isEnabled() == false)
        return;

    Encounter* encounter = encounterFromIndex(ui->treeView->currentIndex());
    if(!encounter)
        return;
}

void MainWindow::handleStartNewBattle()
{
    if((!campaign)||(!_battleDlgMgr))
        return;

    // TODO: HANDLE THIS!
    //_battleDlgMgr->startNewBattle(qobject_cast<EncounterBattle*>(encounterFromIndex(ui->treeView->currentIndex())));
}

void MainWindow::handleLoadBattle()
{
    if((!campaign)||(!_battleDlgMgr))
        return;

    // TODO: HANDLE THIS!
    //_battleDlgMgr->loadBattle(qobject_cast<EncounterBattle*>(encounterFromIndex(ui->treeView->currentIndex())));
}

void MainWindow::handleDeleteBattle()
{
    if((!campaign)||(!_battleDlgMgr))
        return;

    // TODO: HANDLE THIS!
    //_battleDlgMgr->deleteBattle(qobject_cast<EncounterBattle*>(encounterFromIndex(ui->treeView->currentIndex())));
}

/*
void MainWindow::handleAnimationStarted(QColor color)
{
    _animationFrameCount = DMHelper::ANIMATION_TIMER_PREVIEW_FRAMES;
    pubWindow->setBackgroundColor(color);
}
*/

void MainWindow::handleAnimationStarted()
{
    if(pubWindow)
        pubWindow->setBackgroundColor();
    _animationFrameCount = DMHelper::ANIMATION_TIMER_PREVIEW_FRAMES;
}

void MainWindow::handleAnimationPreview(QImage img)
{
    if(!previewFrame)
        return;

    if(++_animationFrameCount > DMHelper::ANIMATION_TIMER_PREVIEW_FRAMES)
    {
        previewFrame->setImage(img);
        _animationFrameCount = 0;
    }
}

void MainWindow::openBestiary()
{
    qDebug() << "[MainWindow] Opening Bestiary";
    if(!Bestiary::Instance())
        return;

    if(Bestiary::Instance()->count() == 0)
    {
        qDebug() << "[MainWindow]    ...Bestiary is empty, creating a first monster";
        bestiaryDlg.createNewMonster();
    }
    else
    {
        bestiaryDlg.setFocus();
        bestiaryDlg.show();
        bestiaryDlg.activateWindow();
    }
}

void MainWindow::exportBestiary()
{
    qDebug() << "[MainWindow] Exporting Bestiary...";
    if(!Bestiary::Instance())
        return;

    BestiaryExportDialog dlg;
    dlg.exec();
}

void MainWindow::importBestiary()
{
    qDebug() << "[MainWindow] Importing Bestiary...";

    if(!Bestiary::Instance())
        return;

    QString filename = QFileDialog::getOpenFileName(this,QString("Select exported file for import"));
    if((!filename.isNull()) && (!filename.isEmpty()) && (QFile::exists(filename)))
    {
        qDebug() << "[MainWindow] Importing bestiary: " << filename;

        QDomDocument doc("DMHelperBestiaryXML");
        QFile file(filename);
        if(!file.open(QIODevice::ReadOnly))
        {
            qDebug() << "[MainWindow] Opening bestiary import file failed.";
            return;
        }

        QTextStream in(&file);
        in.setCodec("UTF-8");
        QString errMsg;
        int errRow;
        int errColumn;
        bool contentResult = doc.setContent(in.readAll(), &errMsg, &errRow, &errColumn);

        file.close();

        if(contentResult == false)
        {
            qDebug() << "[MainWindow] Error reading bestiary import XML content.";
            qDebug() << errMsg << errRow << errColumn;
            return;
        }

        QDomElement root = doc.documentElement();
        if((root.isNull()) || (root.tagName() != "root"))
        {
            qDebug() << "[MainWindow] Bestiary import file missing root item";
            return;
        }

        Bestiary::Instance()->inputXML(root, true);
        openBestiary();

        qDebug() << "[MainWindow] Bestiary import complete.";
    }
}

void MainWindow::openAboutDialog()
{
    qDebug() << "[MainWindow] Opening About Box";

    AboutDialog dlg;
    dlg.exec();
}

void MainWindow::openTextPublisher()
{
    /*
    TextPublishDialog* dlg = new TextPublishDialog(this);
    connect(dlg, SIGNAL(publishImage(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    dlg->show();
    dlg->activateWindow();
    */
    TextPublishDialog dlg;
    connect(&dlg, SIGNAL(publishImage(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    dlg.exec();
}

void MainWindow::openTextTranslator()
{
    /*
    TextTranslateDialog* dlg = new TextTranslateDialog(this);
    connect(dlg, SIGNAL(publishImage(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    dlg->show();
    dlg->activateWindow();
    */
    TextTranslateDialog dlg;
    connect(&dlg, SIGNAL(publishImage(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    dlg.exec();
}

void MainWindow::openRandomMarkets()
{
    /*
    RandomMarketDialog* dlg = new RandomMarketDialog(_options->getShopsFileName(), this);
    dlg->show();
    dlg->activateWindow();
    */
    RandomMarketDialog dlg(_options->getShopsFileName());
    dlg.exec();
}

/*
void MainWindow::openPreview()
{

    previewFrame = new PublishFrame(this);
    connect(previewFrame,SIGNAL(arrowVisibleChanged(bool)),pubWindow,SLOT(setArrowVisible(bool)));
    connect(previewFrame,SIGNAL(arrowVisibleChanged(bool)),previewFrame,SLOT(setArrowVisible(bool)));
    connect(previewFrame,SIGNAL(positionChanged(QPointF)),previewFrame,SLOT(setArrowPosition(QPointF)));
    connect(previewFrame,SIGNAL(positionChanged(QPointF)),pubWindow,SLOT(setArrowPosition(QPointF)));
    connect(this,SIGNAL(dispatchPublishImage(QImage,QColor)),previewFrame,SLOT(setImage(QImage)));

    // Add the preview tab
    //previewTab = new ScrollTabWidget(previewFrame, QSizeF(0.9, 0.9), this);
    //previewTab->setToolTip(QString("Preview"));
    //ui->scrollWidget->addTab(previewTab, QIcon(QPixmap(":/img/data/icon_preview.png")));

    QDialog dlg;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(previewFrame);
    dlg.setLayout(layout);
    dlg.exec();
}

void MainWindow::openScreen()
{
    DMScreenTabWidget* dmScreen = new DMScreenTabWidget(_options->getEquipmentFileName(), this);

    QDialog dlg;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(dmScreen);
    dlg.setLayout(layout);
    dlg.exec();
}

void MainWindow::openTables()
{
    CustomTableFrame* customTableFrame = new CustomTableFrame(_options->getTablesDirectory(), this);

    QDialog dlg;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(customTableFrame);
    dlg.setLayout(layout);
    dlg.exec();
}

void MainWindow::openReference()
{
    QuickRefFrame* quickRefFrame = new QuickRefFrame(_options->getQuickReferenceFileName(), this);
    connect(_options, &OptionsContainer::quickReferenceFileNameChanged, quickRefFrame, &QuickRefFrame::readQuickRef);

    QDialog dlg;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(quickRefFrame);
    dlg.setLayout(layout);
    dlg.exec();
}

void MainWindow::openSoundboard()
{
    AudioPlaybackFrame* audioPlaybackFrame = new AudioPlaybackFrame(this);
    audioPlaybackFrame->setVolume(_options->getAudioVolume());
    connect(_audioPlayer, SIGNAL(positionChanged(qint64)), audioPlaybackFrame, SLOT(setPosition(qint64)));
    connect(_audioPlayer, SIGNAL(durationChanged(qint64)), audioPlaybackFrame, SLOT(setDuration(qint64)));
    connect(_audioPlayer, SIGNAL(trackChanged(AudioTrack*)), audioPlaybackFrame, SLOT(trackChanged(AudioTrack*)));
    connect(_audioPlayer, SIGNAL(stateChanged(AudioPlayer::State)), audioPlaybackFrame, SLOT(stateChanged(AudioPlayer::State)));
    connect(audioPlaybackFrame, SIGNAL(play()), _audioPlayer, SLOT(play()));
    connect(audioPlaybackFrame, SIGNAL(pause()), _audioPlayer, SLOT(pause()));
    connect(audioPlaybackFrame, SIGNAL(positionChanged(qint64)), _audioPlayer, SLOT(setPosition(qint64)));
    connect(audioPlaybackFrame, SIGNAL(volumeChanged(int)), _audioPlayer, SLOT(setVolume(int)));
    connect(audioPlaybackFrame, SIGNAL(volumeChanged(int)), _options, SLOT(setAudioVolume(int)));

    QDialog dlg;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(audioPlaybackFrame);
    dlg.setLayout(layout);
    dlg.exec();
}

void MainWindow::openCalendar()
{
    QDialog dlg;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(timeAndDateFrame);
    dlg.setLayout(layout);
    dlg.exec();
}

void MainWindow::openCountdown()
{
    CountdownFrame* countdownFrame = new CountdownFrame(this);

    QDialog dlg;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(countdownFrame);
    dlg.setLayout(layout);
    dlg.exec();
}
*/

QDialog* MainWindow::createDialog(QWidget* contents)
{
    QDialog* resultDlg = new QDialog();
    QVBoxLayout *dlgLayout = new QVBoxLayout;
    dlgLayout->addWidget(contents);
    dlgLayout->setSpacing(3);
    resultDlg->setLayout(dlgLayout);
    return resultDlg;
}

void MainWindow::connectTextToText()
{
    disconnect(_ribbonTabText, SIGNAL(colorChanged(QColor)), _scrollingTextEdit, SLOT(setColor(QColor)));
    disconnect(_ribbonTabText, SIGNAL(fontFamilyChanged(const QString&)), _scrollingTextEdit, SLOT(setFontFamily(const QString&)));
    disconnect(_ribbonTabText, SIGNAL(fontSizeChanged(int)), _scrollingTextEdit, SLOT(setFontSize(int)));
    disconnect(_ribbonTabText, SIGNAL(fontBoldChanged(bool)), _scrollingTextEdit, SLOT(setFontBold(bool)));
    disconnect(_ribbonTabText, SIGNAL(fontItalicsChanged(bool)), _scrollingTextEdit, SLOT(setFontItalics(bool)));
    disconnect(_ribbonTabText, SIGNAL(alignmentChanged(Qt::Alignment)), _scrollingTextEdit, SLOT(setAlignment(Qt::Alignment)));
}

void MainWindow::connectTextToScroll()
{
    connect(_ribbonTabText, SIGNAL(colorChanged(QColor)), _scrollingTextEdit, SLOT(setColor(QColor)));
    connect(_ribbonTabText, SIGNAL(fontFamilyChanged(const QString&)), _scrollingTextEdit, SLOT(setFontFamily(const QString&)));
    connect(_ribbonTabText, SIGNAL(fontSizeChanged(int)), _scrollingTextEdit, SLOT(setFontSize(int)));
    connect(_ribbonTabText, SIGNAL(fontBoldChanged(bool)), _scrollingTextEdit, SLOT(setFontBold(bool)));
    connect(_ribbonTabText, SIGNAL(fontItalicsChanged(bool)), _scrollingTextEdit, SLOT(setFontItalics(bool)));
    connect(_ribbonTabText, SIGNAL(alignmentChanged(Qt::Alignment)), _scrollingTextEdit, SLOT(setAlignment(Qt::Alignment)));
}

void MainWindow::battleModelChanged(BattleDialogModel* model)
{
    if(!_ribbonTabBattle)
        return;

    if(!model)
    {
        disconnect(_ribbonTabBattle);
    }
    else
    {
        _ribbonTabBattle->setShowDead(model->getShowDead());
        _ribbonTabBattle->setShowLiving(model->getShowAlive());
        _ribbonTabBattle->setShowEffects(model->getShowEffects());
        _ribbonTabBattle->setShowMovement(model->getShowMovement());
        _ribbonTabBattle->setLairActions(model->getShowLairActions());
        connect(_ribbonTabBattle, SIGNAL(showLivingClicked(bool)), model, SLOT(setShowAlive(bool)));
        connect(_ribbonTabBattle, SIGNAL(showDeadClicked(bool)), model, SLOT(setShowDead(bool)));
        connect(_ribbonTabBattle, SIGNAL(showEffectsClicked(bool)), model, SLOT(setShowEffects(bool)));
        connect(_ribbonTabBattle, SIGNAL(showMovementClicked(bool)), model, SLOT(setShowMovement(bool)));
        connect(_ribbonTabBattle, SIGNAL(lairActionsClicked(bool)), model, SLOT(setShowLairActions(bool)));

        _ribbonTabMap->setGridOn(model->getGridOn());
        _ribbonTabMap->setGridScale(model->getGridScale());
        _ribbonTabMap->setGridXOffset(model->getGridOffsetX());
        _ribbonTabMap->setGridYOffset(model->getGridOffsetY());
    }
}

void MainWindow::activateObject(CampaignObjectBase* object)
{
    if(!object)
        return;

    //int selectedWidget = getWidgetFromType(object->getObjectType());

    qDebug() << "[MainWindow] Activating stacked widget for type " << object->getObjectType();

    setRibbonToType(object->getObjectType());
    activateWidget(object->getObjectType(), object);
}

void MainWindow::deactivateObject()
{
    if(_ribbon->getPublishRibbon())
        _ribbon->getPublishRibbon()->cancelPublish();

    CampaignObjectFrame* objectFrame = dynamic_cast<CampaignObjectFrame*>(ui->stackedWidgetEncounter->currentWidget());
    if(objectFrame)
    {
        disconnect(_ribbon->getPublishRibbon(), nullptr, objectFrame, nullptr);
        disconnect(objectFrame, nullptr, _ribbon->getPublishRibbon(), nullptr);
        objectFrame->deactivateObject();
    }
}

void MainWindow::activateWidget(int objectType, CampaignObjectBase* object)
{
    CampaignObjectFrame* objectFrame = ui->stackedWidgetEncounter->setCurrentFrame(objectType); //ui->stackedWidgetEncounter->setCurrentIndex(widgetId);
    if(objectFrame)
    {
        connect(_ribbon->getPublishRibbon(), SIGNAL(clicked(bool)), objectFrame, SLOT(publishClicked(bool)));
        connect(_ribbon->getPublishRibbon(), SIGNAL(rotationChanged(int)), objectFrame, SLOT(setRotation(int)));
        connect(_ribbon->getPublishRibbon(), SIGNAL(colorChanged(QColor)), objectFrame, SLOT(setBackgroundColor(QColor)));

        connect(objectFrame, SIGNAL(setPublishEnabled(bool)), _ribbon->getPublishRibbon(), SLOT(setPublishEnabled(bool)));
        connect(objectFrame, SIGNAL(checkedChanged(bool)), _ribbon->getPublishRibbon(), SLOT(setChecked(bool)));
        connect(objectFrame, SIGNAL(checkableChanged(bool)), _ribbon->getPublishRibbon(), SLOT(setCheckable(bool)));
        connect(objectFrame, SIGNAL(rotationChanged(int)), _ribbon->getPublishRibbon(), SLOT(setRotation(int)));
        connect(objectFrame, SIGNAL(backgroundColorChanged(QColor)), _ribbon->getPublishRibbon(), SLOT(setColor(QColor)));

        objectFrame->activateObject(object);
        if(_ribbon && _ribbon->getPublishRibbon())
        {
            //_ribbon->getPublishRibbon()->setCheckable(objectFrame->isAnimated());
            objectFrame->setRotation(_ribbon->getPublishRibbon()->getRotation());
        }
    }
}

/*
int MainWindow::getWidgetFromType(int objectType)
{
    switch(objectType)
    {
        //ui->stackedWidgetEncounter->addWidget(encounterTextEdit);
        case DMHelper::CampaignType_Campaign:
        case DMHelper::CampaignType_Party:
        case DMHelper::CampaignType_Text:
        case DMHelper::CampaignType_Placeholder:
            return 1;
        //ui->stackedWidgetEncounter->addWidget(battleFrame);
        case DMHelper::CampaignType_Battle:
        case DMHelper::CampaignType_BattleContent:
            return 2;
        //ui->stackedWidgetEncounter->addWidget(charFrame);
        case DMHelper::CampaignType_Combatant:
            return 3;
        //ui->stackedWidgetEncounter->addWidget(mapFrame);
        case DMHelper::CampaignType_Map:
            return 4;
        //ui->stackedWidgetEncounter->addWidget(scrollingTextEdit);
        case DMHelper::CampaignType_ScrollingText:
            return 5;
        //ui->stackedWidgetEncounter->addWidget(audioTrackEdit);
        case DMHelper::CampaignType_AudioTrack:
            return 6;
        //ui->stackedWidgetEncounter->addWidget(welcomeFrame);
        case DMHelper::CampaignType_WelcomeScreen:
            return 7;
        case DMHelper::CampaignType_Base:
        default:
            qDebug() << "[MainWindow] ERROR: activate widget called with unexpected type: " << objectType;
            return 0;
    }
}
*/

void MainWindow::setRibbonToType(int objectType)
{
    //_ribbon->disableTab(_ribbonTabMap);
    //_ribbon->disableTab(_ribbonTabBattle);
    //_ribbon->disableTab(_ribbonTabMiniMap);
    //_ribbon->disableTab(_ribbonTabScrolling);
    //_ribbon->disableTab(_ribbonTabText);

    switch(objectType)
    {
        case DMHelper::CampaignType_Battle:
        case DMHelper::CampaignType_BattleContent:
            _ribbon->enableTab(_ribbonTabMap);
            _ribbon->enableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabMiniMap);
            _ribbon->disableTab(_ribbonTabScrolling);
            _ribbon->disableTab(_ribbonTabText);
            break;
        case DMHelper::CampaignType_Map:
            _ribbon->enableTab(_ribbonTabMiniMap);
            _ribbon->disableTab(_ribbonTabMap);
            _ribbon->disableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabScrolling);
            _ribbon->disableTab(_ribbonTabText);
            break;
        case DMHelper::CampaignType_ScrollingText:
            _ribbon->enableTab(_ribbonTabScrolling);
            _ribbon->disableTab(_ribbonTabMap);
            _ribbon->disableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabMiniMap);
            _ribbon->disableTab(_ribbonTabText);
            connect(_ribbon->getPublishRibbon(), SIGNAL(clicked(bool)), _scrollingTextEdit, SLOT(publishClicked(bool)));
            connect(_ribbon->getPublishRibbon(), SIGNAL(rotationChanged(int)), _scrollingTextEdit, SLOT(setRotation(int)));
            break;
        case DMHelper::CampaignType_Campaign:
        case DMHelper::CampaignType_Party:
        case DMHelper::CampaignType_Text:
            _ribbon->enableTab(_ribbonTabText);
            _ribbon->disableTab(_ribbonTabMap);
            _ribbon->disableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabMiniMap);
            _ribbon->disableTab(_ribbonTabScrolling);
            break;
        case DMHelper::CampaignType_Placeholder:
        case DMHelper::CampaignType_Base:
        case DMHelper::CampaignType_WelcomeScreen:
        case DMHelper::CampaignType_Combatant:
        case DMHelper::CampaignType_AudioTrack:
        default:
            _ribbon->disableTab(_ribbonTabMap);
            _ribbon->disableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabMiniMap);
            _ribbon->disableTab(_ribbonTabScrolling);
            _ribbon->disableTab(_ribbonTabText);
            break;
    }
}

#ifdef INCLUDE_CHASE_SUPPORT

void MainWindow::startChase()
{
    if(!campaign)
        return;

    QString chaseFileName = _options->getChaseFileName();

    if(chaseFileName.isEmpty())
    {
        chaseFileName = QFileDialog::getOpenFileName(this,QString("Select Chase File"),QString(),QString("XML files (*.xml)"));
        if(chaseFileName.isEmpty())
            return;

        _options->setChaseFileName(chaseFileName);
    }

    if(!chaseDlg)
    {
        ChaserSelectionDialog* selectDlg = new ChaserSelectionDialog(this);

        // Add the characters
        selectDlg->addCharacters(campaign->getActiveCharacters());

        // Add combatants from a selected battle encounter
        Encounter* encounter = encounterFromIndex(ui->treeView->currentIndex());
        if((encounter) && (encounter->getType() == DMHelper::EncounterType_Battle))
        {
            EncounterBattle* battle = dynamic_cast<EncounterBattle*>(encounter);
            if(battle)
            {
                selectDlg->addEncounterCombatants(battle->getCombatantsAllWaves());
            }
        }

        // Add living monsters from an existing battle
        /*
        TODO: Fix this for chases
        selectDlg->addBattleCombatants(_battleDlgMgr->getLivingMonsters());
        */

        if(selectDlg->exec() == QDialog::Rejected)
            return;

        chaseDlg = new ChaseDialog(chaseFileName, this);

        chaseDlg->addCombatants(selectDlg->getPursuers(), true);
        chaseDlg->addCombatantGroups(selectDlg->getPursuerGroups(), true);
        chaseDlg->addCombatants(selectDlg->getQuarry(), false);
        chaseDlg->addCombatantGroups(selectDlg->getQuarryGroups(), false);

        connect(chaseDlg, SIGNAL(chaseComplete()), this, SLOT(handleChaseComplete()));
        connect(chaseDlg, SIGNAL(publishChaseScene(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    }

    chaseDlg->show();
    chaseDlg->activateWindow();
}

void MainWindow::handleChaseComplete()
{
    if(!chaseDlg)
        return;

    QMessageBox::StandardButton result = QMessageBox::critical(chaseDlg, QString("Confirm Chase Complete"), QString("Are you sure you wish to end this chase? All changes will be discarded."), QMessageBox::Yes | QMessageBox::No);
    if(result == QMessageBox::Yes)
    {
        delete chaseDlg;
        chaseDlg = nullptr;
    }
}

#endif //INCLUDE_CHASE_SUPPORT
