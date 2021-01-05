#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dmversion.h"
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
#include "encounterbattle.h"
#include "encounterscrollingtext.h"
#include "encounterscrollingtextedit.h"
#include "campaignobjectframe.h"
#include "combatant.h"
#include "campaigntreemodel.h"
#include "campaigntreeitem.h"
#include "battleframe.h"
//#include "audioplaybackframe.h"
#include "soundboardframe.h"
#include "audiofactory.h"
#include "monster.h"
#include "monsterclass.h"
#include "bestiary.h"
#include "spell.h"
#include "spellbook.h"
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
#include "quickref.h"
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
#include "ribbontabtools.h"
#include "ribbontabbattlemap.h"
#include "ribbontabbattle.h"
#include "ribbontabscrolling.h"
#include "ribbontabtext.h"
#include "ribbontabmap.h"
#include "ribbontabaudio.h"
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
#include <QShortcut>
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
    _options(nullptr),
    bestiaryDlg(),
    spellDlg(),
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
    _ribbonTabMap(nullptr),
    _ribbonTabBattle(nullptr),
    _ribbonTabScrolling(nullptr),
    _ribbonTabText(nullptr),
    _ribbonTabMiniMap(nullptr),
    _ribbonTabAudio(nullptr)
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
    qDebug() << "[MainWindow]     Expected Spellbook Version: " << QString::number(DMHelper::SPELLBOOK_MAJOR_VERSION) + "." + QString::number(DMHelper::SPELLBOOK_MINOR_VERSION);
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

    qDebug() << "[MainWindow] Reading Settings";
    _options = new OptionsContainer(this);
    MRUHandler* mruHandler = new MRUHandler(nullptr, DEFAULT_MRU_FILE_COUNT, this);
    connect(mruHandler,SIGNAL(triggerMRU(QString)),this,SLOT(openFile(QString)));
    _options->setMRUHandler(mruHandler);
    _options->readSettings();
    connect(_options,SIGNAL(bestiaryFileNameChanged()),this,SLOT(readBestiary()));
    connect(_options,SIGNAL(spellbookFileNameChanged()),this,SLOT(readSpellbook()));
    qDebug() << "[MainWindow] Settings Read";

    // Set the global font
    QFont f = qApp->font();
    f.setFamily(_options->getFontFamily());
    f.setPointSize(_options->getFontSize());
    qDebug() << "[MainWindow] Setting application font to: " << _options->getFontFamily() << " size " << _options->getFontSize();
    qApp->setFont(f);

    ui->setupUi(this);
    if(screen)
    {
        resize(screen->availableSize().width() * 4 / 5, screen->availableSize().height() * 4 / 5);
    }
    setupRibbonBar();

    // Set the MRU menu to the created menu bar
    mruHandler->setActionsMenu(_ribbonTabFile->getMRUMenu());

    // TODO: find out why the following has no effect! There must be something in the frames that has "minimumexpanding" size policy
    //ui->splitter->setStretchFactor(0,2);
    //ui->splitter->setStretchFactor(1,2);

    qDebug() << "[MainWindow] Initializing Bestiary";
    Bestiary::Initialize();
    qDebug() << "[MainWindow] Bestiary Initialized";

    qDebug() << "[MainWindow] Initializing Spellbook";
    Spellbook::Initialize();
    qDebug() << "[MainWindow] Spellbook Initialized";

    qDebug() << "[MainWindow] Initializing BasicDateServer";
    BasicDateServer::Initialize(_options->getCalendarFileName());
    BasicDateServer* dateServer = BasicDateServer::Instance();
    connect(_options, &OptionsContainer::calendarFileNameChanged, dateServer, &BasicDateServer::readDateInformation);
    qDebug() << "[MainWindow] BasicDateServer Initialized";

    qDebug() << "[MainWindow] Initializing EquipmentServer";
    EquipmentServer::Initialize(_options->getEquipmentFileName());
    EquipmentServer* equipmentServer = EquipmentServer::Instance();
    connect(_options, &OptionsContainer::equipmentFileNameChanged, equipmentServer, &EquipmentServer::readEquipment);
    qDebug() << "[MainWindow] BasicDateServer Initialized";

    // File Menu
    connect(_ribbonTabFile, SIGNAL(newClicked()), this, SLOT(newCampaign()));
    connect(_ribbonTabFile, SIGNAL(openClicked()), this, SLOT(openFileDialog()));
    QShortcut* openShortcut = new QShortcut(QKeySequence(tr("Ctrl+O", "Open")), this);
    connect(openShortcut, SIGNAL(activated()), this, SLOT(openFileDialog()));
    connect(_ribbonTabFile, SIGNAL(saveClicked()), this, SLOT(saveCampaign()));
    QShortcut* saveShortcut = new QShortcut(QKeySequence(tr("Ctrl+S", "Save")), this);
    connect(saveShortcut, SIGNAL(activated()), this, SLOT(saveCampaign()));
    connect(_ribbonTabFile, SIGNAL(saveAsClicked()), this, SLOT(saveCampaignAs()));
    connect(_ribbonTabFile, SIGNAL(optionsClicked()), _options, SLOT(editSettings()));
    connect(_ribbonTabFile, SIGNAL(closeClicked()), this, SLOT(closeCampaign()));
    QShortcut* quitShortcut = new QShortcut(QKeySequence(tr("Ctrl+Q", "Quit")), this);
    connect(quitShortcut, SIGNAL(activated()), this, SLOT(close()));
    connect(ui->actionE_xit,SIGNAL(triggered()),this,SLOT(close()));

    // Campaign Menu
    connect(this,SIGNAL(campaignLoaded(Campaign*)),this,SLOT(handleCampaignLoaded(Campaign*)));
    connect(this,SIGNAL(campaignLoaded(Campaign*)),this,SLOT(clearDirty()));
    connect(_ribbonTabCampaign, SIGNAL(newPartyClicked()), this, SLOT(newParty()));
    connect(_ribbonTabCampaign, SIGNAL(newCharacterClicked()), this, SLOT(newCharacter()));
    connect(_ribbonTabCampaign, SIGNAL(newMapClicked()), this, SLOT(newMap()));
    connect(_ribbonTabCampaign, SIGNAL(newTextClicked()), this, SLOT(newTextEncounter()));
    connect(_ribbonTabCampaign, SIGNAL(newBattleClicked()), this, SLOT(newBattleEncounter()));
    connect(_ribbonTabCampaign, SIGNAL(newScrollingTextClicked()), this, SLOT(newScrollingTextEncounter()));
    connect(_ribbonTabCampaign, SIGNAL(newSoundClicked()), this, SLOT(newAudioEntry()));
    connect(_ribbonTabCampaign, SIGNAL(newSyrinscapeClicked()), this, SLOT(newSyrinscapeEntry()));
    connect(_ribbonTabCampaign, SIGNAL(newYoutubeClicked()), this, SLOT(newYoutubeEntry()));
    connect(_ribbonTabCampaign, SIGNAL(removeItemClicked()), this, SLOT(removeCurrentItem()));
    connect(_ribbonTabCampaign, SIGNAL(exportItemClicked()), this, SLOT(exportCurrentItem()));
    connect(_ribbonTabCampaign, SIGNAL(importItemClicked()), this, SLOT(importItem()));
    connect(_ribbonTabCampaign, SIGNAL(importCharacterClicked()), this, SLOT(importCharacter()));
    enableCampaignMenu();

    // Tools Menu
    /*
#ifdef INCLUDE_CHASE_SUPPORT
    connect(ui->action_Chase_Dialog,SIGNAL(triggered()),this,SLOT(startChase()));
#endif
    */
    connect(_ribbonTabTools, SIGNAL(bestiaryClicked()), this, SLOT(openBestiary()));
    QShortcut* bestiaryShortcut = new QShortcut(QKeySequence(tr("Ctrl+M", "Open Bestiary")), this);
    connect(bestiaryShortcut, SIGNAL(activated()), this, SLOT(openBestiary()));
    connect(_ribbonTabTools, SIGNAL(exportBestiaryClicked()), this, SLOT(exportBestiary()));
    connect(_ribbonTabTools, SIGNAL(importBestiaryClicked()), this, SLOT(importBestiary()));
    connect(_ribbonTabTools, SIGNAL(spellbookClicked()), this, SLOT(openSpellbook()));
    connect(_ribbonTabTools, SIGNAL(rollDiceClicked()), this, SLOT(openDiceDialog()));
    QShortcut* diceRollShortcut = new QShortcut(QKeySequence(tr("Ctrl+D", "Roll Dice")), this);
    connect(diceRollShortcut, SIGNAL(activated()), this, SLOT(openDiceDialog()));
    connect(_ribbonTabTools, SIGNAL(publishTextClicked()), this, SLOT(openTextPublisher()));
    connect(_ribbonTabTools, SIGNAL(translateTextClicked()), this, SLOT(openTextTranslator()));
    connect(_ribbonTabTools, SIGNAL(randomMarketClicked()), this, SLOT(openRandomMarkets()));

    // Help Menu
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

    if(ui->treeView->header())
    {
        ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->treeView->header()->setStretchLastSection(false);
    }

    connect(ui->treeView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(handleCustomContextMenu(QPoint)));
    connect(ui->treeView->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(handleTreeItemSelected(QModelIndex,QModelIndex)));
    connect(ui->treeView,SIGNAL(activated(QModelIndex)),this,SLOT(handleTreeItemDoubleClicked(QModelIndex)));
    connect(treeModel, &CampaignTreeModel::campaignChanged, ui->treeView, &CampaignTree::campaignChanged);
    connect(treeModel, &CampaignTreeModel::itemMoved, ui->treeView, &CampaignTree::handleItemMoved);
    connect(treeModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(handleTreeItemChanged(QStandardItem*)));
    qDebug() << "[MainWindow] Tree Model Created";

    connect(Bestiary::Instance(),SIGNAL(changed()),&bestiaryDlg,SLOT(dataChanged()));
    connect(Spellbook::Instance(),SIGNAL(changed()),&spellDlg,SLOT(dataChanged()));

    qDebug() << "[MainWindow] Loading Bestiary";
#ifndef Q_OS_MAC
    splash.showMessage(QString("Initializing Bestiary...\n"), Qt::AlignBottom | Qt::AlignHCenter);
#endif
    qApp->processEvents();
    readBestiary();
    bestiaryDlg.resize(width() * 9 / 10, height() * 9 / 10);
    qDebug() << "[MainWindow] Bestiary Loaded";

    connect(this, SIGNAL(dispatchPublishImage(QImage)), this, SLOT(showPublishWindow()));
    connect(this, SIGNAL(dispatchPublishImage(QImage, QColor)), this, SLOT(showPublishWindow()));
    connect(this, SIGNAL(dispatchPublishImage(QImage)), pubWindow, SLOT(setImage(QImage)));
    connect(this, SIGNAL(dispatchPublishImage(QImage, QColor)), pubWindow, SLOT(setImage(QImage, QColor)));
    connect(this, SIGNAL(dispatchAnimateImage(QImage)), pubWindow, SLOT(setImageNoScale(QImage)));
    connect(this, SIGNAL(dispatchAnimateImage(QImage)), this, SLOT(handleAnimationPreview(QImage)));

    connect(&bestiaryDlg,SIGNAL(publishMonsterImage(QImage, QColor)),this,SIGNAL(dispatchPublishImage(QImage, QColor)));

    qDebug() << "[MainWindow] Loading Spellbook";
#ifndef Q_OS_MAC
    splash.showMessage(QString("Initializing Spellbook...\n"), Qt::AlignBottom | Qt::AlignHCenter);
#endif
    qApp->processEvents();
    readSpellbook();
    spellDlg.resize(width() * 9 / 10, height() * 9 / 10);
    qDebug() << "[MainWindow] Spellbook Loaded";

    // Add the encounter pages to the stacked widget - implicit mapping to EncounterType enum values
    qDebug() << "[MainWindow] Creating Encounter Pages";

    // Empty Campaign Page
    //ui->stackedWidgetEncounter->addFrames(QList<int>({DMHelper::CampaignType_Base,
    //                                                  DMHelper::CampaignType_AudioTrack}), new EmptyCampaignFrame);
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_Base, new EmptyCampaignFrame);

    // EncounterType_Text
    encounterTextEdit = new EncounterTextEdit;
    connect(encounterTextEdit, SIGNAL(anchorClicked(QUrl)), this, SLOT(linkActivated(QUrl)));
    connect(_ribbonTabText, SIGNAL(colorChanged(QColor)), encounterTextEdit, SLOT(setColor(QColor)));
    connect(_ribbonTabText, SIGNAL(fontFamilyChanged(const QString&)), encounterTextEdit, SLOT(setFont(const QString&)));
    connect(_ribbonTabText, SIGNAL(fontSizeChanged(int)), encounterTextEdit, SLOT(setFontSize(int)));
    connect(_ribbonTabText, SIGNAL(fontBoldChanged(bool)), encounterTextEdit, SLOT(setBold(bool)));
    connect(_ribbonTabText, SIGNAL(fontItalicsChanged(bool)), encounterTextEdit, SLOT(setItalics(bool)));
    connect(_ribbonTabText, SIGNAL(alignmentChanged(Qt::Alignment)), encounterTextEdit, SLOT(setAlignment(Qt::Alignment)));
    connect(_ribbonTabText, SIGNAL(hyperlinkClicked()), encounterTextEdit, SLOT(hyperlinkClicked()));
    connect(encounterTextEdit, SIGNAL(colorChanged(QColor)), _ribbonTabText, SLOT(setColor(QColor)));
    connect(encounterTextEdit, SIGNAL(fontFamilyChanged(const QString&)), _ribbonTabText, SLOT(setFontFamily(const QString&)));
    connect(encounterTextEdit, SIGNAL(fontSizeChanged(int)), _ribbonTabText, SLOT(setFontSize(int)));
    connect(encounterTextEdit, SIGNAL(fontBoldChanged(bool)), _ribbonTabText, SLOT(setFontBold(bool)));
    connect(encounterTextEdit, SIGNAL(fontItalicsChanged(bool)), _ribbonTabText, SLOT(setFontItalics(bool)));
    connect(encounterTextEdit, SIGNAL(alignmentChanged(Qt::Alignment)), _ribbonTabText, SLOT(setAlignment(Qt::Alignment)));
    connect(encounterTextEdit, SIGNAL(setHyperlinkActive(bool)), _ribbonTabText, SLOT(setHyperlinkActive(bool)));
    ui->stackedWidgetEncounter->addFrames(QList<int>({DMHelper::CampaignType_Campaign,
                                                      DMHelper::CampaignType_Text,
                                                      DMHelper::CampaignType_Placeholder}), encounterTextEdit);
    qDebug() << "[MainWindow]     Adding Text Encounter widget as page #" << ui->stackedWidgetEncounter->count() - 1;

    // EncounterType_Battle
    BattleFrame* battleFrame = new BattleFrame;
    battleFrame->setShowOnDeck(_options->getShowOnDeck());
    battleFrame->setShowCountdown(_options->getShowCountdown());
    battleFrame->setCountdownDuration(_options->getCountdownDuration());
    battleFrame->setPointerFile(_options->getPointerFile());
    battleFrame->setSelectedIcon(_options->getSelectedIcon());
    battleFrame->setActiveIcon(_options->getActiveIcon());
    battleFrame->setCombatantFrame(_options->getCombatantFrame());
    battleFrame->setCountdownFrame(_options->getCountdownFrame());
    connect(_options, SIGNAL(showOnDeckChanged(bool)), battleFrame, SLOT(setShowOnDeck(bool)));
    connect(_options, SIGNAL(showCountdownChanged(bool)), battleFrame, SLOT(setShowCountdown(bool)));
    connect(_options, SIGNAL(countdownDurationChanged(int)), battleFrame, SLOT(setCountdownDuration(int)));
    connect(_options, SIGNAL(pointerFileNameChanged(const QString&)), battleFrame, SLOT(setPointerFile(const QString&)));
    connect(_options, SIGNAL(selectedIconChanged(const QString&)), battleFrame, SLOT(setSelectedIcon(const QString&)));
    connect(_options, SIGNAL(activeIconChanged(const QString&)), battleFrame, SLOT(setActiveIcon(const QString&)));
    connect(_options, SIGNAL(combatantFrameChanged(const QString&)), battleFrame, SLOT(setCombatantFrame(const QString&)));
    connect(_options, SIGNAL(countdownFrameChanged(const QString&)), battleFrame, SLOT(setCountdownFrame(const QString&)));
    connect(pubWindow, SIGNAL(frameResized(QSize)), battleFrame, SLOT(setTargetSize(QSize)));
    connect(pubWindow, SIGNAL(labelResized(QSize)), battleFrame, SLOT(setTargetLabelSize(QSize)));
    connect(pubWindow, SIGNAL(publishMouseDown(const QPointF&)), battleFrame, SLOT(publishWindowMouseDown(const QPointF&)));
    connect(pubWindow, SIGNAL(publishMouseMove(const QPointF&)), battleFrame, SLOT(publishWindowMouseMove(const QPointF&)));
    connect(pubWindow, SIGNAL(publishMouseRelease(const QPointF&)), battleFrame, SLOT(publishWindowMouseRelease(const QPointF&)));
    connect(battleFrame, SIGNAL(characterSelected(QUuid)), this, SLOT(openCharacter(QUuid)));
    connect(battleFrame, SIGNAL(monsterSelected(QString)), this, SLOT(openMonster(QString)));
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
    connect(_ribbonTabBattle, SIGNAL(addObjectClicked()), battleFrame, SLOT(addObject()));
    connect(_ribbonTabBattle, SIGNAL(castSpellClicked()), battleFrame, SLOT(castSpell()));
    connect(_ribbonTabBattle, SIGNAL(addEffectRadiusClicked()), battleFrame, SLOT(castSpell()));
    connect(_ribbonTabBattle, SIGNAL(addEffectConeClicked()), battleFrame, SLOT(addEffectCone()));
    connect(_ribbonTabBattle, SIGNAL(addEffectCubeClicked()), battleFrame, SLOT(addEffectCube()));
    connect(_ribbonTabBattle, SIGNAL(addEffectLineClicked()), battleFrame, SLOT(addEffectLine()));
    connect(_ribbonTabBattle, SIGNAL(statisticsClicked()), battleFrame, SLOT(showStatistics()));
    QShortcut* nextShortcut = new QShortcut(QKeySequence(tr("Ctrl+N", "Next Combatant")), this);
    connect(nextShortcut, SIGNAL(activated()), battleFrame, SLOT(next()));

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

    connect(this, SIGNAL(cancelSelect()), battleFrame, SLOT(cancelSelect()));

    ui->stackedWidgetEncounter->addFrames(QList<int>({DMHelper::CampaignType_Battle,
                                                      DMHelper::CampaignType_BattleContent}), battleFrame);
    qDebug() << "[MainWindow]     Adding Battle Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;

    // EncounterType_Character
    CharacterFrame* charFrame = new CharacterFrame;
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_Combatant, charFrame);
    qDebug() << "[MainWindow]     Adding Character Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;
    connect(charFrame, SIGNAL(publishCharacterImage(QImage)), this, SIGNAL(dispatchPublishImage(QImage)));

    PartyFrame* partyFrame = new PartyFrame;
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_Party, partyFrame);
    qDebug() << "[MainWindow]     Adding Party Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;
    connect(partyFrame, SIGNAL(publishPartyImage(QImage)), this, SIGNAL(dispatchPublishImage(QImage)));
    connect(this, SIGNAL(characterChanged(QUuid)), partyFrame, SLOT(handleCharacterChanged(QUuid)));
    connect(partyFrame, SIGNAL(characterSelected(QUuid)), this, SLOT(openCharacter(QUuid)));


    // EncounterType_Map
    MapFrame* mapFrame = new MapFrame;
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_Map, mapFrame);
    qDebug() << "[MainWindow]     Adding Map Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;
    connect(mapFrame,SIGNAL(publishImage(QImage)),this,SIGNAL(dispatchPublishImage(QImage)));
    connect(mapFrame, SIGNAL(animateImage(QImage)), this, SIGNAL(dispatchAnimateImage(QImage)));
    connect(mapFrame, SIGNAL(animationStarted()), this, SLOT(handleAnimationStarted()));
    connect(mapFrame, SIGNAL(showPublishWindow()), this, SLOT(showPublishWindow()));
    connect(pubWindow, SIGNAL(frameResized(QSize)), mapFrame, SLOT(targetResized(QSize)));

    connect(_ribbonTabMiniMap, SIGNAL(editFileClicked()), mapFrame, SLOT(editMapFile()));
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
    connect(audioTrackEdit, SIGNAL(trackTypeChanged(int)), _ribbonTabAudio, SLOT(setTrackType(int)));
    connect(_ribbonTabAudio, SIGNAL(playClicked(bool)), audioTrackEdit, SLOT(setPlay(bool)));
    connect(audioTrackEdit, SIGNAL(playChanged(bool)), _ribbonTabAudio, SLOT(setPlay(bool)));
    connect(_ribbonTabAudio, SIGNAL(repeatClicked(bool)), audioTrackEdit, SLOT(setRepeat(bool)));
    connect(audioTrackEdit, SIGNAL(repeatChanged(bool)), _ribbonTabAudio, SLOT(setRepeat(bool)));
    connect(_ribbonTabAudio, SIGNAL(muteClicked(bool)), audioTrackEdit, SLOT(setMute(bool)));
    connect(audioTrackEdit, SIGNAL(muteChanged(bool)), _ribbonTabAudio, SLOT(setMute(bool)));
    connect(_ribbonTabAudio, SIGNAL(volumeChanged(int)), audioTrackEdit, SLOT(setVolume(int)));
    connect(audioTrackEdit, SIGNAL(volumeChanged(int)), _ribbonTabAudio, SLOT(setVolume(int)));

    // EncounterType_WelcomeScreen
    WelcomeFrame* welcomeFrame = new WelcomeFrame(mruHandler);
    connect(welcomeFrame, SIGNAL(openCampaignFile(QString)), this, SLOT(openFile(QString)));
    connect(_ribbonTabFile, SIGNAL(userGuideClicked()), welcomeFrame, SLOT(openUsersGuide()));
    connect(_ribbonTabFile, SIGNAL(gettingStartedClicked()), welcomeFrame, SLOT(openGettingStarted()));
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_WelcomeScreen, welcomeFrame);
    qDebug() << "[MainWindow]     Adding Welcome Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;

    qDebug() << "[MainWindow] Encounter Pages Created";

    // Ensure publishing a single image stops any running animations
    connect(this, SIGNAL(dispatchPublishImage(QImage,QColor)), _ribbon->getPublishRibbon(), SLOT(cancelPublish()));

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
    connect(this,SIGNAL(dispatchPublishImage(QImage)),previewFrame,SLOT(setImage(QImage)));
    connect(this,SIGNAL(dispatchPublishImage(QImage,QColor)),previewFrame,SLOT(setImage(QImage)));
    connect(_options, SIGNAL(pointerFileNameChanged(const QString&)), previewFrame, SLOT(setPointerFile(const QString&)));
    previewFrame->setPointerFile(_options->getPointerFile());
    previewDlg = createDialog(previewFrame, QSize(width() * 9 / 10, height() * 9 / 10));
    connect(_ribbon->getPublishRibbon(), SIGNAL(previewClicked()), previewDlg, SLOT(exec()));
    QShortcut* previewShortcut = new QShortcut(QKeySequence(tr("Ctrl+L", "Preview")), this);
    connect(previewShortcut, SIGNAL(activated()), previewDlg, SLOT(exec()));

    dmScreenDlg = createDialog(new DMScreenTabWidget(_options->getEquipmentFileName(), this), QSize(width() * 9 / 10, height() * 9 / 10));
    tableDlg = createDialog(new CustomTableFrame(_options->getTablesDirectory(), this), QSize(width() * 9 / 10, height() * 9 / 10));

    QuickRef::Initialize();
    QuickRefFrame* quickRefFrame = new QuickRefFrame(this);
    quickRefDlg = createDialog(quickRefFrame, QSize(width() * 3 / 4, height() * 9 / 10));
    connect(_options, &OptionsContainer::quickReferenceFileNameChanged, this, &MainWindow::readQuickRef);
    connect(QuickRef::Instance(), &QuickRef::changed, quickRefFrame, &QuickRefFrame::refreshQuickRef);
    readQuickRef();

    /*
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
    */
    SoundboardFrame* soundboard = new SoundboardFrame(this);
    connect(this, SIGNAL(campaignLoaded(Campaign*)), soundboard, SLOT(setCampaign(Campaign*)));
    connect(this, SIGNAL(audioTrackAdded(AudioTrack*)), soundboard, SLOT(addTrackToTree(AudioTrack*)));
    connect(soundboard, SIGNAL(trackCreated(CampaignObjectBase*)), this, SLOT(addNewObject(CampaignObjectBase*)));
    connect(soundboard, SIGNAL(dirty()), this, SLOT(setDirty()));
    soundDlg = createDialog(soundboard, QSize(width() * 9 / 10, height() * 9 / 10));

    timeAndDateFrame = new TimeAndDateFrame(this);
    calendarDlg = createDialog(timeAndDateFrame, QSize(width() / 2, height() * 9 / 10));
    countdownDlg = createDialog(new CountdownFrame(this));

    connect(_ribbonTabTools, SIGNAL(screenClicked()), dmScreenDlg, SLOT(exec()));
    QShortcut* dmScreenShortcut = new QShortcut(QKeySequence(tr("Ctrl+E", "DM Screen")), this);
    connect(dmScreenShortcut, SIGNAL(activated()), dmScreenDlg, SLOT(exec()));
    connect(_ribbonTabTools, SIGNAL(tablesClicked()), tableDlg, SLOT(show()));
    QShortcut* tablesShortcut = new QShortcut(QKeySequence(tr("Ctrl+T", "Tables")), this);
    connect(tablesShortcut, SIGNAL(activated()), tableDlg, SLOT(exec()));
    connect(_ribbonTabTools, SIGNAL(referenceClicked()), quickRefDlg, SLOT(exec()));
    QShortcut* referenceShortcut = new QShortcut(QKeySequence(tr("Ctrl+R", "Reference")), this);
    connect(referenceShortcut, SIGNAL(activated()), quickRefDlg, SLOT(exec()));
    connect(_ribbonTabTools, SIGNAL(soundboardClicked()), soundDlg, SLOT(exec()));
    QShortcut* soundboardShortcut = new QShortcut(QKeySequence(tr("Ctrl+G", "Soundboard")), this);
    connect(soundboardShortcut, SIGNAL(activated()), soundDlg, SLOT(exec()));
    connect(_ribbonTabTools, SIGNAL(calendarClicked()), calendarDlg, SLOT(exec()));
    QShortcut* calendarShortcut = new QShortcut(QKeySequence(tr("Ctrl+K", "Calendar")), this);
    connect(calendarShortcut, SIGNAL(activated()), calendarDlg, SLOT(exec()));
    connect(_ribbonTabTools, SIGNAL(countdownClicked()), countdownDlg, SLOT(exec()));

    qDebug() << "[MainWindow] Reference Tabs Created";

#ifndef Q_OS_MAC
    splash.showMessage(QString("Preparing DM Helper\n"),Qt::AlignBottom | Qt::AlignHCenter);
#endif
    qApp->processEvents();

    _audioPlayer = new AudioPlayer(this);
    _audioPlayer->setVolume(_options->getAudioVolume());
    //connect(audioTrackEdit, SIGNAL(trackSelected(AudioTrack*)), _audioPlayer, SLOT(playTrack(AudioTrack*)));
    connect(mapFrame, SIGNAL(startTrack(AudioTrack*)), _audioPlayer, SLOT(playTrack(AudioTrack*)));

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
    qDebug() << "[MainWindow] Creating a new character...";

    if(!campaign)
    {
        qDebug() << "[MainWindow] New character cannot be created without a valid campaign open.";
        return;
    }

    bool ok;
    QString characterName = QInputDialog::getText(this,
                                                  QString("New Character"),
                                                  QString("Characters added without a party are treated as NPCs. To add a PC, either create the character directly in a party or drag an NPC into a party.") + QChar::LineFeed + QChar::LineFeed + QString("Enter character name:"),
                                                  QLineEdit::Normal,
                                                  QString(),
                                                  &ok);
    if((!ok) || (characterName.isEmpty()))
    {
        qDebug() << "[MainWindow] New character not created because the character name dialog was cancelled";
        return;
    }

    Character* character = dynamic_cast<Character*>(CombatantFactory().createObject(DMHelper::CampaignType_Combatant, DMHelper::CombatantType_Character, characterName, false));

    if(Bestiary::Instance()->count() > 0)
    {
        QMessageBox::StandardButton monsterQuestion = QMessageBox::question(this,
                                                                            QString("New Character"),
                                                                            QString("Do you want to base this character on a monster from the bestiary?"));

        if(monsterQuestion == QMessageBox::Yes)
        {
            QString monsterName = QInputDialog::getItem(this,
                                                        QString("New Character Monster Selection"),
                                                        QString("Select the monster you would like to base the new chararacter on:"),
                                                        Bestiary::Instance()->getMonsterList(),
                                                        0,
                                                        false,
                                                        &ok);
            if((!ok) || (monsterName.isEmpty()))
            {
                qDebug() << "[MainWindow] New character not created because the select monster dialog was cancelled";
                return;
            }

            MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(monsterName);
            if(!monsterClass)
            {
                qDebug() << "[MainWindow] New character not created because not able to find selected monster: " << monsterName;
                return;
            }

            character->copyMonsterValues(*monsterClass);
        }
    }

    addNewObject(character);
}

void MainWindow::importCharacter()
{
    if(!campaign)
        return;

    CharacterImporter* importer = new CharacterImporter();
    connect(importer, &CharacterImporter::characterCreated, this, &MainWindow::addNewObject);
    connect(importer, &CharacterImporter::characterImported, this, &MainWindow::updateCampaignTree);
    connect(importer, &CharacterImporter::characterImported, this, &MainWindow::openCharacter);
    connect(this, &MainWindow::campaignLoaded, importer, &CharacterImporter::campaignChanged);
    importer->importCharacter(campaign, true);
}

void MainWindow::importItem()
{
    if(!campaign)
        return;

    ObjectImporter importer;
    importer.importObject(*campaign);
}

void MainWindow::newParty()
{
    newEncounter(DMHelper::CampaignType_Party, QString("New Party"), QString("Enter new party name:"));
}

void MainWindow::newTextEncounter()
{
    newEncounter(DMHelper::CampaignType_Text, QString("New Text Entry"), QString("Enter new entry name:"));
}

void MainWindow::newBattleEncounter()
{
    CampaignObjectBase* encounter = newEncounter(DMHelper::CampaignType_Battle, QString("New Battle"), QString("Enter new battle name:"));
    if(!encounter)
        return;

    EncounterBattle* battle = dynamic_cast<EncounterBattle*>(encounter);
    if(battle)
    {
        battle->createBattleDialogModel();
        BattleFrame* battleFrame = dynamic_cast<BattleFrame*>(ui->stackedWidgetEncounter->getCurrentFrame());
        if(battleFrame)
            battleFrame->setBattle(battle);
    }
}

void MainWindow::newScrollingTextEncounter()
{
    newEncounter(DMHelper::CampaignType_ScrollingText, QString("New Scrolling Text"), QString("Enter new scrolling text entry name:"));
}

void MainWindow::newMap()
{
    // TODO: throw a message box when it doesn't work.
    if(!campaign)
        return;

    bool ok = false;
    QString mapName = QInputDialog::getText(this, QString("Enter Map Name"), QString("New Map"), QLineEdit::Normal, QString(), &ok);
    if(!ok)
        return;

    QString filename = QFileDialog::getOpenFileName(this, QString("Select Map Image..."));
    if(filename.isEmpty())
        return;

    Map* map = dynamic_cast<Map*>(MapFactory().createObject(DMHelper::CampaignType_Map, -1, mapName, false));
    if(!map)
        return;

    map->setFileName(filename);

    addNewObject(map);
}

void MainWindow::newAudioEntry()
{
    if(!campaign)
        return;

    addNewAudioObject(QFileDialog::getOpenFileName(this, QString("Select local audio file")));
}

void MainWindow::newSyrinscapeEntry()
{
    if(!campaign)
        return;

    QString syrinscapeInstructions("To add a link to a Syrinscape sound:\n\n1) Hit the '+' key or select ""3rd party app integration"" ENABLE in the settings menu\n2) Little pluses will appear next to all the MOODs and OneShots\n3) Click one of these pluses to copy a URI shortcut to the clipboard\n4) Paste this URI into the text box here:\n");

    bool ok = false;
    QString urlName = QInputDialog::getText(this, QString("Enter Syrinscape Audio URI"), syrinscapeInstructions, QLineEdit::Normal, QString(), &ok);
    if((!ok)||(urlName.isEmpty()))
        return;

    addNewAudioObject(urlName);
}

void MainWindow::newYoutubeEntry()
{
    if(!campaign)
        return;

    QString youtubeInstructions("To add a YouTube video as an audio file, paste the link/URL into the text box here:\n");

    bool ok = false;
    QString urlName = QInputDialog::getText(this, QString("Enter Youtube URL"), youtubeInstructions, QLineEdit::Normal, QString(), &ok);
    if((!ok)||(urlName.isEmpty()))
        return;

    addNewAudioObject(urlName);
}

void MainWindow::removeCurrentItem()
{
    if((!campaign)||(!treeModel))
        return;

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

    qDebug() << "[MainWindow] Removed object from the campaign tree: " << removeObject->getName() << ", ID: " << removeObject->getID();

    delete campaign->removeObject(removeObject->getID());
    updateCampaignTree();
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

        if(map)
        {
            bool ok = false;
            QString mapName = QInputDialog::getText(this, QString("Enter Map Name"), QString("New Map"), QLineEdit::Normal, QString(), &ok);
            if(ok)
                map->setName(mapName);
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

void MainWindow::addNewObject(CampaignObjectBase* newObject)
{
    if(!campaign || !treeModel || !newObject)
        return;

    CampaignObjectBase* currentObject = ui->treeView->currentCampaignObject();
    if(!currentObject)
        currentObject = campaign;

    qDebug() << "[MainWindow] Adding object " << newObject->getName() << " (" << newObject->getID() << "), to object " << currentObject->getName() << " (" << currentObject->getID() << ")";

    currentObject->setExpanded(true);
    currentObject->addObject(newObject);

    updateCampaignTree();

    selectItem(newObject->getID());
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
    qDebug() << "[MainWindow] Check for updates starting.";

    UpdateChecker* checker = new UpdateChecker(*_options, silentUpdate, true, this);
    checker->checkForUpdates();

    qDebug() << "[MainWindow] Check for updates started.";
}

void MainWindow::showPublishWindow(bool visible)
{
    if(visible)
    {
        if(!pubWindow->isVisible())
        {
            pubWindow->show();
        }
        // TODO: do we need this at all?
        // pubWindow->activateWindow();
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
            //QModelIndex index = treeIndexMap.value(linkName);
            //ui->treeView->setCurrentIndex(index);
            selectItem(treeIndexMap.value(linkName));
        }
    }
    else
    {
        QDesktopServices::openUrl(link);
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
    if(bestiaryFileName.isEmpty())
    {
        qDebug() << "[MainWindow] ERROR! No known bestiary found, unable to load bestiary";
        return;
    }

    qDebug() << "[MainWindow] Reading bestiary: " << bestiaryFileName;

    QDomDocument doc("DMHelperBestiaryXML");
    QFile file(bestiaryFileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[MainWindow] Reading bestiary file open failed.";
        QMessageBox::critical(this, QString("Bestiary file open failed"),
                              QString("Unable to open the bestiary file: ") + bestiaryFileName);
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
        qDebug() << "[MainWindow] Error reading bestiary XML content. The XML is probably not valid.";
        qDebug() << errMsg << errRow << errColumn;
        QMessageBox::critical(this, QString("Bestiary file invalid"),
                              QString("Unable to read the bestiary file: ") + bestiaryFileName + QString(", the XML is invalid"));
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "root"))
    {
        qDebug() << "[MainWindow] Bestiary file missing root item";
        QMessageBox::critical(this, QString("Bestiary file invalid"),
                              QString("Unable to read the bestiary file: ") + bestiaryFileName + QString(", the XML does not have the expected root item."));
        return;
    }

    // Bestiary file seems ok, make a backup
    _options->backupFile(bestiaryFileName);

    QFileInfo fileInfo(bestiaryFileName);
    Bestiary::Instance()->setDirectory(fileInfo.absoluteDir());
    Bestiary::Instance()->inputXML(root, false);

    if(!_options->getLastMonster().isEmpty() && Bestiary::Instance()->exists(_options->getLastMonster()))
        bestiaryDlg.setMonster(_options->getLastMonster());
    else
        bestiaryDlg.setMonster(Bestiary::Instance()->getFirstMonsterClass());

    qDebug() << "[MainWindow] Bestiary reading complete.";
}

void MainWindow::readSpellbook()
{
    qDebug() << "[MainWindow] Requested to read Spellbook";

    if(!Spellbook::Instance())
    {
        qDebug() << "[MainWindow] Spellbook instance not found, reading stopped";
        return;
    }

    QString spellbookFileName = _options->getSpellbookFileName();
    if(spellbookFileName.isEmpty())
    {
        qDebug() << "[MainWindow] ERROR! No known spellbook found, unable to load spellbook";
        return;
    }

    qDebug() << "[MainWindow] Reading spellbook: " << spellbookFileName;

    QDomDocument doc("DMHelperSpellbookXML");
    QFile file(spellbookFileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[MainWindow] Reading spellbook file open failed.";
        QMessageBox::critical(this, QString("Spellbook file open failed"),
                              QString("Unable to open the spellbook file: ") + spellbookFileName);
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
        qDebug() << "[MainWindow] Error reading spellbook XML content. The XML is probably not valid.";
        qDebug() << errMsg << errRow << errColumn;
        QMessageBox::critical(this, QString("Spellbook file invalid"),
                              QString("Unable to read the spellbook file: ") + spellbookFileName + QString(", the XML is invalid"));
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "root"))
    {
        qDebug() << "[MainWindow] Spellbook file missing root item";
        QMessageBox::critical(this, QString("Spellbook file invalid"),
                              QString("Unable to read the spellbook file: ") + spellbookFileName + QString(", the XML does not have the expected root item."));
        return;
    }

    // Spellbook file seems ok, make a backup
    _options->backupFile(spellbookFileName);

    QFileInfo fileInfo(spellbookFileName);
    Spellbook::Instance()->setDirectory(fileInfo.absoluteDir());
    Spellbook::Instance()->inputXML(root, false);

    if(!_options->getLastSpell().isEmpty() && Spellbook::Instance()->exists(_options->getLastSpell()))
        spellDlg.setSpell(_options->getLastSpell());
    else
        spellDlg.setSpell(Spellbook::Instance()->getFirstSpell());

    qDebug() << "[MainWindow] Spellbook reading complete.";
}

void MainWindow::readQuickRef()
{
    qDebug() << "[MainWindow] Requested to read Quick Reference";

    if(!QuickRef::Instance())
        QuickRef::Initialize();

    QuickRef::Instance()->readQuickRef(_options->getQuickReferenceFileName());

    qDebug() << "[MainWindow] Quick Reference reading complete.";
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

    writeSpellbook();
    writeBestiary();

    _options->setLastMonster(bestiaryDlg.getMonster() ? bestiaryDlg.getMonster()->getName() : "");
    _options->setLastSpell(spellDlg.getSpell() ? spellDlg.getSpell()->getName() : "");
    _options->writeSettings();

    qApp->quit();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
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
    _ribbonTabTools = new RibbonTabTools(this);
    _ribbon->enableTab(_ribbonTabTools);

    _ribbonTabMap = new RibbonTabBattleMap(this);
    _ribbonTabMap->hide();
    _ribbonTabBattle = new RibbonTabBattle(this);
    _ribbonTabBattle->hide();
    _ribbonTabScrolling = new RibbonTabScrolling(this);
    _ribbonTabScrolling->hide();
    _ribbonTabText = new RibbonTabText(this);
    _ribbonTabText->hide();
    _ribbonTabMiniMap = new RibbonTabMap(this);
    _ribbonTabMiniMap->hide();
    _ribbonTabAudio = new RibbonTabAudio(this);
    _ribbonTabAudio->hide();

    connect(_ribbon->getPublishRibbon(), SIGNAL(playersWindowClicked(bool)), this, SLOT(showPublishWindow(bool)));
    QShortcut* publishShortcut = new QShortcut(QKeySequence(tr("Ctrl+P", "Publish")), this);
    connect(publishShortcut, SIGNAL(activated()), _ribbon, SLOT(clickPublish()));

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
}

bool MainWindow::selectItem(QUuid itemId)
{
    if((treeModel) && (!itemId.isNull()))
    {
        QModelIndex index = treeModel->getObjectIndex(itemId);
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
}

bool MainWindow::selectItem(int itemType, QUuid itemId, QUuid adventureId)
{
    Q_UNUSED(itemType);
    Q_UNUSED(adventureId);

    return selectItem(itemId);
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
    qDebug() << "[MainWindow] Writing Bestiary...";

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

void MainWindow::writeSpellbook()
{
    qDebug() << "[MainWindow] Writing Spellbook...";

    if(!Spellbook::Instance())
    {
        qDebug() << "[MainWindow] Spellbook instance not found, no file written.";
        return;
    }

    if(Spellbook::Instance()->count() <= 0)
    {
        qDebug() << "[MainWindow] Spellbook is empty, no file will be written";
        return;
    }

    QString spellbookFileName = _options->getSpellbookFileName();
    qDebug() << "[MainWindow] Writing Spellbook to " << spellbookFileName;

    if(spellbookFileName.isEmpty())
    {
        spellbookFileName = QFileDialog::getSaveFileName(this, QString("Save Spellbook"), QString(), QString("XML files (*.xml)"));
        if(spellbookFileName.isEmpty())
            return;

        _options->setSpellbookFileName(spellbookFileName);
    }

    QDomDocument doc("DMHelperSpellbookXML");

    QDomElement root = doc.createElement("root");
    doc.appendChild(root);

    QFileInfo fileInfo(spellbookFileName);
    QDir targetDirectory(fileInfo.absoluteDir());
    if(Spellbook::Instance()->outputXML(doc, root, targetDirectory, false) <= 0)
    {
        qDebug() << "[MainWindow] Spellbook output did not find any spells. Aborting writing to file";
        return;
    }

    QString xmlString = doc.toString();

    QFile file(spellbookFileName);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "[MainWindow] Unable to open Spellbook file for writing: " << spellbookFileName;
        qDebug() << "       Error " << file.error() << ": " << file.errorString();
        QFileInfo info(file);
        qDebug() << "       Full filename: " << info.absoluteFilePath();
        spellbookFileName.clear();
        return;
    }

    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    ts << xmlString;

    file.close();

    qDebug() << "[MainWindow] Spellbook file writing complete: " << spellbookFileName;
}

CampaignObjectBase* MainWindow::newEncounter(int encounterType, const QString& dialogTitle, const QString& dialogText)
{
    if((!campaign)||(!treeModel))
        return nullptr;

    bool ok;
    QString encounterName = QInputDialog::getText(this, dialogTitle, dialogText, QLineEdit::Normal, QString(), &ok);
    if(!ok)
        return nullptr;

    CampaignObjectBase* encounter = EncounterFactory().createObject(encounterType, -1, encounterName, false);
    if(!encounter)
        return nullptr;

    addNewObject(encounter);

    return encounter;
}

void MainWindow::addNewAudioObject(const QString& audioFile)
{
    if(!campaign || audioFile.isEmpty())
        return;

    QUrl url(audioFile);
    QFileInfo fileInfo(url.path());
    bool ok = false;
    QString trackName = QInputDialog::getText(this, QString("Enter track name"), QString("New Track Name"), QLineEdit::Normal, fileInfo.baseName(), &ok);
    if((!ok)||(trackName.isEmpty()))
        return;

    AudioTrack* track = AudioFactory().createTrackFromUrl(url, trackName);
    if(!track)
        return;

    addNewObject(track);
    emit audioTrackAdded(track);
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

    // The campaign file seems good, back it up!
    _options->backupFile(campaignFileName);

    selectItem(DMHelper::TreeType_Campaign, QUuid());
    emit campaignLoaded(campaign);

    if(_options->getMRUHandler())
        _options->getMRUHandler()->addMRUFile(filename);
}

void MainWindow::handleCampaignLoaded(Campaign* campaign)
{
    qDebug() << "[MainWindow] Campaign Loaded: " << campaignFileName;

    // don't need this, the setcampaign below should do it: updateCampaignTree();
    updateMapFiles();
    updateClock();

    treeModel->setCampaign(campaign);

    ui->treeView->setMinimumWidth(ui->treeView->sizeHint().width());

    if(campaign)
    {
        QModelIndex firstIndex = treeModel->index(0,0);
        if(firstIndex.isValid())
            ui->treeView->setCurrentIndex(firstIndex); // Activate the first entry in the tree
        else
            ui->stackedWidgetEncounter->setCurrentFrame(DMHelper::CampaignType_Base); // ui->stackedWidgetEncounter->setCurrentIndex(0);
        connect(campaign,SIGNAL(dirty()),this,SLOT(setDirty()));
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
    if(treeModel)
        treeModel->refresh();

    treeIndexMap.clear();
    treeIndexMap = treeModel->getTreeEntryMap();
    encounterTextEdit->setKeys(treeIndexMap.uniqueKeys());
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
    QAction* addBattle = new QAction(QIcon(":/img/data/icon_newbattle.png"), QString("New Combat"), contextMenu);
    connect(addBattle, SIGNAL(triggered()), this, SLOT(newBattleEncounter()));
    contextMenu->addAction(addBattle);

    // New scroll text
    QAction* addScrollingText = new QAction(QIcon(":/img/data/icon_newscrollingtext.png"), QString("New Scrolling Text"), contextMenu);
    connect(addScrollingText, SIGNAL(triggered()), this, SLOT(newScrollingTextEncounter()));
    contextMenu->addAction(addScrollingText);

    contextMenu->addSeparator();

    // New audio file
    QAction* addAudioFile = new QAction(QIcon(":/img/data/icon_newsound.png"), QString("New Sound"), contextMenu);
    connect(addAudioFile, SIGNAL(triggered()), this, SLOT(newAudioEntry()));
    contextMenu->addAction(addAudioFile);

    // New Syrinscape sound
    QAction* addSyrinscape = new QAction(QIcon(":/img/data/icon_newsyrinscape.png"), QString("New Syrinscape Sound"), contextMenu);
    connect(addSyrinscape, SIGNAL(triggered()), this, SLOT(newSyrinscapeEntry()));
    contextMenu->addAction(addSyrinscape);

    // New Youtube sound
    QAction* addYoutube = new QAction(QIcon(":/img/data/icon_newyoutube.png"), QString("New Youtube Sound"), contextMenu);
    connect(addYoutube, SIGNAL(triggered()), this, SLOT(newYoutubeEntry()));
    contextMenu->addAction(addYoutube);

    contextMenu->addSeparator();

    // Remove item
    QAction* removeItem = new QAction(QIcon(":/img/data/icon_remove.png"), QString("Remove Item"), contextMenu);
    connect(removeItem, SIGNAL(triggered()), this, SLOT(removeCurrentItem()));
    contextMenu->addAction(removeItem);

    contextMenu->addSeparator();

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

    CampaignTreeItem* campaignItem = dynamic_cast<CampaignTreeItem*>(item);
    if(!campaignItem)
        return;

    CampaignObjectBase* itemObject = campaignItem->getCampaignItemObject();
    if(!itemObject)
        return;

    qDebug() << "[MainWindow] Tree Item Changed: " << item << ", item name: " << campaignItem->text() << ", object name: " << itemObject->getName();

    if(campaignItem->text() != itemObject->getName())
    {
        itemObject->setName(campaignItem->text());
    }

    if(item->data(DMHelper::TreeItemData_Type).toInt() == DMHelper::TreeType_Character)
    {
        emit characterChanged(QUuid(item->data(DMHelper::TreeItemData_ID).toString()));
    }
}

void MainWindow::handleTreeItemSelected(const QModelIndex & current, const QModelIndex & previous)
{
    // TODO: refactor and abstract (make a deselect and select)
    Q_UNUSED(previous);

    qDebug() << "[MainWindow] Tree Item Selected. Current: " << current << " Previous: " << previous;

    // Deactivate the currently selected object
    deactivateObject();

    // Look for the next object to activate it
    CampaignTreeItem* item = treeModel->campaignItemFromIndex(current);
    CampaignObjectBase* itemObject = nullptr;

    if(item)
    {
        itemObject = item->getCampaignItemObject();
        if(itemObject)
            activateObject(itemObject);
    }

    _ribbonTabCampaign->setAddPCButton((itemObject) &&
                                       ((itemObject->getObjectType() == DMHelper::CampaignType_Party) ||
                                       (itemObject->getParentByType(DMHelper::CampaignType_Party) != nullptr)));
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
}

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

void MainWindow::openSpellbook()
{
    qDebug() << "[MainWindow] Opening Spellbook";
    if(!Spellbook::Instance())
        return;

    if(Spellbook::Instance()->count() == 0)
    {
        qDebug() << "[MainWindow]    ...Spellbook is empty, creating a first spell";
        spellDlg.createNewSpell();
    }
    else
    {
        spellDlg.setFocus();
        spellDlg.show();
        spellDlg.activateWindow();
    }
}

void MainWindow::exportSpellbook()
{
    // TODO: add import/export for spells
}

void MainWindow::importSpellbook()
{
    // TODO: add import/export for spells
}

void MainWindow::openAboutDialog()
{
    qDebug() << "[MainWindow] Opening About Box";

    AboutDialog dlg;
    dlg.resize(qMax(dlg.width(), width() * 3 / 4), qMax(dlg.height(), height() * 3 / 4));
    dlg.exec();
}

void MainWindow::openTextPublisher()
{
    TextPublishDialog dlg;
    connect(&dlg, SIGNAL(publishImage(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    dlg.exec();
}

void MainWindow::openTextTranslator()
{
    TextTranslateDialog dlg;
    connect(&dlg, SIGNAL(publishImage(QImage, QColor)), this, SIGNAL(dispatchPublishImage(QImage, QColor)));
    dlg.exec();
}

void MainWindow::openRandomMarkets()
{
    RandomMarketDialog dlg(_options->getShopsFileName());
    dlg.exec();
}

QDialog* MainWindow::createDialog(QWidget* contents, const QSize& dlgSize)
{
    QDialog* resultDlg = new QDialog();
    if(!dlgSize.isNull())
        resultDlg->resize(dlgSize);

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
            objectFrame->setRotation(_ribbon->getPublishRibbon()->getRotation());
        }
    }
}

void MainWindow::setRibbonToType(int objectType)
{
    switch(objectType)
    {
        case DMHelper::CampaignType_Battle:
        case DMHelper::CampaignType_BattleContent:
            _ribbon->enableTab(_ribbonTabMap);
            _ribbon->enableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabMiniMap);
            _ribbon->disableTab(_ribbonTabScrolling);
            _ribbon->disableTab(_ribbonTabText);
            _ribbon->disableTab(_ribbonTabAudio);
            break;
        case DMHelper::CampaignType_Map:
            _ribbon->enableTab(_ribbonTabMiniMap);
            _ribbon->disableTab(_ribbonTabMap);
            _ribbon->disableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabScrolling);
            _ribbon->disableTab(_ribbonTabText);
            _ribbon->disableTab(_ribbonTabAudio);
            break;
        case DMHelper::CampaignType_ScrollingText:
            _ribbon->enableTab(_ribbonTabScrolling);
            _ribbon->disableTab(_ribbonTabMap);
            _ribbon->disableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabMiniMap);
            _ribbon->disableTab(_ribbonTabText);
            _ribbon->disableTab(_ribbonTabAudio);
            connect(_ribbon->getPublishRibbon(), SIGNAL(clicked(bool)), _scrollingTextEdit, SLOT(publishClicked(bool)));
            connect(_ribbon->getPublishRibbon(), SIGNAL(rotationChanged(int)), _scrollingTextEdit, SLOT(setRotation(int)));
            break;
        case DMHelper::CampaignType_Campaign:
        case DMHelper::CampaignType_Text:
            _ribbon->enableTab(_ribbonTabText);
            _ribbon->disableTab(_ribbonTabMap);
            _ribbon->disableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabMiniMap);
            _ribbon->disableTab(_ribbonTabScrolling);
            _ribbon->disableTab(_ribbonTabAudio);
            break;
        case DMHelper::CampaignType_AudioTrack:
            _ribbon->enableTab(_ribbonTabAudio);
            _ribbon->disableTab(_ribbonTabMap);
            _ribbon->disableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabMiniMap);
            _ribbon->disableTab(_ribbonTabScrolling);
            _ribbon->disableTab(_ribbonTabText);
            break;
        case DMHelper::CampaignType_Party:
        case DMHelper::CampaignType_Placeholder:
        case DMHelper::CampaignType_Base:
        case DMHelper::CampaignType_WelcomeScreen:
        case DMHelper::CampaignType_Combatant:
        default:
            _ribbon->disableTab(_ribbonTabMap);
            _ribbon->disableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabMiniMap);
            _ribbon->disableTab(_ribbonTabScrolling);
            _ribbon->disableTab(_ribbonTabText);
            _ribbon->disableTab(_ribbonTabAudio);
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
