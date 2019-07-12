#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "publishwindow.h"
#include "publishframe.h"
#include "dicerolldialog.h"
#include "dicerollframe.h"
#include "countdownframe.h"
#include "character.h"
#include "characterimporter.h"
#include "objectimporter.h"
#include "characterframe.h"
#include "campaign.h"
#include "adventure.h"
#include "encounter.h"
#include "map.h"
#include "mapframe.h"
#include "mruhandler.h"
#include "encounterfactory.h"
#include "encountertextedit.h"
#include "encounterbattleedit.h"
#include "encounterbattle.h"
#include "encounterscrollingtext.h"
#include "encounterscrollingtextedit.h"
#include "combatant.h"
#include "campaigntreemodel.h"
#include "battledialogmanager.h"
#include "audioplaybackframe.h"
#include "monster.h"
#include "monsterclass.h"
#include "bestiary.h"
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

// Next Todos:
// DONE: Add empty DMHelper.log as part of the deploy procedure

// Duplicate monsters in bestiary
// Update bestiary to have proficiencies, etc separately stored

//  1. Finish settings (publish and other interactions) - DONE
//  2. Add NPCs in the world
        // Check remove/drag settings and NPCs
        // Check NPC adding to chases and battles
        // track tree collapsing
//  4. Add NPCs and monsters to the party
//
//  Refactor mainwindow, make it smaller
//  Avoid redrawing the tree every time a change is made
//  Add maps to Settings
//  Add NPCs - first just to the tree, then to battles, chases or party
//  Add non-characters to the party (monsters or NPCs)
//  Spell reference
// Equipment tables as XML

/*
 * TODO:
 * More editing ability for UI elements (campaign, adventure, encounter, maps - rename, remap, etc)
 * Open PDF: D:\Data\Personal\Documents\Dnd\Regeln>"C:\Program Files (x86)\Adobe\Acrobat Reader DC\Reader\acrord32" /A "page=100&view=Fit" "dnd 5e players handbook.pdf"
 *          <a href="file://DnD 5e Players Handbook.pdf#page=100&view=FitBV">OPEN FILE</a>
 * Subclass for windows: Ctrl-M for main window or so
 * Finish chase
 * Options for re-selecting Bestiary or Chase data, DM Guide, MM, PHB
    //QDesktopServices::openUrl(QUrl::fromLocalFile("C:\\Users\\deincrtu\\Documents\\Personal\\Documents\\Dnd\\Regeln\\DnD 5e Players Handbook.pdf"));
 * Quick links for external references
 * Cross-populate preview screens
 * Image publisher
 * Drag and drop for publishing images
 * More DM Screen data
 * Battle-type encounters - improve
 * Add People
 * Add Places
 * Drag and Drop to add maps and places
 * Check member varaible naming convention
 * Maps - Tokens and DM layers, clickable?
 * Links between map edges
 * different brushes for FOW clearing
 * Full character sheet
 * Drag and Drop reorder of characters
 *
 * DONE - Drag and drop for publishing text (pre-supported)
 * DONE - Text publishing
 * DONE - DM Screen data
 * DONE - Drag and Drop reorder of enounters/maps
 */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    pubWindow(nullptr),
    previewTab(nullptr),
    timeAndDateFrame(nullptr),
    previewFrame(nullptr),
    encounterTextEdit(nullptr),
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
    _animationFrameCount(DMHelper::ANIMATION_TIMER_PREVIEW_FRAMES)
{
    qDebug() << "[Main] Initializing Main";

    qDebug() << "[Main] DMHelper version information";
    qDebug() << "[Main]     DMHelper Version: " << QString::number(DMHelper::DMHELPER_MAJOR_VERSION) + "." + QString::number(DMHelper::DMHELPER_MINOR_VERSION);
    qDebug() << "[Main]     Expected Bestiary Version: " << QString::number(DMHelper::BESTIARY_MAJOR_VERSION) + "." + QString::number(DMHelper::BESTIARY_MINOR_VERSION);
    qDebug() << "[Main]     Expected Campaign File Version: " << QString::number(DMHelper::CAMPAIGN_MAJOR_VERSION) + "." + QString::number(DMHelper::CAMPAIGN_MINOR_VERSION);
    qDebug() << "[Main]     Build: " << __DATE__ << " " << __TIME__;
    qDebug() << "[Main]     Working Directory: " << QDir::currentPath();

    qDebug() << "[Main] Qt Information";
    qDebug() << "[Main]     Qt Version: " << QLibraryInfo::version().toString();
    qDebug() << "[Main]     Is Debug? " << QLibraryInfo::isDebugBuild();
    qDebug() << "[Main]     PrefixPath: " << QLibraryInfo::location(QLibraryInfo::PrefixPath);
    qDebug() << "[Main]     DocumentationPath: " << QLibraryInfo::location(QLibraryInfo::DocumentationPath);
    qDebug() << "[Main]     HeadersPath: " << QLibraryInfo::location(QLibraryInfo::HeadersPath);
    qDebug() << "[Main]     LibrariesPath: " << QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    qDebug() << "[Main]     LibraryExecutablesPath: " << QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath);
    qDebug() << "[Main]     BinariesPath: " << QLibraryInfo::location(QLibraryInfo::BinariesPath);
    qDebug() << "[Main]     PluginsPath: " << QLibraryInfo::location(QLibraryInfo::PluginsPath);
    qDebug() << "[Main]     ImportsPath: " << QLibraryInfo::location(QLibraryInfo::ImportsPath);
    qDebug() << "[Main]     Qml2ImportsPath: " << QLibraryInfo::location(QLibraryInfo::Qml2ImportsPath);
    qDebug() << "[Main]     ArchDataPath: " << QLibraryInfo::location(QLibraryInfo::ArchDataPath);
    qDebug() << "[Main]     DataPath: " << QLibraryInfo::location(QLibraryInfo::DataPath);
    qDebug() << "[Main]     TranslationsPath: " << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    qDebug() << "[Main]     ExamplesPath: " << QLibraryInfo::location(QLibraryInfo::ExamplesPath);
    qDebug() << "[Main]     TestsPath: " << QLibraryInfo::location(QLibraryInfo::TestsPath);
    qDebug() << "[Main]     SettingsPath: " << QLibraryInfo::location(QLibraryInfo::SettingsPath);

    // TODO: cleanup this constructor and mainwindow in general
    ui->setupUi(this);

    //qsrand(static_cast<unsigned int>(QTime::currentTime().msec()));

    qDebug() << "[Main] Initializing Bestiary";
    Bestiary::Initialize();
    qDebug() << "[Main] Bestiary Initialized";

    qDebug() << "[Main] Initializing BasicDateServer";
    BasicDateServer::Initialize();
    qDebug() << "[Main] BasicDateServer Initialized";

    qDebug() << "[Main] Initializing EquipmentServer";
    EquipmentServer::Initialize();
    qDebug() << "[Main] BasicDateServer Initialized";

    connect(ui->action_NewCampaign,SIGNAL(triggered()),this,SLOT(newCampaign()));
    connect(ui->action_OpenCampaign,SIGNAL(triggered()),this,SLOT(openFileDialog()));
    connect(ui->action_SaveCampaign,SIGNAL(triggered()),this,SLOT(saveCampaign()));
    connect(ui->actionSave_Campaign_As,SIGNAL(triggered()),this,SLOT(saveCampaignAs()));
    connect(ui->actionClose_Campaign,SIGNAL(triggered()),this,SLOT(closeCampaign()));
    //connect(ui->actionE_xit,SIGNAL(triggered()),qApp,SLOT(quit()));
    connect(ui->actionE_xit,SIGNAL(triggered()),this,SLOT(close()));
    connect(ui->actionDice,SIGNAL(triggered()),this,SLOT(openDiceDialog()));

    connect(this,SIGNAL(campaignLoaded(Campaign*)),this,SLOT(handleCampaignLoaded(Campaign*)));
    connect(this,SIGNAL(campaignLoaded(Campaign*)),this,SLOT(clearDirty()));
    connect(ui->actionNew_Adventure,SIGNAL(triggered()),this,SLOT(newAdventure()));
    connect(ui->actionNew_Character,SIGNAL(triggered()),this,SLOT(newCharacter()));
    connect(ui->actionNew_Text_Encounter,SIGNAL(triggered()),this,SLOT(newTextEncounter()));
    connect(ui->actionNew_Battle_Encounter,SIGNAL(triggered()),this,SLOT(newBattleEncounter()));
    connect(ui->actionNew_Scrolling_Text_Encounter,SIGNAL(triggered()),this,SLOT(newScrollingTextEncounter()));
    connect(ui->actionNew_Map,SIGNAL(triggered()),this,SLOT(newMap()));
    connect(ui->actionExport_Item,SIGNAL(triggered()),this,SLOT(exportCurrentItem()));
    connect(ui->actionImport_Item,SIGNAL(triggered()),this,SLOT(importItem()));
    // TODO: reenable Import Character (?)
    //connect(ui->action_Import_Character,SIGNAL(triggered()),this,SLOT(importCharacter()));
    ui->action_Import_Character->setVisible(false);
    connect(ui->actionStart_Battle,SIGNAL(triggered()),this,SLOT(handleStartNewBattle()));

    connect(ui->action_Open_Bestiary,SIGNAL(triggered()),this,SLOT(openBestiary()));
    connect(ui->action_New_Monster,SIGNAL(triggered()),&bestiaryDlg,SLOT(createNewMonster()));
    connect(ui->actionOpen_Players_Window,SIGNAL(triggered()),this,SLOT(showPublishWindow()));
    connect(ui->actionPublish_Text,SIGNAL(triggered()),this,SLOT(openTextPublisher()));
    connect(ui->actionTranslate_Text,SIGNAL(triggered()),this,SLOT(openTextTranslator()));
    connect(ui->actionRandom_Market,SIGNAL(triggered()),this,SLOT(openRandomMarkets()));
#ifdef INCLUDE_CHASE_SUPPORT
    connect(ui->action_Chase_Dialog,SIGNAL(triggered()),this,SLOT(startChase()));
#endif
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(openAboutDialog()));

    connect(ui->treeView,SIGNAL(expanded(QModelIndex)),this,SLOT(handleTreeItemExpanded(QModelIndex)));
    connect(ui->treeView,SIGNAL(collapsed(QModelIndex)),this,SLOT(handleTreeItemCollapsed(QModelIndex)));

    qDebug() << "[Main] Creating Player's Window";
    pubWindow = new PublishWindow(QString("DM Helper Player's Window"));
    qDebug() << "[Main] Player's Window Created";

    qDebug() << "[Main] Creating Tree Model";
    ui->treeView->setHeaderHidden(true);
    treeModel = new CampaignTreeModel(ui->treeView);
    ui->treeView->setModel(treeModel);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(ui->treeView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(handleCustomContextMenu(QPoint)));
    connect(ui->treeView->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(handleTreeItemSelected(QModelIndex,QModelIndex)));
    connect(ui->treeView,SIGNAL(activated(QModelIndex)),this,SLOT(handleTreeItemDoubleClicked(QModelIndex)));
    connect(treeModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(handleTreeItemChanged(QStandardItem*)));
    qDebug() << "[Main] Tree Model Created";

    connect(Bestiary::Instance(),SIGNAL(changed()),&bestiaryDlg,SLOT(dataChanged()));

    qDebug() << "[Main] Reading Settings";
    _options = new OptionsContainer(this);
    MRUHandler* mruHandler = new MRUHandler(ui->menuRecent_Campaigns, DEFAULT_MRU_FILE_COUNT, this);
    connect(mruHandler,SIGNAL(triggerMRU(QString)),this,SLOT(openFile(QString)));
    _options->setMRUHandler(mruHandler);
    _options->readSettings();
    qDebug() << "[Main] Settings Read";

    connect(ui->actionOptions,SIGNAL(triggered()),_options,SLOT(editSettings()));
    connect(_options,SIGNAL(bestiaryFileNameChanged()),this,SLOT(readBestiary()));
    connect(_options,SIGNAL(showAnimationsChanged(bool)),ui->scrollWidget,SLOT(setAnimatedTransitions(bool)));
    qDebug() << "[Main] Loading Bestiary";
    readBestiary();
    qDebug() << "[Main] Bestiary Loaded";

    connect(this, SIGNAL(dispatchPublishImage(QImage, QColor)), this, SLOT(showPublishWindow()));
    connect(this, SIGNAL(dispatchPublishImage(QImage, QColor)), pubWindow, SLOT(setImage(QImage, QColor)));
    connect(this, SIGNAL(dispatchAnimateImage(QImage)), pubWindow, SLOT(setImageNoScale(QImage)));
    connect(this, SIGNAL(dispatchAnimateImage(QImage)), this, SLOT(handleAnimationPreview(QImage)));

    connect(&bestiaryDlg,SIGNAL(publishMonsterImage(QImage, QColor)),this,SIGNAL(dispatchPublishImage(QImage, QColor)));

    // Add the encounter pages to the stacked widget - implicit mapping to EncounterType enum values
    // TODO: insert a MAP in between to be explicit about this mapping
    qDebug() << "[Main] Creating Encounter Pages";
    // EncounterType_Text
    encounterTextEdit = new EncounterTextEdit;
    connect(encounterTextEdit, SIGNAL(anchorClicked(QUrl)), this, SLOT(linkActivated(QUrl)));
    ui->stackedWidgetEncounter->addWidget(encounterTextEdit);
    // EncounterType_Battle
    EncounterBattleEdit* encounterBattleEdit = new EncounterBattleEdit;
    connect(encounterBattleEdit, SIGNAL(openMonster(QString)), this, SLOT(openMonster(QString)));
    ui->stackedWidgetEncounter->addWidget(encounterBattleEdit);
    // EncounterType_Character
    /*
    QScrollArea* scrollArea = new QScrollArea;
    CharacterFrame* charFrame = new CharacterFrame;
    scrollArea->setWidget(charFrame);
    ui->stackedWidgetEncounter->addWidget(scrollArea);
    connect(charFrame, SIGNAL(publishCharacterImage(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    */
    CharacterFrame* charFrame = new CharacterFrame;
    ui->stackedWidgetEncounter->addWidget(charFrame);
    connect(charFrame, SIGNAL(publishCharacterImage(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    // EncounterType_Map
    MapFrame* mapFrame = new MapFrame;
    ui->stackedWidgetEncounter->addWidget(mapFrame);
    connect(mapFrame,SIGNAL(publishImage(QImage, QColor)),this,SIGNAL(dispatchPublishImage(QImage, QColor)));
    // EncounterType_ScrollingText
    EncounterScrollingTextEdit* scrollingTextEdit = new EncounterScrollingTextEdit;
    ui->stackedWidgetEncounter->addWidget(scrollingTextEdit);
    // EncounterType_AudioTrack
    AudioTrackEdit* audioTrackEdit = new AudioTrackEdit;
    connect(this, SIGNAL(campaignLoaded(Campaign*)), audioTrackEdit, SLOT(setCampaign(Campaign*)));
    ui->stackedWidgetEncounter->addWidget(audioTrackEdit);
    qDebug() << "[Main] Encounter Pages Created";

    // Load the quick reference tabs
    qDebug() << "[Main] Creating Reference Tabs";
    previewFrame = new PublishFrame(this);
    connect(previewFrame,SIGNAL(visibleChanged(bool)),pubWindow,SLOT(setArrowVisible(bool)));
    connect(previewFrame,SIGNAL(visibleChanged(bool)),previewFrame,SLOT(setArrowVisible(bool)));
    connect(previewFrame,SIGNAL(positionChanged(QPointF)),previewFrame,SLOT(setArrowPosition(QPointF)));
    connect(previewFrame,SIGNAL(positionChanged(QPointF)),pubWindow,SLOT(setArrowPosition(QPointF)));
    connect(this,SIGNAL(dispatchPublishImage(QImage,QColor)),previewFrame,SLOT(setImage(QImage)));
    // Add the preview tab
    previewTab = new ScrollTabWidget(previewFrame, QSizeF(0.9, 0.9), this);
    previewTab->setToolTip(QString("Preview"));
    ui->scrollWidget->addTab(previewTab, QIcon(QPixmap(":/img/data/icon_preview.png")));
    // Add the time/date tab
    timeAndDateFrame = new TimeAndDateFrame(this);
    timeAndDateFrame->setToolTip(QString("Time & Date"));
    ui->scrollWidget->addTab(new ScrollTabWidget(timeAndDateFrame, QSizeF(0,0), this), QIcon(QPixmap(":/img/data/icon_clock.png")));
    // Add the quick reference frame
    ScrollTabWidget* w2 = new ScrollTabWidget(new QuickRefFrame(this), QSizeF(0,0), this);
    w2->setToolTip(QString("Quick Reference"));
    ui->scrollWidget->addTab(w2, QIcon(QPixmap(":/img/data/icon_reference.png")));
    // Add the DM screen widget
    DMScreenTabWidget* dmScreen = new DMScreenTabWidget(this);
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
    // Add the audio playback frame
    AudioPlaybackFrame* audioPlaybackFrame = new AudioPlaybackFrame(this);
    audioPlaybackFrame->setToolTip(QString("Audio Playback"));
    ui->scrollWidget->addTab(new ScrollTabWidget(audioPlaybackFrame, QSizeF(0,0), this), QIcon(QPixmap(":/img/data/icon_music.png")));
    //Initialize animation settings
    ui->scrollWidget->setAnimatedTransitions(_options->getShowAnimations());
    qDebug() << "[Main] Reference Tabs Created";

    qDebug() << "[Main] Initializing Battle Dialog Manager";
    _battleDlgMgr = new BattleDialogManager(this);
    connect(ui->actionBattle_Dialog, SIGNAL(triggered()), _battleDlgMgr, SLOT(showBattleDialog()));
    connect(_battleDlgMgr, SIGNAL(battleActive(bool)), ui->actionBattle_Dialog, SLOT(setEnabled(bool)));
    connect(_battleDlgMgr, SIGNAL(characterSelected(QUuid)), this, SLOT(openCharacter(QUuid)));
    connect(_battleDlgMgr, SIGNAL(monsterSelected(QString)), this, SLOT(openMonster(QString)));
    connect(_battleDlgMgr, SIGNAL(publishImage(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    connect(_battleDlgMgr, SIGNAL(animateImage(QImage)), this, SIGNAL(dispatchAnimateImage(QImage)));
    connect(_battleDlgMgr, SIGNAL(animationStarted(QColor)), this, SLOT(handleAnimationStarted(QColor)));
    connect(_battleDlgMgr, SIGNAL(showPublishWindow()), this, SLOT(showPublishWindow()));
    connect(_battleDlgMgr, SIGNAL(dirty()), this, SLOT(setDirty()));
    connect(pubWindow, SIGNAL(frameResized(QSize)), _battleDlgMgr, SLOT(targetResized(QSize)));
    connect(this, SIGNAL(campaignLoaded(Campaign*)), _battleDlgMgr, SLOT(setCampaign(Campaign*)));
    connect(this, SIGNAL(dispatchPublishImage(QImage,QColor)), _battleDlgMgr, SLOT(cancelPublish()));
    _battleDlgMgr->setShowOnDeck(_options->getShowOnDeck());
    _battleDlgMgr->setShowCountdown(_options->getShowCountdown());
    _battleDlgMgr->setCountdownDuration(_options->getCountdownDuration());
    connect(_options, SIGNAL(showOnDeckChanged(bool)), _battleDlgMgr, SLOT(setShowOnDeck(bool)));
    connect(_options, SIGNAL(showCountdownChanged(bool)), _battleDlgMgr, SLOT(setShowCountdown(bool)));
    connect(_options, SIGNAL(countdownDurationChanged(int)), _battleDlgMgr, SLOT(setCountdownDuration(int)));
    connect(encounterBattleEdit, SIGNAL(startBattle(EncounterBattle*)), _battleDlgMgr, SLOT(startNewBattle(EncounterBattle*)));
    connect(encounterBattleEdit, SIGNAL(loadBattle(EncounterBattle*)), _battleDlgMgr, SLOT(loadBattle(EncounterBattle*)));
    connect(encounterBattleEdit, SIGNAL(deleteBattle(EncounterBattle*)), _battleDlgMgr, SLOT(deleteBattle(EncounterBattle*)));
    qDebug() << "[Main] Battle Dialog Manager Initialized.";

    _audioPlayer = new AudioPlayer(this);
    _audioPlayer->setVolume(_options->getAudioVolume());
    audioPlaybackFrame->setVolume(_options->getAudioVolume());
    connect(audioTrackEdit, SIGNAL(trackSelected(AudioTrack*)), _audioPlayer, SLOT(playTrack(AudioTrack*)));
    connect(_audioPlayer, SIGNAL(positionChanged(qint64)), audioPlaybackFrame, SLOT(setPosition(qint64)));
    connect(_audioPlayer, SIGNAL(durationChanged(qint64)), audioPlaybackFrame, SLOT(setDuration(qint64)));
    connect(_audioPlayer, SIGNAL(trackChanged(AudioTrack*)), audioPlaybackFrame, SLOT(trackChanged(AudioTrack*)));
    connect(_audioPlayer, SIGNAL(stateChanged(AudioPlayer::State)), audioPlaybackFrame, SLOT(stateChanged(AudioPlayer::State)));
    connect(audioPlaybackFrame, SIGNAL(play()), _audioPlayer, SLOT(play()));
    connect(audioPlaybackFrame, SIGNAL(pause()), _audioPlayer, SLOT(pause()));
    connect(audioPlaybackFrame, SIGNAL(positionChanged(qint64)), _audioPlayer, SLOT(setPosition(qint64)));
    connect(audioPlaybackFrame, SIGNAL(volumeChanged(int)), _audioPlayer, SLOT(setVolume(int)));
    connect(audioPlaybackFrame, SIGNAL(volumeChanged(int)), _options, SLOT(setAudioVolume(int)));
    connect(mapFrame, SIGNAL(startTrack(AudioTrack*)), _audioPlayer, SLOT(playTrack(AudioTrack*)));
    connect(encounterBattleEdit, SIGNAL(startTrack(AudioTrack*)), _audioPlayer, SLOT(playTrack(AudioTrack*)));

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

    qDebug() << "[Main] Main Initialization complete";
}

MainWindow::~MainWindow()
{
    delete _battleDlgMgr;

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
        qDebug() << "[Main] Campaign created: " << campaignName;
        emit campaignLoaded(campaign);
        selectItem(DMHelper::TreeType_Campaign, QUuid());
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
            qDebug() << "[Main] Invalid campaign not saved";
            return false;
        }
        else
        {
            qDebug() << "[Main] Invalid campaign saved despite warning!";
        }
    }

    if(campaignFileName.isEmpty())
    {
        campaignFileName = QFileDialog::getSaveFileName(this,QString("Save Campaign"),QString(),QString("XML files (*.xml)"));
        if(campaignFileName.isEmpty())
            return false;
    }

    qDebug() << "[Main] Saving Campaign: " << campaignFileName;

    QDomDocument doc( "DMHelperXML" );

    QDomElement root = doc.createElement( "root" );
    doc.appendChild( root );

    QFileInfo fileInfo(campaignFileName);
    QDir targetDir(fileInfo.absoluteDir());
    campaign->outputXML(doc, root, targetDir, false);

    QFile file(campaignFileName);
    if( !file.open( QIODevice::WriteOnly ) )
    {
        campaignFileName.clear();
        return false;
    }

    QTextStream ts( &file );
    ts.setCodec("UTF-8");
    ts << doc.toString();

    file.close();

    clearDirty();

    qDebug() << "[Main] Campaign saved: " << campaignFileName;

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
    qDebug() << "[Main] Closing Campaign: " << campaignFileName;

    if(dirty)
    {
        QMessageBox::StandardButton result = QMessageBox::question(this,
                                                                   QString("Save Campaign"),
                                                                   QString("Would you like to save the current campaign before proceeding? Unsaved changes will be lost."),
                                                                   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
        if(result == QMessageBox::Cancel)
        {
            qDebug() << "[Main] Closíng Campaign cancelled";
            return false;
        }

        if(result == QMessageBox::Yes)
        {
            saveCampaign();
        }
        else
        {
            qDebug() << "[Main] User decided not to save Campaign: " << campaignFileName;
        }
    }

    deleteCampaign();
    clearDirty();

    qDebug() << "[Main] Campaign closed";

    return true;
}

void MainWindow::openDiceDialog()
{
    DiceRollDialog *drDlg = new DiceRollDialog(this);
    drDlg->show();
}

void MainWindow::openCharacter(QUuid id)
{
    if((!campaign) || (id.isNull()))
        return;

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

    activateWindow();
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
    QString characterName = QInputDialog::getText(this, QString("Enter New Character Name"),QString("Character"),QLineEdit::Normal,QString(),&ok);
    if((!ok) || (characterName.isEmpty()))
        return;

    Character* newCharacter = new Character();
    newCharacter->setName(characterName);
    campaign->addCharacter(newCharacter);
    openCharacter(newCharacter->getID());
}

void MainWindow::importCharacter()
{
    if(!campaign)
        return;

    CharacterImporter importer;
    openCharacter(importer.importCharacter(*campaign));
}

void MainWindow::importItem()
{
    if(!campaign)
        return;

    ObjectImporter importer;
    importer.importObject(*campaign);
}

void MainWindow::newNPC()
{
    bool ok;
    QString npcName = QInputDialog::getText(this, QString("Enter New NPC Name"),QString("NPC"),QLineEdit::Normal,QString(),&ok);
    if((!ok) || (npcName.isEmpty()))
        return;

    Character* newNPC = new Character();
    newNPC->setName(npcName);
    campaign->addNPC(newNPC);
    openCharacter(newNPC->getID());
}

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

void MainWindow::newAdventure()
{
    if(!campaign)
        return;

    bool ok;
    QString adventureName = QInputDialog::getText(this, QString("Enter Adventure Name"),QString("New Adventure"),QLineEdit::Normal,QString(),&ok);
    if(ok)
    {
        Adventure* newAdventure = new Adventure(adventureName);
        campaign->addAdventure(newAdventure);
        selectItem(DMHelper::TreeType_Adventure, newAdventure->getID());
    }
}

void MainWindow::newTextEncounter()
{
    newEncounter(DMHelper::EncounterType_Text);
}

void MainWindow::newBattleEncounter()
{
    newEncounter(DMHelper::EncounterType_Battle);
}

void MainWindow::newScrollingTextEncounter()
{
    newEncounter(DMHelper::EncounterType_ScrollingText);
}

void MainWindow::newMap()
{
    if((!campaign)||(!treeModel))
        return;

    QStandardItem* parentItem = findParentbyType(treeModel->itemFromIndex(ui->treeView->currentIndex()), DMHelper::TreeType_Adventure);
    if(!parentItem)
    {
        parentItem = findParentbyType(treeModel->itemFromIndex(ui->treeView->currentIndex()), DMHelper::TreeType_Settings_Title);
        if(!parentItem)
        {
            return;
        }
    }

    QUuid id = QUuid(parentItem->data(DMHelper::TreeItemData_ID).toString());
    bool ok = false;
    QString mapName = QInputDialog::getText(this, QString("Enter Map Name"),QString("New Map"),QLineEdit::Normal,QString(),&ok);
    if(!ok)
        return;

    QString filename = QFileDialog::getOpenFileName(this,QString("Select Map Image..."));
    if(filename.isEmpty())
        return;

    Map* newMap = new Map(mapName, filename);

    Adventure* adventure = campaign->getAdventureById(id);
    if(adventure)
    {
        adventure->addMap(newMap);
    }
    else
    {
        campaign->addSetting(newMap);
    }

    selectItem(DMHelper::TreeType_Map, newMap->getID(), id);
}

void MainWindow::editCurrentMap()
{
    if((!campaign)||(!treeModel))
        return;

    QStandardItem* mapItem = treeModel->itemFromIndex(ui->treeView->currentIndex());
    int type = mapItem->data(DMHelper::TreeItemData_Type).toInt();
    if(type == DMHelper::TreeType_Map)
    {
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

        QString filename = QFileDialog::getOpenFileName(this,QString("Select Map Image..."));
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

    QStandardItem* removeItem = treeModel->itemFromIndex(ui->treeView->currentIndex());
    int type = removeItem->data(DMHelper::TreeItemData_Type).toInt();
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
                encounter->widgetDeactivated(ui->stackedWidgetEncounter->currentWidget());
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
        Map* map = nullptr;
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

        if(map)
        {
            bool ok = false;
            QString mapName = QInputDialog::getText(this, QString("Enter Map Name"),QString("New Map"),QLineEdit::Normal,QString(),&ok);
            if(ok)
            {
                map->setName(mapName); // TODO: Why doesn't this emit changed?
                updateCampaignTree();
            }
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

    qDebug() << "[Main] Exporting object with ID " << exportId << " to " << exportFileName;

    CampaignExporter exporter(*campaign, exportId, targetDir);
    if(!exporter.isValid())
    {
        qDebug() << "[Main] Error - invalid export created!";
        return;
    }

    QString exportString = exporter.getExportDocument().toString();
    if(exportString.isEmpty())
    {
        qDebug() << "[Main] Error - export null string found, no export created!";
        return;
    }

    QFile file(exportFileName);
    if( !file.open( QIODevice::WriteOnly ) )
    {
        qDebug() << "[Main] Not able to open export file " << exportFileName;
        return;
    }

    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    ts << exportString;
    file.close();

    qDebug() << "[Main] Export complete";
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

void MainWindow::showPublishWindow()
{
    if(!pubWindow->isVisible())
    {
        pubWindow->show();
        pubWindow->activateWindow();
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
    if(!Bestiary::Instance())
        return;

    QString bestiaryFileName = _options->getBestiaryFileName();

    if(bestiaryFileName.isEmpty())
    {
        /*
        QMessageBox::StandardButton result = QMessageBox::question(this, QString("Select Bestiary"),QString("A current bestiary could not be found. Would you like to open an existing bestiary?"));

        if(result == QMessageBox::Yes)
        {
            bestiaryFileName = QFileDialog::getOpenFileName(this,QString("Select Bestiary File"),QString(),QString("XML files (*.xml)"));
        }
        else
        {
            bestiaryFileName = QFileDialog::getSaveFileName(this,QString("Select New Bestiary File"),QString(),QString("XML files (*.xml)"));
        }

        if(bestiaryFileName.isEmpty())
            return;

        _options->setBestiaryFileName(bestiaryFileName);
        */

        qDebug() << "[Main] No known bestiary found, attempting to load default bestiary";
        bestiaryFileName = QString("./bestiary/DMHelperBestiary.xml");
    }

    qDebug() << "[Main] Reading bestiary: " << bestiaryFileName;

    QDomDocument doc( "DMHelperBestiaryXML" );
    QFile file( bestiaryFileName );
    if( !file.open( QIODevice::ReadOnly ) )
    {
        qDebug() << "[Main] Reading bestiary file open failed.";
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
        qDebug() << "[Main] Reading bestiary reading XML content.";
        qDebug() << errMsg << errRow << errColumn;
        return;
    }

    QDomElement root = doc.documentElement();
    if( (root.isNull()) || (root.tagName() != "root") )
        return;

    QFileInfo fileInfo(bestiaryFileName);
    Bestiary::Instance()->setDirectory(fileInfo.absoluteDir());
    Bestiary::Instance()->inputXML(root, false);

    if(!_options->getLastMonster().isEmpty() && Bestiary::Instance()->exists(_options->getLastMonster()))
        bestiaryDlg.setMonster(_options->getLastMonster());
    else
        bestiaryDlg.setMonster(Bestiary::Instance()->getFirstMonsterClass());
}

void MainWindow::showEvent(QShowEvent * event)
{
    qDebug() << "[Main] Main window Show event.";
    if(!initialized)
    {
        // Implement any one-time initialization here
        initialized = true;
    }

    ui->scrollWidget->resizeTabs();

    QMainWindow::showEvent(event);
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    Q_UNUSED(event);

    qDebug() << "[Main] Close event received.";

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
    ui->scrollWidget->resizeTabs();
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


void MainWindow::deleteCampaign()
{
    if(treeModel)
    {
        // Deselect the current object
        handleTreeItemSelected(QModelIndex(), ui->treeView->currentIndex());
        // Clear the tree model
        treeModel->clear();
    }

    Campaign* oldCampaign = campaign;
    campaign = nullptr;
    emit campaignLoaded(campaign);

    // Clear the campaign itself
    delete oldCampaign;

    // Ensure the file name is removed
    campaignFileName.clear();
}

void MainWindow::enableCampaignMenu()
{
    ui->menu_Campaign->setEnabled(campaign != nullptr);

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
}

Encounter* MainWindow::notesFromIndex(const QModelIndex & index)
{
    if( (!treeModel) || (!index.isValid()) )
        return nullptr;

    QStandardItem* item = treeModel->itemFromIndex(index);
    if(!item)
        return nullptr;

    int type = item->data(DMHelper::TreeItemData_Type).toInt();
    if(type != DMHelper::TreeType_Notes)
        return nullptr;

    return campaign->getNotes();
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
}

Encounter* MainWindow::encounterFromIndex(const QModelIndex & index)
{
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
}

Map* MainWindow::mapFromIndex(const QModelIndex & index)
{
    if( (!treeModel) || (!index.isValid()) )
        return nullptr;

    QStandardItem* item = treeModel->itemFromIndex(index);
    if(!item)
        return nullptr;

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
}

bool MainWindow::selectItem(int itemType, QUuid itemId)
{
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
}

bool MainWindow::selectItem(int itemType, QUuid itemId, QUuid adventureId)
{
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
    if(!Bestiary::Instance())
        return;

    if(Bestiary::Instance()->count() <= 0)
    {
        qDebug() << "[Main] Bestiary is empty, no file will be written";
        return;
    }

    QString bestiaryFileName = _options->getBestiaryFileName();
    qDebug() << "[Main] Writing Bestiary to " << bestiaryFileName;

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
        qDebug() << "[Main] Bestiary output did not find any monsters. Aborting writing to file";
        return;
    }

    QFile file(bestiaryFileName);
    if( !file.open( QIODevice::WriteOnly ) )
    {
        qDebug() << "[Main] Unable to open Bestiary file for writing: " << bestiaryFileName;
        bestiaryFileName.clear();
        return;
    }

    QTextStream ts( &file );
    ts.setCodec("UTF-8");
    ts << doc.toString();

    file.close();
}

void MainWindow::newEncounter(int encounterType)
{
    if((!campaign)||(!treeModel))
        return;

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
}

void MainWindow::openFile(const QString& filename)
{
    if( !closeCampaign() )
        return;

    qDebug() << "[Main] Loading Campaign: " << filename;

    QDomDocument doc( "DMHelperXML" );
    QFile file( filename );
    if( !file.open( QIODevice::ReadOnly ) )
    {
        qDebug() << "[Main] Loading Failed: Unable to open campaign file";
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString contentError;
    int contentErrorLine = 0;
    int contentErrorColumn = 0;
    bool contentResult = doc.setContent(in.readAll(), &contentError, &contentErrorLine, &contentErrorColumn);

    file.close();

    if( contentResult == false )
    {
        qDebug() << "[Main] Loading Failed: Error reading XML (line " << contentErrorLine << ", column " << contentErrorColumn << "): " << contentError;
        return;
    }

    QDomElement root = doc.documentElement();
    if( (root.isNull()) || (root.tagName() != "root") )
    {
        qDebug() << "[Main] Loading Failed: Error reading XML - unable to find root entry";
        return;
    }

    QDomElement campaignElement = root.firstChildElement( QString("campaign") );
    if( campaignElement.isNull() )
    {
        qDebug() << "[Main] Loading Failed: Error reading XML - unable to find campaign entry";
        return;
    }

    campaignFileName = filename;
    QFileInfo fileInfo(campaignFileName);
    QDir::setCurrent(fileInfo.absolutePath());
    campaign = new Campaign(campaignElement, false);
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
            qDebug() << "[Main] Invalid campaign discarded";
            delete campaign;
            campaign = nullptr;
            return;
        }
    }

    emit campaignLoaded(campaign);
    selectItem(DMHelper::TreeType_Campaign, QUuid());

    //_battleDlgMgr->loadBattle(campaign, root);

    if(_options->getMRUHandler())
        _options->getMRUHandler()->addMRUFile(filename);
}

void MainWindow::handleCampaignLoaded(Campaign* campaign)
{
    qDebug() << "[Main] Campaign Loaded: " << campaignFileName;

    ui->stackedWidgetEncounter->setCurrentIndex(DMHelper::EncounterType_Blank);

    updateCampaignTree();
    updateMapFiles();
    updateClock();

    if(campaign)
    {
        connect(campaign,SIGNAL(dirty()),this,SLOT(setDirty()));
        connect(campaign,SIGNAL(changed()),this,SLOT(updateCampaignTree()));
        setWindowTitle(QString("DM Helper - ") + campaign->getName() + QString("[*]"));
    }
    else
    {
        setWindowTitle(QString("DM Helper [*]"));
    }
}

void MainWindow::updateCampaignTree()
{
    qDebug() << "[Main] Updating Campaign Tree";
    enableCampaignMenu();

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
}

void MainWindow::updateMapFiles()
{
    if(!campaign)
        return;

    QFileInfo fileInfo(campaignFileName);
    QDir sourceDir(fileInfo.absoluteDir());

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
    if(!treeModel)
        return;

    QModelIndex index = ui->treeView->indexAt(point);
    if( index.isValid() )
    {
        QStandardItem* item = treeModel->itemFromIndex(index);
        if(item)
        {
            QMenu* contextMenu = new QMenu(ui->treeView);
            int type = item->data(DMHelper::TreeItemData_Type).toInt();

            QAction* addCampgainAdventure = new QAction(QString("Add Adventure"), contextMenu);
            connect(addCampgainAdventure,SIGNAL(triggered()),this,SLOT(newAdventure()));
            contextMenu->addAction(addCampgainAdventure);
            QAction* addCampgainCharacter = new QAction(QString("Add Character"), contextMenu);
            connect(addCampgainCharacter,SIGNAL(triggered()),this,SLOT(newCharacter()));
            contextMenu->addAction(addCampgainCharacter);
            QMenu* addEncounterMenu = new QMenu(QString("Add Encounter"), contextMenu);
                QAction* addTextEncounter = new QAction(QString("Add Text Encounter"), addEncounterMenu);
                connect(addTextEncounter,SIGNAL(triggered()),this,SLOT(newTextEncounter()));
                addEncounterMenu->addAction(addTextEncounter);
                QAction* addBattleEncounter = new QAction(QString("Add Battle Encounter"), addEncounterMenu);
                connect(addBattleEncounter,SIGNAL(triggered()),this,SLOT(newBattleEncounter()));
                addEncounterMenu->addAction(addBattleEncounter);
                QAction* addScrollingTextEncounter = new QAction(QString("Add Scrolling Text Encounter"), addEncounterMenu);
                connect(addScrollingTextEncounter,SIGNAL(triggered()),this,SLOT(newScrollingTextEncounter()));
                addEncounterMenu->addAction(addScrollingTextEncounter);
            contextMenu->addMenu(addEncounterMenu);
            QAction* addMap = new QAction(QString("Add Map"), contextMenu);
            connect(addMap,SIGNAL(triggered()),this,SLOT(newMap()));
            contextMenu->addAction(addMap);
            QAction* addNPC = new QAction(QString("Add NPC"), contextMenu);
            connect(addNPC,SIGNAL(triggered()),this,SLOT(newNPC()));
            contextMenu->addAction(addNPC);

            if((type == DMHelper::TreeType_Map)||(type == DMHelper::TreeType_Encounter))
            {
                QAction* startBattleItem = new QAction(QString("Start Battle"), contextMenu);
                connect(startBattleItem,SIGNAL(triggered()),this,SLOT(handleStartNewBattle()));
                contextMenu->addAction(startBattleItem);
            }

            if(type == DMHelper::TreeType_Map)
            {
                QAction* editMapItem = new QAction(QString("Edit Map File"), contextMenu);
                connect(editMapItem,SIGNAL(triggered()),this,SLOT(editCurrentMap()));
                contextMenu->addAction(editMapItem);
            }

            if((item->isEditable())||(type == DMHelper::TreeType_Map))
            {
                QAction* editItem = new QAction(QString("Edit Item"), contextMenu);
                connect(editItem,SIGNAL(triggered()),this,SLOT(editCurrentItem()));
                contextMenu->addAction(editItem);
            }

            if((type == DMHelper::TreeType_Adventure)||(type == DMHelper::TreeType_Encounter)||(type == DMHelper::TreeType_Map)||(type == DMHelper::TreeType_Character))
            {
                QAction* removeItem = new QAction(QString("Remove Item"), contextMenu);
                connect(removeItem,SIGNAL(triggered()),this,SLOT(removeCurrentItem()));
                contextMenu->addAction(removeItem);
            }

            contextMenu->addSeparator();

            QAction* exportItem = new QAction(QString("Export Item..."));
            connect(exportItem, SIGNAL(triggered()), this, SLOT(exportCurrentItem()));
            contextMenu->addAction(exportItem);

            if(contextMenu->actions().count() > 0)
                contextMenu->exec(ui->treeView->mapToGlobal(point));
            delete contextMenu;
        }
    }
}

void MainWindow::handleTreeItemChanged(QStandardItem * item)
{
    if((!item) || (!campaign))
        return;

    qDebug() << "[Main] Tree Item Changed: " << item;

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
            return;

            // Todo: what if the name of the character is changed?
        }
    }

    campaign->endBatchChanges();

}

void MainWindow::handleTreeItemSelected(const QModelIndex & current, const QModelIndex & previous)
{
    // TODO: refactor and abstract (make a deselect and select)
    Q_UNUSED(previous);

    qDebug() << "[Main] Tree Item Selected. Current: " << current << " Previous: " << previous;

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
        }

        ui->stackedWidgetEncounter->setEnabled(false);
    }

    // If the previous index is an map...
    Map* map = mapFromIndex(previous);
    if(map)
    {
        ui->stackedWidgetEncounter->setEnabled(false);
        MapFrame* mapFrame = dynamic_cast<MapFrame*>(ui->stackedWidgetEncounter->widget(DMHelper::EncounterType_Map));
        map->unregisterWindow(mapFrame);
        delete undoAction; undoAction = nullptr;
        delete redoAction; redoAction = nullptr;
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
            EncounterBattleEdit* battleEdit = dynamic_cast<EncounterBattleEdit*>(ui->stackedWidgetEncounter->currentWidget());
            if(battleEdit)
                connect(encounter,SIGNAL(destroyed(QObject*)),battleEdit,SLOT(clear()));
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
        ui->menu_Edit->addAction(undoAction);
        redoAction = mapFrame->getRedoAction(this);
        redoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y));
        ui->menu_Edit->addAction(redoAction);
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
    QStandardItem* item = treeModel->itemFromIndex(index);
    if(!item)
        return;

    int itemType = item->data(DMHelper::TreeItemData_Type).toInt();
    QUuid itemId = QUuid(item->data(DMHelper::TreeItemData_ID).toString());

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

    _battleDlgMgr->startNewBattle(qobject_cast<EncounterBattle*>(encounterFromIndex(ui->treeView->currentIndex())));
}

void MainWindow::handleLoadBattle()
{
    if((!campaign)||(!_battleDlgMgr))
        return;

    _battleDlgMgr->loadBattle(qobject_cast<EncounterBattle*>(encounterFromIndex(ui->treeView->currentIndex())));
}

void MainWindow::handleDeleteBattle()
{
    if((!campaign)||(!_battleDlgMgr))
        return;

    _battleDlgMgr->deleteBattle(qobject_cast<EncounterBattle*>(encounterFromIndex(ui->treeView->currentIndex())));
}

void MainWindow::handleAnimationStarted(QColor color)
{
    _animationFrameCount = DMHelper::ANIMATION_TIMER_PREVIEW_FRAMES;
    pubWindow->setBackgroundColor(color);
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
    qDebug() << "[Main] Opening Bestiary";
    if(!Bestiary::Instance())
        return;

    if(Bestiary::Instance()->count() == 0)
    {
        qDebug() << "[Main]    ...Bestiary is empty, creating a first monster";
        bestiaryDlg.createNewMonster();
    }
    else
    {
        bestiaryDlg.setFocus();
        bestiaryDlg.show();
        bestiaryDlg.activateWindow();
    }
}

void MainWindow::openAboutDialog()
{
    qDebug() << "[Main] Opening About Box";

    AboutDialog dlg;
    dlg.exec();
}

void MainWindow::openTextPublisher()
{
    TextPublishDialog* dlg = new TextPublishDialog(this);

    connect(dlg, SIGNAL(publishImage(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    dlg->show();
    dlg->activateWindow();
}

void MainWindow::openTextTranslator()
{
    TextTranslateDialog* dlg = new TextTranslateDialog(this);

    connect(dlg, SIGNAL(publishImage(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    dlg->show();
    dlg->activateWindow();
}

void MainWindow::openRandomMarkets()
{
    RandomMarketDialog* dlg = new RandomMarketDialog(this);

    dlg->show();
    dlg->activateWindow();
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
