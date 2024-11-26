#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dmversion.h"
#include "publishwindow.h"
#include "dicerolldialog.h"
#include "countdownframe.h"
#include "globalsearchframe.h"
#include "party.h"
#include "characterv2.h"
#include "characterimporter.h"
#include "characterv2converter.h"
#include "objectimportdialog.h"
#include "partyframe.h"
#include "characterframe.h"
#include "charactertemplateframe.h"
#include "campaign.h"
#include "combatantfactory.h"
#include "campaignobjectfactory.h"
#include "map.h"
#include "mapfactory.h"
#include "mapframe.h"
#include "battleframemapdrawer.h"
#include "mruhandler.h"
#include "encounterfactory.h"
#include "emptycampaignframe.h"
#include "encountertextedit.h"
#include "encountertextlinked.h"
#include "encounterbattle.h"
#include "campaignobjectframe.h"
#include "campaigntreemodel.h"
#include "campaigntreeitem.h"
#include "battleframe.h"
#include "soundboardframe.h"
#include "audiofactory.h"
#include "monsterclass.h"
#include "bestiary.h"
#include "spell.h"
#include "spellbook.h"
#include "campaignnotesdialog.h"
#include "bestiaryexportdialog.h"
#include "exportdialog.h"
#include "equipmentserver.h"
#include "rulefactory.h"
#include "randommarketdialog.h"
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
#include "helpdialog.h"
#include "dmhlogger.h"
#include "newcampaigndialog.h"
#include "basicdateserver.h"
#include "welcomeframe.h"
#include "customtableframe.h"
#include "legaldialog.h"
#include "updatechecker.h"
#include "ribbonmain.h"
#include "battledialogmodel.h"
#include "ribbontabfile.h"
#include "ribbontabcampaign.h"
#include "ribbontabtools.h"
#include "ribbontabbattlemap.h"
#include "ribbontabbattleview.h"
#include "ribbontabbattle.h"
#include "ribbontabtext.h"
#include "ribbontabmap.h"
#include "ribbontabworldmap.h"
#include "ribbontabaudio.h"
#include "dmhcache.h"
#include "dmh_vlc.h"
#include "dmh_opengl.h"
#include "whatsnewdialog.h"
#include "configurelockedgriddialog.h"
#include "layerimage.h"
#include "layerfow.h"
#include "layervideo.h"
#include "layergrid.h"
#include "layertokens.h"
#include "layerreference.h"
#include "layerblank.h"
#include "mapselectdialog.h"
#include "mapblankdialog.h"
#include "battledialogmodelcharacter.h"
#include <QResizeEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QMimeDatabase>
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
#include <QFontDatabase>
#include <QSurfaceFormat>
#include <QTimer>
#ifndef Q_OS_MAC
#include <QSplashScreen>
#endif

const int AUTOSAVE_TIMER_INTERVAL = 60000; // 1 minute

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _pubWindow(nullptr),
    _dmScreenDlg(nullptr),
    _tableDlg(nullptr),
    _quickRefFrame(nullptr),
    _quickRefDlg(nullptr),
    _soundDlg(nullptr),
    _timeAndDateFrame(nullptr),
    _calendarDlg(nullptr),
    _countdownDlg(nullptr),
    _globalSearchFrame(nullptr),
    _globalSearchDlg(nullptr),
    _encounterTextEdit(nullptr),
    _treeModel(nullptr),
    _activeItems(nullptr),
    _characterLayout(nullptr),
    _campaign(nullptr),
    _campaignFileName(),
    _autoSaveTimer(nullptr),
    _options(nullptr),
    _bestiaryDlg(),
    _spellDlg(),
    _battleDlgMgr(nullptr),
    _audioPlayer(nullptr),
#ifdef INCLUDE_NETWORK_SUPPORT
    _networkController(nullptr),
#endif
    _mouseDown(false),
    _mouseDownPos(),
    _undoAction(nullptr),
    _redoAction(nullptr),
    _recoveryMode(true),
    _initialized(false),
    _dirty(false),
    _animationFrameCount(DMHelper::ANIMATION_TIMER_PREVIEW_FRAMES),
    _ribbon(nullptr),
    _ribbonTabFile(nullptr),
    _ribbonTabCampaign(nullptr),
    _ribbonTabTools(nullptr),
    _ribbonTabBattleMap(nullptr),
    _ribbonTabBattleView(nullptr),
    _ribbonTabBattle(nullptr),
    _ribbonTabText(nullptr),
    _ribbonTabMap(nullptr),
    _ribbonTabWorldMap(nullptr),
    _ribbonTabAudio(nullptr),
    _battleFrame(nullptr),
    _mapFrame(nullptr),
    _characterFrame(nullptr)
{
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

    DMHCache cache;
    cache.ensureCacheExists();
    qDebug() << "[MainWindow]     Cache Directory: " << cache.getCachePath();

    qDebug() << "[MainWindow] Qt Information";
    qDebug() << "[MainWindow]     Qt Version: " << QLibraryInfo::version().toString();
    qDebug() << "[MainWindow]     Is Debug? " << QLibraryInfo::isDebugBuild();
    qDebug() << "[MainWindow]     PrefixPath: " << QLibraryInfo::path(QLibraryInfo::PrefixPath);
    qDebug() << "[MainWindow]     DocumentationPath: " << QLibraryInfo::path(QLibraryInfo::DocumentationPath);
    qDebug() << "[MainWindow]     HeadersPath: " << QLibraryInfo::path(QLibraryInfo::HeadersPath);
    qDebug() << "[MainWindow]     LibrariesPath: " << QLibraryInfo::path(QLibraryInfo::LibrariesPath);
    qDebug() << "[MainWindow]     LibraryExecutablesPath: " << QLibraryInfo::path(QLibraryInfo::LibraryExecutablesPath);
    qDebug() << "[MainWindow]     BinariesPath: " << QLibraryInfo::path(QLibraryInfo::BinariesPath);
    qDebug() << "[MainWindow]     PluginsPath: " << QLibraryInfo::path(QLibraryInfo::PluginsPath);
    qDebug() << "[MainWindow]     QmlImportsPath: " << QLibraryInfo::path(QLibraryInfo::QmlImportsPath);
    qDebug() << "[MainWindow]     Qml2ImportsPath: " << QLibraryInfo::path(QLibraryInfo::Qml2ImportsPath);
    qDebug() << "[MainWindow]     ArchDataPath: " << QLibraryInfo::path(QLibraryInfo::ArchDataPath);
    qDebug() << "[MainWindow]     DataPath: " << QLibraryInfo::path(QLibraryInfo::DataPath);
    qDebug() << "[MainWindow]     TranslationsPath: " << QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    qDebug() << "[MainWindow]     ExamplesPath: " << QLibraryInfo::path(QLibraryInfo::ExamplesPath);
    qDebug() << "[MainWindow]     TestsPath: " << QLibraryInfo::path(QLibraryInfo::TestsPath);
    qDebug() << "[MainWindow]     SettingsPath: " << QLibraryInfo::path(QLibraryInfo::SettingsPath);

    QSurfaceFormat fmt;
    qDebug() << "[MainWindow] OpenGL Information";
    qDebug() << "[MainWindow]     Version: " << fmt.majorVersion() << "." << fmt.minorVersion();
    qDebug() << "[MainWindow]     Device pixel ratio: " << this->devicePixelRatio();

    qDebug() << "[MainWindow] Standard Path Information";
    qDebug() << "[MainWindow]     DocumentsLocation: " << (QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).constFirst());
    qDebug() << "[MainWindow]     ApplicationsLocation: " << (QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).constFirst());
    qDebug() << "[MainWindow]     RuntimeLocation: " << (QStandardPaths::standardLocations(QStandardPaths::RuntimeLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::RuntimeLocation).constFirst());
    qDebug() << "[MainWindow]     ConfigLocation: " << (QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).constFirst());
    qDebug() << "[MainWindow]     AppDataLocation: " << (QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst());
    qDebug() << "[MainWindow]     AppLocalDataLocation: " << (QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).constFirst());

    qDebug() << "[MainWindow] Registering application fonts";
    QFontDatabase::addApplicationFont(":/img/data/fonts/Rellanic-Agx7.ttf");
    QFontDatabase::addApplicationFont(":/img/data/fonts/Davek-vGXA.ttf");
    QFontDatabase::addApplicationFont(":/img/data/fonts/Iokharic-dqvK.ttf");

    qDebug() << "[MainWindow] Reading Settings";
    _options = new OptionsContainer(this);
    MRUHandler* mruHandler = new MRUHandler(nullptr, DEFAULT_MRU_FILE_COUNT, this);
    connect(mruHandler, SIGNAL(triggerMRU(QString)), this, SLOT(openCampaign(QString)));
    _options->setMRUHandler(mruHandler);
    _options->readSettings();
    _recoveryMode = _options->isLoading();
    qDebug() << "[MainWindow] Recovery Mode: " << _recoveryMode;
    _options->setLoading(true);

    connect(_options, SIGNAL(bestiaryFileNameChanged()), this, SLOT(readBestiary()));
    connect(_options, SIGNAL(spellbookFileNameChanged()), this, SLOT(readSpellbook()));
    qDebug() << "[MainWindow] Settings Read";

    // Set the global font
    QFont f = qApp->font();
    f.setFamily(_options->getFontFamily());
    f.setPointSize(_options->getFontSize());
    qDebug() << "[MainWindow] Setting application font to: " << _options->getFontFamily() << " size " << _options->getFontSize();
    qApp->setFont(f);

    connect(_options, &OptionsContainer::autoSaveChanged, this, &MainWindow::handleAutoSaveChanged);
    connect(this, &MainWindow::campaignLoaded, this, &MainWindow::handleAutoSaveChanged);

    DMH_DEBUG_OPENGL_Singleton::Initialize();

    QScreen* screen = QGuiApplication::primaryScreen();

#ifndef Q_OS_MAC
    // Run the splash screen as soon as the application font has been selected to avoid font-change-popping
    QPixmap pixmap(":/img/data/dmhelper_opaque.png");
    QSize screenSize = screen != nullptr ? screen->availableSize() : QSize(1000, 1000);
    QSplashScreen splash(pixmap.scaled(screenSize.width() / 2, screenSize.height() / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    splash.show();
    splash.showMessage(QString("Initializing DMHelper\n"), Qt::AlignBottom | Qt::AlignHCenter);
#endif

    QImageReader::setAllocationLimit(0);

    ui->setupUi(this);
    if(screen)
    {
        resize(screen->availableSize().width() * 4 / 5, screen->availableSize().height() * 4 / 5);
    }
    setupRibbonBar();

    // Set the MRU menu to the created menu bar
    mruHandler->setActionsMenu(_ribbonTabFile->getMRUMenu());

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

    qDebug() << "[MainWindow] Initializing Rule Factory";
    RuleFactory::Initialize(_options->getRulesetFileName());
    RuleFactory* ruleFactory = RuleFactory::Instance();
    connect(_options, &OptionsContainer::rulesetFileNameChanged, ruleFactory, &RuleFactory::readRuleset);
    qDebug() << "[MainWindow] Rule Factory Initialized";

    qDebug() << "[MainWindow] Initializing EquipmentServer";
    EquipmentServer::Initialize(_options->getEquipmentFileName());
    EquipmentServer* equipmentServer = EquipmentServer::Instance();
    connect(_options, &OptionsContainer::equipmentFileNameChanged, equipmentServer, &EquipmentServer::readEquipment);
    qDebug() << "[MainWindow] EquipmentServer Initialized";

    // File Menu
    connect(_ribbonTabFile, SIGNAL(newClicked()), this, SLOT(newCampaign()));
    connect(_ribbonTabFile, SIGNAL(openClicked()), this, SLOT(openFileDialog()));
    QShortcut* openShortcut = new QShortcut(QKeySequence(tr("Ctrl+O", "Open")), this);
    connect(openShortcut, SIGNAL(activated()), this, SLOT(openFileDialog()));
    connect(_ribbonTabFile, &RibbonTabFile::saveClicked, this, &MainWindow::saveCampaign);
    QShortcut* saveShortcut = new QShortcut(QKeySequence(tr("Ctrl+S", "Save")), this);
    connect(saveShortcut, &QShortcut::activated, this, &MainWindow::saveCampaign);
    connect(_ribbonTabFile, SIGNAL(saveAsClicked()), this, SLOT(saveCampaignAs()));
    connect(_ribbonTabFile, &RibbonTabFile::optionsClicked, this, &MainWindow::handleEditSettings);
    connect(_ribbonTabFile, SIGNAL(closeClicked()), this, SLOT(closeCampaign()));
    QShortcut* quitShortcut = new QShortcut(QKeySequence(tr("Ctrl+Q", "Quit")), this);
    connect(quitShortcut, SIGNAL(activated()), this, SLOT(close()));
    connect(ui->actionE_xit, SIGNAL(triggered()), this, SLOT(close()));

    // Campaign Menu
    connect(this, SIGNAL(campaignLoaded(Campaign*)), this, SLOT(handleCampaignLoaded(Campaign*)));
    connect(_ribbonTabCampaign, SIGNAL(newPartyClicked()), this, SLOT(newParty()));
    connect(_ribbonTabCampaign, SIGNAL(newCharacterClicked()), this, SLOT(newCharacter()));
    connect(_ribbonTabCampaign, SIGNAL(newMapClicked()), this, SLOT(newMap()));
    connect(_ribbonTabCampaign, SIGNAL(newMediaClicked()), this, SLOT(newMedia()));
    connect(_ribbonTabCampaign, SIGNAL(newTextClicked()), this, SLOT(newTextEncounter()));
    connect(_ribbonTabCampaign, SIGNAL(newLinkedClicked()), this, SLOT(newLinkedText()));
    connect(_ribbonTabCampaign, SIGNAL(newBattleClicked()), this, SLOT(newBattleEncounter()));
    connect(_ribbonTabCampaign, SIGNAL(newSoundClicked()), this, SLOT(newAudioEntry()));
    connect(_ribbonTabCampaign, SIGNAL(newSyrinscapeClicked()), this, SLOT(newSyrinscapeEntry()));
    connect(_ribbonTabCampaign, SIGNAL(newSyrinscapeOnlineClicked()), this, SLOT(newSyrinscapeOnlineEntry()));
    connect(_ribbonTabCampaign, SIGNAL(newYoutubeClicked()), this, SLOT(newYoutubeEntry()));
    connect(_ribbonTabCampaign, SIGNAL(removeItemClicked()), this, SLOT(removeCurrentItem()));
    connect(_ribbonTabCampaign, SIGNAL(showNotesClicked()), this, SLOT(showNotes()));
    QShortcut* notesShortcut = new QShortcut(QKeySequence(tr("Ctrl+Alt+N", "Add Note")), this);
    connect(notesShortcut, SIGNAL(activated()), this, SLOT(addNote()));
    connect(_ribbonTabCampaign, SIGNAL(exportItemClicked()), this, SLOT(exportCurrentItem()));
    connect(_ribbonTabCampaign, SIGNAL(importItemClicked()), this, SLOT(importItem()));
    connect(_ribbonTabCampaign, SIGNAL(importCharacterClicked()), this, SLOT(importCharacter()));
    enableCampaignMenu();

    // Tools Menu
    connect(_ribbonTabTools, SIGNAL(bestiaryClicked()), this, SLOT(openBestiary()));
    QShortcut* bestiaryShortcut = new QShortcut(QKeySequence(tr("Ctrl+M", "Open Bestiary")), this);
    connect(bestiaryShortcut, SIGNAL(activated()), this, SLOT(openBestiary()));
    connect(_ribbonTabTools, SIGNAL(exportBestiaryClicked()), this, SLOT(exportBestiary()));
    connect(_ribbonTabTools, SIGNAL(importBestiaryClicked()), this, SLOT(importBestiary()));
    connect(_ribbonTabTools, SIGNAL(spellbookClicked()), this, SLOT(openSpellbook()));
    QShortcut* openSpellbookShortcut = new QShortcut(QKeySequence(tr("Ctrl+H", "Open Spellbook")), this);
    connect(openSpellbookShortcut, SIGNAL(activated()), this, SLOT(openSpellbook()));
    connect(_ribbonTabTools, SIGNAL(rollDiceClicked()), this, SLOT(openDiceDialog()));
    QShortcut* diceRollShortcut = new QShortcut(QKeySequence(tr("Ctrl+D", "Roll Dice")), this);
    connect(diceRollShortcut, SIGNAL(activated()), this, SLOT(openDiceDialog()));
    connect(_ribbonTabTools, SIGNAL(randomMarketClicked()), this, SLOT(openRandomMarkets()));
    _ribbonTabTools->setGridLocked(_options->getGridLocked());
    connect(_ribbonTabTools, &RibbonTabTools::lockGridClicked, _options, &OptionsContainer::setGridLocked);
    connect(_ribbonTabTools, &RibbonTabTools::configureGridClicked, this, &MainWindow::configureGridLock);

    // Help Menu
    connect(_ribbonTabFile, SIGNAL(checkForUpdatesClicked()), this, SLOT(checkForUpdates()));
    connect(_ribbonTabFile, SIGNAL(aboutClicked()), this, SLOT(openAboutDialog()));
    connect(_ribbonTabFile, SIGNAL(helpClicked()), this, SLOT(openHelpDialog()));
    connect(ui->treeView, SIGNAL(expanded(QModelIndex)), this, SLOT(handleTreeItemExpanded(QModelIndex)));
    connect(ui->treeView, SIGNAL(collapsed(QModelIndex)), this, SLOT(handleTreeItemCollapsed(QModelIndex)));

    // Battle Menu
    // connections set up elsewhere

    // Battle Map Menu

    // Battle View Menu
    connect(_options, SIGNAL(pointerFileNameChanged(QString)), _ribbonTabBattleView, SLOT(setPointerFile(QString)));
    _ribbonTabBattleView->setPointerFile(_options->getPointerFile());

    // Mini Map Menu
    // connections set up elsewhere

    // Text Menu
    // connections set up elsewhere

    qDebug() << "[MainWindow] Creating Player's Window";
    _pubWindow = new PublishWindow(QString("DMHelper Player's Window"));
    _pubWindow->resize(width() * 9 / 10, height() * 9 / 10);
    connect(_pubWindow, SIGNAL(windowVisible(bool)), _ribbon->getPublishRibbon(), SLOT(setPlayersWindow(bool)));
    connect(_ribbon->getPublishRibbon(), SIGNAL(colorChanged(const QColor&)), _pubWindow, SLOT(setBackgroundColor(const QColor&)));
    qDebug() << "[MainWindow] Player's Window Created";

    qDebug() << "[MainWindow] Creating Tree Model";
    ui->treeView->setHeaderHidden(true);
    _treeModel = new CampaignTreeModel(ui->treeView);
    ui->treeView->setModel(_treeModel);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);

    if(ui->treeView->header())
    {
        ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->treeView->header()->setStretchLastSection(false);
    }

    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(handleCustomContextMenu(QPoint)));
    connect(ui->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(handleTreeItemSelected(QModelIndex, QModelIndex)));
    connect(ui->treeView, SIGNAL(activated(QModelIndex)), this, SLOT(handleTreeItemDoubleClicked(QModelIndex)));
    connect(_treeModel, &CampaignTreeModel::campaignChanged, ui->treeView, &CampaignTree::campaignChanged);
    connect(_treeModel, &CampaignTreeModel::itemMoved, ui->treeView, &CampaignTree::handleItemMoved);
    connect(_treeModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(handleTreeItemChanged(QStandardItem*)));
    qDebug() << "[MainWindow] Tree Model Created";

    qDebug() << "[MainWindow] Loading Bestiary";
#ifndef Q_OS_MAC
    splash.showMessage(QString("Initializing Bestiary...\n"), Qt::AlignBottom | Qt::AlignHCenter);
#endif
    qApp->processEvents();
    readBestiary();
    _bestiaryDlg.setOptions(_options);
    _bestiaryDlg.resize(width() * 9 / 10, height() * 9 / 10);
    qDebug() << "[MainWindow] Bestiary Loaded";

    connect(this, SIGNAL(dispatchPublishImage(QImage)), this, SLOT(showPublishWindow()));
    connect(this, SIGNAL(dispatchPublishImage(QImage, const QColor&)), this, SLOT(showPublishWindow()));
    connect(this, SIGNAL(dispatchPublishImage(QImage)), _pubWindow, SLOT(setImage(QImage)));
    connect(this, SIGNAL(dispatchPublishImage(QImage, const QColor&)), _pubWindow, SLOT(setImage(QImage, const QColor&)));

    connect(&_bestiaryDlg, SIGNAL(publishMonsterImage(QImage, const QColor&)), _ribbon->getPublishRibbon(), SLOT(cancelPublish()));
    connect(&_bestiaryDlg, SIGNAL(publishMonsterImage(QImage, const QColor&)), this, SIGNAL(dispatchPublishImage(QImage, const QColor&)));
    connect(&_bestiaryDlg, &BestiaryDialog::dialogClosed, this, &MainWindow::writeBestiary);

    qDebug() << "[MainWindow] Loading Spellbook";
#ifndef Q_OS_MAC
    splash.showMessage(QString("Initializing Spellbook...\n"), Qt::AlignBottom | Qt::AlignHCenter);
#endif
    qApp->processEvents();
    readSpellbook();
    _spellDlg.resize(width() * 9 / 10, height() * 9 / 10);
    qDebug() << "[MainWindow] Spellbook Loaded";

    connect(&_spellDlg, &SpellbookDialog::dialogClosed, this, &MainWindow::writeSpellbook);

    // Add the encounter pages to the stacked widget - implicit mapping to EncounterType enum values
    qDebug() << "[MainWindow] Creating Encounter Pages";

    // Empty Campaign Page
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_Base, new EmptyCampaignFrame);

    // EncounterType_Text
    _encounterTextEdit = new EncounterTextEdit;
    connect(_encounterTextEdit, SIGNAL(anchorClicked(QUrl)), this, SLOT(linkActivated(QUrl)));
    connect(_encounterTextEdit, SIGNAL(publishImage(QImage)), this, SIGNAL(dispatchPublishImage(QImage)));
    connect(_encounterTextEdit, SIGNAL(showPublishWindow()), this, SLOT(showPublishWindow()));
    connect(_encounterTextEdit, SIGNAL(registerRenderer(PublishGLRenderer*)), _pubWindow, SLOT(setRenderer(PublishGLRenderer*)));
    connect(_pubWindow, SIGNAL(frameResized(QSize)), _encounterTextEdit, SLOT(targetResized(QSize)));
    connect(_ribbonTabText, SIGNAL(animationClicked(bool)), _encounterTextEdit, SLOT(setAnimated(bool)));
    connect(_ribbonTabText, SIGNAL(speedChanged(int)), _encounterTextEdit, SLOT(setScrollSpeed(int)));
    connect(_ribbonTabText, SIGNAL(widthChanged(int)), _encounterTextEdit, SLOT(setTextWidth(int)));
    connect(_ribbonTabText, SIGNAL(rewindClicked()), _encounterTextEdit, SLOT(rewind()));
    connect(_ribbonTabText, &RibbonTabText::playPauseClicked, _encounterTextEdit, &EncounterTextEdit::playPause);
    connect(_encounterTextEdit, &EncounterTextEdit::playPauseChanged, _ribbonTabText, &RibbonTabText::setPlaying);
    connect(_encounterTextEdit, SIGNAL(animatedChanged(bool)), _ribbonTabText, SLOT(setAnimation(bool)));
    connect(_encounterTextEdit, SIGNAL(scrollSpeedChanged(int)), _ribbonTabText, SLOT(setSpeed(int)));
    connect(_encounterTextEdit, SIGNAL(textWidthChanged(int)), _ribbonTabText, SLOT(setWidth(int)));
    connect(_ribbonTabText, SIGNAL(colorChanged(const QColor&)), _encounterTextEdit, SLOT(setColor(const QColor&)));
    connect(_ribbonTabText, SIGNAL(fontFamilyChanged(const QString&)), _encounterTextEdit, SLOT(setFont(const QString&)));
    connect(_ribbonTabText, SIGNAL(fontSizeChanged(int)), _encounterTextEdit, SLOT(setFontSize(int)));
    connect(_ribbonTabText, SIGNAL(fontBoldChanged(bool)), _encounterTextEdit, SLOT(setBold(bool)));
    connect(_ribbonTabText, SIGNAL(fontItalicsChanged(bool)), _encounterTextEdit, SLOT(setItalics(bool)));
    connect(_ribbonTabText, SIGNAL(fontUnderlineChanged(bool)), _encounterTextEdit, SLOT(setUnderline(bool)));
    connect(_ribbonTabText, SIGNAL(alignmentChanged(Qt::Alignment)), _encounterTextEdit, SLOT(setAlignment(Qt::Alignment)));
    connect(_ribbonTabText, SIGNAL(pasteRichChanged(bool)), _encounterTextEdit, SLOT(setPasteRich(bool)));
    connect(_ribbonTabText, SIGNAL(pasteRichChanged(bool)), _options, SLOT(setPasteRich(bool)));
    _ribbonTabText->setPasteRich(_options->getPasteRich());
    connect(_ribbonTabText, SIGNAL(hyperlinkClicked()), _encounterTextEdit, SLOT(hyperlinkClicked()));
    connect(_ribbonTabText, SIGNAL(translateTextClicked(bool)), _encounterTextEdit, SLOT(setTranslated(bool)));
    connect(_ribbonTabText, SIGNAL(codeViewClicked(bool)), _encounterTextEdit, SLOT(setCodeView(bool)));
    //translate - both directions, get rid of previous button & link, make dialog bigger, better, apply, clear
    // remove text publish dialog
    connect(_encounterTextEdit, SIGNAL(colorChanged(const QColor&)), _ribbonTabText, SLOT(setColor(const QColor&)));
    connect(_encounterTextEdit, SIGNAL(fontFamilyChanged(const QString&)), _ribbonTabText, SLOT(setFontFamily(const QString&)));
    connect(_encounterTextEdit, SIGNAL(fontSizeChanged(int)), _ribbonTabText, SLOT(setFontSize(int)));
    connect(_encounterTextEdit, SIGNAL(fontBoldChanged(bool)), _ribbonTabText, SLOT(setFontBold(bool)));
    connect(_encounterTextEdit, SIGNAL(fontItalicsChanged(bool)), _ribbonTabText, SLOT(setFontItalics(bool)));
    connect(_encounterTextEdit, SIGNAL(fontUnderlineChanged(bool)), _ribbonTabText, SLOT(setFontUnderline(bool)));
    connect(_encounterTextEdit, SIGNAL(alignmentChanged(Qt::Alignment)), _ribbonTabText, SLOT(setAlignment(Qt::Alignment)));
    connect(_encounterTextEdit, SIGNAL(setHyperlinkActive(bool)), _ribbonTabText, SLOT(setHyperlinkActive(bool)));
    connect(_encounterTextEdit, SIGNAL(translatedChanged(bool)), _ribbonTabText, SLOT(setTranslationActive(bool)));
    connect(_encounterTextEdit, SIGNAL(codeViewChanged(bool)), _ribbonTabText, SLOT(setCodeView(bool)));
    connect(_encounterTextEdit, SIGNAL(codeViewVisible(bool)), _ribbonTabText, SLOT(showCodeView(bool)));
    connect(_encounterTextEdit, &EncounterTextEdit::setLayers, _ribbon->getPublishRibbon(), &PublishButtonProxy::setLayers);
    connect(_ribbon->getPublishRibbon(), &PublishButtonProxy::layerSelected, _encounterTextEdit, &EncounterTextEdit::layerSelected);
    ui->stackedWidgetEncounter->addFrames(QList<int>({DMHelper::CampaignType_Campaign,
                                                      DMHelper::CampaignType_Text,
                                                      DMHelper::CampaignType_LinkedText,
                                                      DMHelper::CampaignType_Placeholder}), _encounterTextEdit);
    qDebug() << "[MainWindow]     Adding Text Encounter widget as page #" << ui->stackedWidgetEncounter->count() - 1;

    // EncounterType_Battle
    _battleFrame = new BattleFrame;
    _battleFrame->setInitiativeType(_options->getInitiativeType());
    _battleFrame->setInitiativeScale(_options->getInitiativeScale());
    _battleFrame->setShowCountdown(_options->getShowCountdown());
    _battleFrame->setCountdownDuration(_options->getCountdownDuration());
    _battleFrame->setPointerFile(_options->getPointerFile());
    _battleFrame->setSelectedIcon(_options->getSelectedIcon());
    _battleFrame->setActiveIcon(_options->getActiveIcon());
    _battleFrame->setCombatantFrame(_options->getCombatantFrame());
    _battleFrame->setCountdownFrame(_options->getCountdownFrame());
    _battleFrame->setGridLocked(_options->getGridLocked());
    _battleFrame->setGridLockScale(_options->getGridLockScale());
    connect(_options, SIGNAL(initiativeTypeChanged(int)), _battleFrame, SLOT(setInitiativeType(int)));
    connect(_options, SIGNAL(initiativeScaleChanged(qreal)), _battleFrame, SLOT(setInitiativeScale(qreal)));
    connect(_options, SIGNAL(showCountdownChanged(bool)), _battleFrame, SLOT(setShowCountdown(bool)));
    connect(_options, SIGNAL(countdownDurationChanged(int)), _battleFrame, SLOT(setCountdownDuration(int)));
    connect(_options, SIGNAL(pointerFileNameChanged(const QString&)), _battleFrame, SLOT(setPointerFile(const QString&)));
    connect(_options, SIGNAL(selectedIconChanged(const QString&)), _battleFrame, SLOT(setSelectedIcon(const QString&)));
    connect(_options, SIGNAL(activeIconChanged(const QString&)), _battleFrame, SLOT(setActiveIcon(const QString&)));
    connect(_options, SIGNAL(combatantFrameChanged(const QString&)), _battleFrame, SLOT(setCombatantFrame(const QString&)));
    connect(_options, SIGNAL(countdownFrameChanged(const QString&)), _battleFrame, SLOT(setCountdownFrame(const QString&)));
    connect(_options, SIGNAL(gridLockedChanged(bool)), _battleFrame, SLOT(setGridLocked(bool)));
    connect(_options, SIGNAL(gridLockScaleChanged(qreal)), _battleFrame, SLOT(setGridLockScale(qreal)));
    connect(_pubWindow, SIGNAL(frameResized(QSize)), _battleFrame, SLOT(setTargetSize(QSize)));
    connect(_pubWindow, SIGNAL(labelResized(QSize)), _battleFrame, SLOT(setTargetLabelSize(QSize)));
    connect(_pubWindow, SIGNAL(publishMouseDown(const QPointF&)), _battleFrame, SLOT(publishWindowMouseDown(const QPointF&)));
    connect(_pubWindow, SIGNAL(publishMouseMove(const QPointF&)), _battleFrame, SLOT(publishWindowMouseMove(const QPointF&)));
    connect(_pubWindow, SIGNAL(publishMouseRelease(const QPointF&)), _battleFrame, SLOT(publishWindowMouseRelease(const QPointF&)));
    connect(_battleFrame, SIGNAL(characterSelected(QUuid)), this, SLOT(openCharacter(QUuid)));
    connect(_battleFrame, SIGNAL(monsterSelected(QString)), this, SLOT(openMonster(QString)));
    connect(_battleFrame, SIGNAL(registerRenderer(PublishGLRenderer*)), _pubWindow, SLOT(setRenderer(PublishGLRenderer*)));
    connect(_battleFrame, SIGNAL(showPublishWindow()), this, SLOT(showPublishWindow()));
    connect(_battleFrame, SIGNAL(modelChanged(BattleDialogModel*)), this, SLOT(battleModelChanged(BattleDialogModel*)));
    connect(_battleFrame, &BattleFrame::mapCreated, this, &MainWindow::updateCampaignTree);
    connect(_battleFrame, &BattleFrame::setLayers, _ribbon->getPublishRibbon(), &PublishButtonProxy::setLayers);
    connect(_ribbonTabBattle, SIGNAL(addCharacterClicked()), _battleFrame, SLOT(addCharacter()));
    connect(_ribbonTabBattle, SIGNAL(addMonsterClicked()), _battleFrame, SLOT(addMonsters()));
    connect(_ribbonTabBattle, SIGNAL(addNPCClicked()), _battleFrame, SLOT(addNPC()));
    connect(_ribbonTabBattle, SIGNAL(addObjectClicked()), _battleFrame, SLOT(addEffectObject()));
    connect(_ribbonTabBattle, SIGNAL(castSpellClicked()), _battleFrame, SLOT(castSpell()));
    connect(_ribbonTabBattle, SIGNAL(addEffectRadiusClicked()), _battleFrame, SLOT(addEffectRadius()));
    connect(_ribbonTabBattle, SIGNAL(addEffectConeClicked()), _battleFrame, SLOT(addEffectCone()));
    connect(_ribbonTabBattle, SIGNAL(addEffectCubeClicked()), _battleFrame, SLOT(addEffectCube()));
    connect(_ribbonTabBattle, SIGNAL(addEffectLineClicked()), _battleFrame, SLOT(addEffectLine()));
    connect(_ribbonTabBattle, SIGNAL(duplicateClicked()), _battleFrame, SLOT(duplicateSelection()));
    connect(_ribbonTabBattle, SIGNAL(statisticsClicked()), _battleFrame, SLOT(showStatistics()));
    connect(_ribbon->getPublishRibbon(), &PublishButtonProxy::layerSelected, _battleFrame, &BattleFrame::layerSelected);
    QShortcut* nextShortcut = new QShortcut(QKeySequence(tr("Ctrl+N", "Next Combatant")), this);
    connect(nextShortcut, SIGNAL(activated()), _battleFrame, SLOT(next()));

    connect(_ribbonTabBattleMap, SIGNAL(reloadMapClicked()), _battleFrame, SLOT(reloadMap()));
    connect(_ribbonTabBattleMap, &RibbonTabBattleMap::gridTypeChanged, _battleFrame, &BattleFrame::setGridType);
    connect(_ribbonTabBattleMap, SIGNAL(gridScaleChanged(int)), _battleFrame, SLOT(setGridScale(int)));
    connect(_battleFrame, &BattleFrame::gridConfigChanged, _ribbonTabBattleMap, &RibbonTabBattleMap::setGridConfig);
    connect(_ribbonTabBattleMap, &RibbonTabBattleMap::gridScaleSetClicked, _battleFrame, &BattleFrame::selectGridCount);
    connect(_ribbonTabBattleMap, SIGNAL(gridAngleChanged(int)), _battleFrame, SLOT(setGridAngle(int)));
    connect(_ribbonTabBattleMap, SIGNAL(gridXOffsetChanged(int)), _battleFrame, SLOT(setXOffset(int)));
    connect(_ribbonTabBattleMap, SIGNAL(gridYOffsetChanged(int)), _battleFrame, SLOT(setYOffset(int)));
    connect(_ribbonTabBattleMap, &RibbonTabBattleMap::gridWidthChanged, _battleFrame, &BattleFrame::setGridWidth);
    connect(_ribbonTabBattleMap, &RibbonTabBattleMap::gridColorChanged, _battleFrame, &BattleFrame::setGridColor);

    connect(_ribbonTabBattleMap, SIGNAL(editFoWClicked(bool)), _battleFrame, SLOT(setFoWEdit(bool)));
    connect(_battleFrame, SIGNAL(foWEditToggled(bool)), _ribbonTabBattleMap, SLOT(setEditFoW(bool)));
    connect(_ribbonTabBattleMap, SIGNAL(selectFoWClicked(bool)), _battleFrame, SLOT(setFoWSelect(bool)));
    connect(_battleFrame, SIGNAL(foWSelectToggled(bool)), _ribbonTabBattleMap, SLOT(setSelectFoW(bool)));
    BattleFrameMapDrawer* mapDrawer = _battleFrame->getMapDrawer();
    connect(_ribbonTabBattleMap, SIGNAL(drawEraseClicked(bool)), mapDrawer, SLOT(setErase(bool)));
    connect(_ribbonTabBattleMap, SIGNAL(smoothClicked(bool)), mapDrawer, SLOT(setSmooth(bool)));
    connect(_ribbonTabBattleMap, SIGNAL(brushSizeChanged(int)), mapDrawer, SLOT(setSize(int)));
    connect(_ribbonTabBattleMap, SIGNAL(fillFoWClicked()), mapDrawer, SLOT(fillFoW()));
    connect(_ribbonTabBattleMap, SIGNAL(brushModeChanged(int)), mapDrawer, SLOT(setBrushMode(int)));

    connect(this, SIGNAL(cancelSelect()), _battleFrame, SLOT(cancelSelect()));

    ui->stackedWidgetEncounter->addFrames(QList<int>({DMHelper::CampaignType_Battle,
                                                      DMHelper::CampaignType_BattleContent}), _battleFrame);
    qDebug() << "[MainWindow]     Adding Battle Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;

    // EncounterType_Character
    //CharacterFrame* charFrame = new CharacterFrame(_options);
    _characterFrame = new CharacterTemplateFrame(_options);
    _characterFrame->setHeroForgeToken(_options->getHeroForgeToken());
    //connect(_options, &OptionsContainer::heroForgeTokenChanged, charFrame, &CharacterFrame::setHeroForgeToken);
    //connect(charFrame, &CharacterFrame::heroForgeTokenChanged, _options, &OptionsContainer::setHeroForgeToken);
    connect(_options, &OptionsContainer::heroForgeTokenChanged, _characterFrame, &CharacterTemplateFrame::setHeroForgeToken);
    connect(_characterFrame, &CharacterTemplateFrame::heroForgeTokenChanged, _options, &OptionsContainer::setHeroForgeToken);
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_Combatant, _characterFrame);
    qDebug() << "[MainWindow]     Adding Character Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;
    connect(_characterFrame, SIGNAL(publishCharacterImage(QImage)), this, SIGNAL(dispatchPublishImage(QImage)));
    //connect(charFrame, SIGNAL(spellSelected(QString)), this, SLOT(openSpell(QString)));

    PartyFrame* partyFrame = new PartyFrame;
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_Party, partyFrame);
    qDebug() << "[MainWindow]     Adding Party Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;
    connect(partyFrame, SIGNAL(publishPartyImage(QImage)), this, SIGNAL(dispatchPublishImage(QImage)));
    connect(this, SIGNAL(characterChanged(QUuid)), partyFrame, SLOT(handleCharacterChanged(QUuid)));
    connect(partyFrame, SIGNAL(characterSelected(QUuid)), this, SLOT(openCharacter(QUuid)));

    // EncounterType_Map
    _mapFrame = new MapFrame;
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_Map, _mapFrame);
    qDebug() << "[MainWindow]     Adding Map Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;
    _mapFrame->setPointerFile(_options->getPointerFile());
    //connect(mapFrame, SIGNAL(publishImage(QImage)), this, SIGNAL(dispatchPublishImage(QImage)));
    connect(_mapFrame, SIGNAL(showPublishWindow()), this, SLOT(showPublishWindow()));
    connect(_mapFrame, SIGNAL(registerRenderer(PublishGLRenderer*)), _pubWindow, SLOT(setRenderer(PublishGLRenderer*)));
    connect(_pubWindow, SIGNAL(frameResized(QSize)), _mapFrame, SLOT(targetResized(QSize)));
    connect(_mapFrame, SIGNAL(encounterSelected(QUuid)), this, SLOT(openEncounter(QUuid)));

    connect(_ribbonTabMap, SIGNAL(editFileClicked()), _mapFrame, SLOT(editMapFile()));
    // connect(_ribbonTabMap, SIGNAL(zoomOneClicked()), mapFrame, SLOT(zoomOne()));

    connect(_ribbonTabMap, SIGNAL(mapEditClicked(bool)), _mapFrame, SLOT(setMapEdit(bool)));
    connect(_mapFrame, SIGNAL(mapEditChanged(bool)), _ribbonTabMap, SLOT(setMapEdit(bool)));

    connect(_ribbonTabMap, SIGNAL(drawEraseClicked(bool)), _mapFrame, SLOT(setErase(bool)));
    connect(_ribbonTabMap, SIGNAL(smoothClicked(bool)), _mapFrame, SLOT(setSmooth(bool)));
    connect(_ribbonTabMap, SIGNAL(brushSizeChanged(int)), _mapFrame, SLOT(brushSizeChanged(int)));
    connect(_ribbonTabMap, SIGNAL(fillFoWClicked()), _mapFrame, SLOT(fillFoW()));
    connect(_ribbonTabMap, SIGNAL(colorizeClicked()), _mapFrame, SLOT(colorize()));
    connect(_ribbonTabMap, SIGNAL(brushModeChanged(int)), _mapFrame, SLOT(setBrushMode(int)));
    connect(_mapFrame, SIGNAL(brushModeSet(int)), _ribbonTabMap, SLOT(setBrushMode(int)));

    connect(_ribbonTabWorldMap, &RibbonTabWorldMap::partySelected, _mapFrame, &MapFrame::setParty);
    connect(_ribbonTabWorldMap, &RibbonTabWorldMap::partyIconSelected, _mapFrame, &MapFrame::setPartyIcon);
    connect(_ribbonTabWorldMap, &RibbonTabWorldMap::showPartyClicked, _mapFrame, &MapFrame::setShowParty);
    connect(_ribbonTabWorldMap, &RibbonTabWorldMap::scaleChanged, _mapFrame, &MapFrame::setPartyScale);
    connect(_ribbonTabWorldMap, &RibbonTabWorldMap::showMarkersClicked, _mapFrame, &MapFrame::setShowMarkers);
    connect(_ribbonTabWorldMap, &RibbonTabWorldMap::addMarkerClicked, _mapFrame, &MapFrame::addNewMarker);
    connect(_ribbon->getPublishRibbon(), &PublishButtonProxy::layerSelected, _mapFrame, &MapFrame::layerSelected);
    connect(_mapFrame, &MapFrame::partyChanged, _ribbonTabWorldMap, &RibbonTabWorldMap::setParty);
    connect(_mapFrame, &MapFrame::partyIconChanged, _ribbonTabWorldMap, &RibbonTabWorldMap::setPartyIcon);
    connect(_mapFrame, &MapFrame::showPartyChanged, _ribbonTabWorldMap, &RibbonTabWorldMap::setShowParty);
    connect(_mapFrame, &MapFrame::partyScaleChanged, _ribbonTabWorldMap, &RibbonTabWorldMap::setScale);
    connect(_mapFrame, &MapFrame::setLayers, _ribbon->getPublishRibbon(), &PublishButtonProxy::setLayers);

    connect(_mapFrame, &MapFrame::showMarkersChanged, _ribbonTabWorldMap, &RibbonTabWorldMap::setShowMarkers);
    connect(_options, SIGNAL(pointerFileNameChanged(const QString&)), _mapFrame, SLOT(setPointerFile(const QString&)));

    connect(this, SIGNAL(cancelSelect()), _mapFrame, SLOT(cancelSelect()));

    connect(_pubWindow, SIGNAL(labelResized(QSize)), _mapFrame, SLOT(setTargetLabelSize(QSize)));
    connect(_pubWindow, SIGNAL(publishMouseDown(const QPointF&)), _mapFrame, SLOT(publishWindowMouseDown(const QPointF&)));
    connect(_pubWindow, SIGNAL(publishMouseMove(const QPointF&)), _mapFrame, SLOT(publishWindowMouseMove(const QPointF&)));
    connect(_pubWindow, SIGNAL(publishMouseRelease(const QPointF&)), _mapFrame, SLOT(publishWindowMouseRelease(const QPointF&)));

    connect(this, SIGNAL(cancelSelect()), _battleFrame, SLOT(cancelSelect()));

    // Connect the battle view ribbon to the battle frame and map frame
    connectBattleView(false); // initialize to false (default in the class is true) to ensure all connections are made

    // EncounterType_AudioTrack
    AudioTrackEdit* audioTrackEdit = new AudioTrackEdit;
    //TODO: can this edit frame be completely removed?
    //connect(this, SIGNAL(campaignLoaded(Campaign*)), audioTrackEdit, SLOT(setCampaign(Campaign*)));
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_AudioTrack, audioTrackEdit);
    qDebug() << "[MainWindow]     Adding Audio Track widget as page #" << ui->stackedWidgetEncounter->count() - 1;
    connect(audioTrackEdit, SIGNAL(trackTypeChanged(int)), _ribbonTabAudio, SLOT(setTrackType(int)));
    connect(_ribbonTabAudio, SIGNAL(playClicked(bool)), audioTrackEdit, SLOT(setPlay(bool)));
    connect(audioTrackEdit, SIGNAL(playChanged(bool)), _ribbonTabAudio, SLOT(setPlay(bool)));
    connect(_ribbonTabAudio, SIGNAL(repeatClicked(bool)), audioTrackEdit, SLOT(setRepeat(bool)));
    connect(audioTrackEdit, SIGNAL(repeatChanged(bool)), _ribbonTabAudio, SLOT(setRepeat(bool)));
    connect(_ribbonTabAudio, SIGNAL(muteClicked(bool)), audioTrackEdit, SLOT(setMute(bool)));
    connect(audioTrackEdit, SIGNAL(muteChanged(bool)), _ribbonTabAudio, SLOT(setMute(bool)));
    connect(_ribbonTabAudio, SIGNAL(volumeChanged(float)), audioTrackEdit, SLOT(setVolume(float)));
    connect(audioTrackEdit, SIGNAL(volumeChanged(float)), _ribbonTabAudio, SLOT(setVolume(float)));

    // EncounterType_WelcomeScreen
    WelcomeFrame* welcomeFrame = new WelcomeFrame(mruHandler);
    connect(welcomeFrame, SIGNAL(openCampaignFile(QString)), this, SLOT(openCampaign(QString)));
    connect(_ribbonTabFile, &RibbonTabFile::userGuideClicked, this, &MainWindow::openUsersGuide);
    connect(this, &MainWindow::openUsersGuide, welcomeFrame, &WelcomeFrame::openUsersGuide);
    connect(_ribbonTabFile, &RibbonTabFile::gettingStartedClicked, this, &MainWindow::openGettingStarted);
    connect(this, &MainWindow::openGettingStarted, welcomeFrame, &WelcomeFrame::openGettingStarted);
    connect(_ribbonTabFile, SIGNAL(userGuideClicked()), welcomeFrame, SLOT(openUsersGuide()));
    connect(_ribbonTabFile, SIGNAL(gettingStartedClicked()), welcomeFrame, SLOT(openGettingStarted()));
    connect(_ribbonTabFile, SIGNAL(gettingStartedClicked()), welcomeFrame, SLOT(openGettingStarted()));
    ui->stackedWidgetEncounter->addFrame(DMHelper::CampaignType_WelcomeScreen, welcomeFrame);
    qDebug() << "[MainWindow]     Adding Welcome Frame widget as page #" << ui->stackedWidgetEncounter->count() - 1;

    qDebug() << "[MainWindow] Encounter Pages Created";

    // Load the quick reference tabs
#ifndef Q_OS_MAC
    splash.showMessage(QString("Initializing Quick Reference tabs...\n"), Qt::AlignBottom | Qt::AlignHCenter);
#endif
    qApp->processEvents();
    qDebug() << "[MainWindow] Creating Reference Tabs";

    QuickRef::Initialize();
    _quickRefFrame = new QuickRefFrame(this);
    _quickRefDlg = createDialog(_quickRefFrame, QSize(width() * 3 / 4, height() * 9 / 10));
    connect(_options, &OptionsContainer::quickReferenceFileNameChanged, this, &MainWindow::readQuickRef);
    connect(QuickRef::Instance(), &QuickRef::changed, _quickRefFrame, &QuickRefFrame::refreshQuickRef);
    readQuickRef();

    connect(_ribbonTabTools, &RibbonTabTools::screenClicked, this, &MainWindow::handleOpenDMScreen);
    QShortcut* dmScreenShortcut = new QShortcut(QKeySequence(tr("Ctrl+E", "DM Screen")), this);
    connect(dmScreenShortcut, &QShortcut::activated, this, &MainWindow::handleOpenDMScreen);

    connect(_ribbonTabTools, &RibbonTabTools::tablesClicked, this, &MainWindow::handleOpenTables);
    QShortcut* tablesShortcut = new QShortcut(QKeySequence(tr("Ctrl+T", "Tables")), this);
    connect(tablesShortcut, &QShortcut::activated, this, &MainWindow::handleOpenTables);

    connect(_ribbonTabTools, &RibbonTabTools::referenceClicked, _quickRefDlg, &QDialog::exec);
    QShortcut* referenceShortcut = new QShortcut(QKeySequence(tr("Ctrl+R", "Reference")), this);
    connect(referenceShortcut, &QShortcut::activated, this, [=]() {openQuickref(QString());});

    connect(_ribbonTabTools, &RibbonTabTools::soundboardClicked, this, &MainWindow::handleOpenSoundboard);
    QShortcut* soundboardShortcut = new QShortcut(QKeySequence(tr("Ctrl+G", "Soundboard")), this);
    connect(soundboardShortcut, &QShortcut::activated, this, &MainWindow::handleOpenSoundboard);

    connect(_ribbonTabTools, &RibbonTabTools::calendarClicked, this, &MainWindow::handleOpenCalendar);
    QShortcut* calendarShortcut = new QShortcut(QKeySequence(tr("Ctrl+K", "Calendar")), this);
    connect(calendarShortcut, &QShortcut::activated, this, &MainWindow::handleOpenCalendar);

    connect(_ribbonTabTools, &RibbonTabTools::countdownClicked, this, &MainWindow::handleOpenCountdown);

    connect(_ribbonTabTools, &RibbonTabTools::searchClicked, this, &MainWindow::handleOpenGlobalSearch);
    QShortcut* searchShortcut = new QShortcut(QKeySequence(tr("Ctrl+F", "Search")), this);
    connect(searchShortcut, &QShortcut::activated, this, &MainWindow::handleOpenGlobalSearch);


    qDebug() << "[MainWindow] Reference Tabs Created";

#ifndef Q_OS_MAC
    splash.showMessage(QString("Preparing DMHelper\n"), Qt::AlignBottom | Qt::AlignHCenter);
#endif
    qApp->processEvents();

    _activeItems = new CampaignTreeActiveStack(this);
    connect(_activeItems, &CampaignTreeActiveStack::activateItem, this, &MainWindow::selectItemFromStack);
    connect(ui->btnBack, &QAbstractButton::clicked, _activeItems, &CampaignTreeActiveStack::backwards);
    connect(ui->btnForwards, &QAbstractButton::clicked, _activeItems, &CampaignTreeActiveStack::forwards);
    connect(_activeItems, &CampaignTreeActiveStack::backwardsAvailable, ui->btnBack, &QAbstractButton::setEnabled);
    connect(_activeItems, &CampaignTreeActiveStack::forwardsAvailable, ui->btnForwards, &QAbstractButton::setEnabled);
    connect(_battleFrame, &BattleFrame::navigateBackwards, _activeItems, &CampaignTreeActiveStack::backwards);
    connect(_battleFrame, &BattleFrame::navigateForwards, _activeItems, &CampaignTreeActiveStack::forwards);

    _audioPlayer = new AudioPlayer(this);
    _audioPlayer->setVolume(_options->getAudioVolume());
    //connect(mapFrame, SIGNAL(startTrack(AudioTrack*)), _audioPlayer, SLOT(playTrack(AudioTrack*)));

#ifdef INCLUDE_NETWORK_SUPPORT
    /*
    _networkController = new NetworkController(this);
    _networkController->setNetworkLogin(_options->getURLString(), _options->getUserName(), _options->getPassword(), _options->getSessionID(), QString());
    _networkController->enableNetworkController(_options->getNetworkEnabled());
    connect(this, SIGNAL(dispatchPublishImage(QImage)), _networkController, SLOT(uploadImage(QImage)));
    connect(_audioPlayer, SIGNAL(trackChanged(AudioTrack*)), _networkController, SLOT(uploadTrack(AudioTrack*)));
    connect(_options, SIGNAL(networkEnabledChanged(bool)), _networkController, SLOT(enableNetworkController(bool)));
    connect(_options, SIGNAL(networkSettingsChanged(QString, QString, QString, QString, QString)), _networkController, SLOT(setNetworkLogin(QString, QString, QString, QString, QString)));
    // TODO: _battleDlgMgr->setNetworkManager(_networkController);
    */
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

    delete ui;

    CampaignObjectFactory::Shutdown(); //CombatantFactory::Shutdown(); is handled by the CampaignObjectFactory

    Bestiary::Shutdown();
    DMH_VLC::Shutdown();
    ScaledPixmap::cleanupDefaultPixmap();
    DMH_DEBUG_OPENGL_Singleton::Shutdown();
}

void MainWindow::newCampaign()
{
    if(!closeCampaign())
        return;

    NewCampaignDialog* newCampaignDialog = new NewCampaignDialog(this);
    int result = newCampaignDialog->exec();
    if(result == QDialog::Accepted)
    {
        QString campaignName = newCampaignDialog->getCampaignName();
        if(campaignName.isEmpty())
            campaignName = QString("Campaign");

        _campaign = new Campaign(campaignName);

        _campaign->getRuleset().setRuleInitiative(newCampaignDialog->getInitiativeType());
        _campaign->getRuleset().setCharacterDataFile(newCampaignDialog->getCharacterDataFile());
        _campaign->getRuleset().setCharacterUIFile(newCampaignDialog->getCharacterUIFile());
        _campaign->getRuleset().setCombatantDoneCheckbox(newCampaignDialog->isCombatantDone());
        CampaignObjectFactory::configureFactories(_campaign->getRuleset(), DMHelper::CAMPAIGN_MAJOR_VERSION, DMHelper::CAMPAIGN_MINOR_VERSION);

        _campaign->addObject(EncounterFactory().createObject(DMHelper::CampaignType_Text, -1, QString("Notes"), false));
        _campaign->addObject(EncounterFactory().createObject(DMHelper::CampaignType_Party, -1, QString("Party"), false));
        _campaign->addObject(EncounterFactory().createObject(DMHelper::CampaignType_Text, -1, QString("Adventures"), false));
        _campaign->addObject(EncounterFactory().createObject(DMHelper::CampaignType_Text, -1, QString("World"), false));
        qDebug() << "[MainWindow] Campaign created: " << campaignName;
        selectItem(DMHelper::TreeType_Campaign, QUuid());
        emit campaignLoaded(_campaign);
        setDirty();
    }

    newCampaignDialog->deleteLater();
}

bool MainWindow::saveCampaign()
{
    return doSaveCampaign(_campaign ? _campaign->getName() + QString(".xml") : QString());
}

void MainWindow::saveCampaignAs()
{
    QString previousCampaignFileName = _campaignFileName;

    _campaignFileName.clear();

    if(doSaveCampaign(previousCampaignFileName) == false)
        _campaignFileName = previousCampaignFileName;
}

void MainWindow::openFileDialog()
{
    QString filename = QFileDialog::getOpenFileName(this, QString("Select Campaign"), QString(), QString("XML files (*.xml)"));
    if((!filename.isNull()) && (!filename.isEmpty()) && (QFile::exists(filename)))
        openCampaign(filename);
}

bool MainWindow::closeCampaign()
{
    if(!_campaign)
        return true;

    qDebug() << "[MainWindow] Closing Campaign: " << _campaignFileName;

    if((_ribbon) && (_ribbon->getPublishRibbon()))
        _ribbon->getPublishRibbon()->cancelPublish();

    if(_dirty)
    {
        QMessageBox::StandardButton result = QMessageBox::question(this,
                                                                   QString("Save Campaign"),
                                                                   QString("Would you like to save the current campaign before proceeding? Unsaved changes will be lost."),
                                                                   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if(result == QMessageBox::Cancel)
        {
            qDebug() << "[MainWindow] Closíng Campaign cancelled";
            return false;
        }

        if(result == QMessageBox::Yes)
            saveCampaign();
        else
            qDebug() << "[MainWindow] User decided not to save Campaign: " << _campaignFileName;
    }

    deleteCampaign();
    clearDirty();

    qDebug() << "[MainWindow] Campaign closed";

    return true;
}

void MainWindow::openDiceDialog()
{
    DiceRollDialog *drDlg = new DiceRollDialog(this);
    drDlg->resize(width() / 2, height() / 2);
    drDlg->exec();
}

void MainWindow::openEncounter(QUuid id)
{
    selectItem(id);
}

void MainWindow::openCharacter(QUuid id)
{
    selectItem(id);
}

void MainWindow::openMonster(const QString& monsterClass)
{
    if((monsterClass.isEmpty()) || (!Bestiary::Instance()->exists(monsterClass)))
        return;

    _bestiaryDlg.setMonster(monsterClass);
    openBestiary();
}

void MainWindow::openSpell(const QString& spellName)
{
    if((spellName.isEmpty()) || (!Spellbook::Instance()->exists(spellName)))
        return;

    _spellDlg.setSpell(spellName);
    openSpellbook();
}

void MainWindow::openQuickref(const QString& quickRefSection)
{
    _quickRefFrame->setQuickRefSection(quickRefSection);
    _quickRefDlg->exec();
}

void MainWindow::newCharacter()
{
    qDebug() << "[MainWindow] Creating a new character...";

    if(!_campaign)
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

    if(!CombatantFactory::Instance())
    {
        qDebug() << "[MainWindow] New character not created because the combatant factory could not be found";
        return;
    }

    Characterv2* character = nullptr;

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

            Characterv2Converter* convertCharacter = new Characterv2Converter();
            CombatantFactory::Instance()->setDefaultValues(convertCharacter);
            convertCharacter->readFromMonsterClass(*monsterClass);
            convertCharacter->setName(characterName);
            character = convertCharacter;
        }
    }

    if(!character)
        character = dynamic_cast<Characterv2*>(CombatantFactory::Instance()->createObject(DMHelper::CampaignType_Combatant, DMHelper::CombatantType_Character, characterName, false));

    addNewObject(character);
}

void MainWindow::importCharacter()
{
    if(!_campaign)
        return;

    CharacterImporter* importer = new CharacterImporter();
    connect(importer, &CharacterImporter::characterCreated, this, &MainWindow::addNewObject);
    connect(importer, &CharacterImporter::characterImported, this, &MainWindow::updateCampaignTree);
    connect(importer, &CharacterImporter::characterImported, this, &MainWindow::openCharacter);
    connect(this, &MainWindow::campaignLoaded, importer, &CharacterImporter::campaignChanged);
    importer->importCharacter(_campaign, true);
}

void MainWindow::importItem()
{
    if(!_campaign)
        return;

    CampaignObjectBase* currentObject = ui->treeView->currentCampaignObject();

    ObjectImportDialog dlg(_campaign, currentObject ? currentObject : _campaign, _campaignFileName);
    connect(&dlg, &ObjectImportDialog::importComplete, this, &MainWindow::updateCampaignTree);
    QScreen* primary = QGuiApplication::primaryScreen();
    if(primary)
        dlg.resize(primary->availableSize().width() / 2, primary->availableSize().height() / 4);
    dlg.exec();
}

void MainWindow::newParty()
{
    Party* newParty = dynamic_cast<Party*>(newEncounter(DMHelper::CampaignType_Party, QString("New Party"), QString("Enter new party name:")));
    _ribbonTabWorldMap->registerPartyIcon(newParty);
}

void MainWindow::newTextEncounter()
{
    newEncounter(DMHelper::CampaignType_Text, QString("New Entry"), QString("Enter new entry name:"));
}

void MainWindow::newLinkedText()
{
    QString extFilename = QFileDialog::getOpenFileName(this, QString("Select linked file"), QString(), tr("Text Files(*.txt);;HTML files (*.htm *.html);;Markdown files (*.md)"));
    if(extFilename.isEmpty())
        return;

    EncounterTextLinked* encounter = dynamic_cast<EncounterTextLinked*>(newEncounter(DMHelper::CampaignType_LinkedText, QString("New Linked Entry"), QString("Enter new entry name:")));
    if(!encounter)
        return;

    encounter->setLinkedFile(extFilename);
}

void MainWindow::newBattleEncounter()
{
    if(!_campaign)
        return;

    bool ok;
    QString encounterName = QInputDialog::getText(this, QString("New Combat"), QString("Enter new combat name:"), QLineEdit::Normal, QString(), &ok);
    if(!ok)
        return;

    CampaignObjectBase* encounter = EncounterFactory().createObject(DMHelper::CampaignType_Battle, -1, encounterName, false);
    if(!encounter)
        return;

    EncounterBattle* battle = dynamic_cast<EncounterBattle*>(encounter);
    if(!battle)
        return;

    battle->createBattleDialogModel();
    if(!battle->getBattleDialogModel())
        return;

    int gridScale = DMHelper::STARTING_GRID_SCALE;
    LayerGrid* gridLayer = nullptr;
    LayerTokens* monsterTokens = nullptr;
    LayerTokens* pcTokens = nullptr;
    bool hasGrid = false;

    qDebug() << "[Battle Frame] Selecting a new map...";

    CampaignObjectBase* currentObject = ui->treeView->currentCampaignObject();
    if(!currentObject)
        currentObject = _campaign;

    MapSelectDialog mapSelectDlg(*_campaign, currentObject->getID());
    if(mapSelectDlg.exec() == QDialog::Accepted)
    {
        if(mapSelectDlg.isMapSelected())
        {
            Map* battleMap = mapSelectDlg.getSelectedMap();
            if(battleMap)
            {
                battleMap->initialize();
                gridScale = battleMap->getLayerScene().getScale();
                hasGrid = battleMap->getLayerScene().layerCount(DMHelper::LayerType_Grid) > 0;

                // Create a grid after the first image layer, a monster token layer before the FoW
                for(int i = 0; i < battleMap->getLayerScene().layerCount(); ++i)
                {
                    Layer* layer = battleMap->getLayerScene().layerAt(i);
                    if(layer)
                    {
                        if((!monsterTokens) && (layer->getFinalType() == DMHelper::LayerType_Fow))
                        {
                            monsterTokens = new LayerTokens(battle->getBattleDialogModel(), QString("Monster tokens"));
                            battle->getBattleDialogModel()->getLayerScene().appendLayer(monsterTokens);
                        }

                        battle->getBattleDialogModel()->getLayerScene().appendLayer(new LayerReference(battleMap, layer, layer->getOrder()));

                        if((!hasGrid) && ((layer->getFinalType() == DMHelper::LayerType_Image) || (layer->getFinalType() == DMHelper::LayerType_Video)))
                        {
                             gridLayer = new LayerGrid(QString("Grid"));
                             battle->getBattleDialogModel()->getLayerScene().appendLayer(gridLayer);
                             hasGrid = true;
                        }
                    }
                }
            }
        }
        else if(mapSelectDlg.isBlankMap())
        {
            MapBlankDialog blankDlg;
            int result = blankDlg.exec();
            if(result == QDialog::Accepted)
            {
                LayerBlank* blankLayer = new LayerBlank(QString("Blank Layer"), blankDlg.getMapColor());
                blankLayer->setSize(blankDlg.getMapSize());
                battle->getBattleDialogModel()->getLayerScene().appendLayer(blankLayer);
            }
        }
        else if(mapSelectDlg.isNewMapImage())
        {
            Layer* mapLayer = selectMapFile();
            if(mapLayer)
            {
                mapLayer->initialize(QSize());
                battle->getBattleDialogModel()->getLayerScene().appendLayer(mapLayer);
            }
        }
    }

    if((!gridLayer) && (!hasGrid))
    {
        gridLayer = new LayerGrid(QString("Grid"));
        battle->getBattleDialogModel()->getLayerScene().appendLayer(gridLayer);
    }

    if(gridLayer)
        gridLayer->getConfig().setGridScale(gridScale);
    battle->getBattleDialogModel()->getLayerScene().setScale(gridScale);

    if(!monsterTokens)
    {
        monsterTokens = new LayerTokens(battle->getBattleDialogModel(), QString("Monster tokens"));
        battle->getBattleDialogModel()->getLayerScene().appendLayer(monsterTokens);
    }

    pcTokens = new LayerTokens(battle->getBattleDialogModel(), QString("PC tokens"));
    battle->getBattleDialogModel()->getLayerScene().appendLayer(pcTokens);

    // Add the active characters
    battle->getBattleDialogModel()->getLayerScene().setSelectedLayer(pcTokens);
    QPointF mapCenter = battle->getBattleDialogModel()->getLayerScene().boundingRect().center();
    if(mapCenter.isNull())
            mapCenter = QPointF(gridScale, gridScale);
    QPointF multiplePos(gridScale / 10.0, gridScale / 10.0);
    QList<Characterv2*> activeCharacters = _campaign->getActiveCharacters();
    for(int i = 0; i < activeCharacters.count(); ++i)
    {
        BattleDialogModelCharacter* newCharacter = new BattleDialogModelCharacter(activeCharacters.at(i));
        newCharacter->setPosition(mapCenter + (multiplePos * i));
        battle->getBattleDialogModel()->appendCombatant(newCharacter);
    }

    // Select the monster layer as a default to add monsters
    battle->getBattleDialogModel()->getLayerScene().setSelectedLayer(monsterTokens);
    battle->getBattleDialogModel()->setMapRect(battle->getBattleDialogModel()->getLayerScene().boundingRect().toRect());

//    battleFrame->setBattle(battle);
//    if(!battleFrame->createNewBattle())
//        battleFrame->editLayers();
    addNewObject(encounter);

//    BattleFrame* battleFrame = dynamic_cast<BattleFrame*>(ui->stackedWidgetEncounter->getCurrentFrame());
//    battleFrame->recenterCombatants();
}

void MainWindow::newMap(Layer* imageLayer)
{
    // TODO: throw a message box when it doesn't work.
    if(!_campaign)
        return;

    bool ok = false;
    QString mapName = QInputDialog::getText(this, QString("Enter Map Name"), QString("New Map"), QLineEdit::Normal, QString(), &ok);
    if(!ok)
        return;

    Layer* mapLayer = (imageLayer != nullptr) ? imageLayer : selectMapFile();
    if(!mapLayer)
        return;

    Map* map = dynamic_cast<Map*>(MapFactory().createObject(DMHelper::CampaignType_Map, -1, mapName, false));
    if(!map)
    {
        delete mapLayer;
        return;
    }

    map->getLayerScene().appendLayer(mapLayer);

    ok = false;
    int gridCount = QInputDialog::getInt(this, QString("Map Scale"), QString("How many grid squares should the map have horizontally? This is used to set the size of tokens on the map, even if you don't use the map for combat or with a grid."), DMHelper::DEFAULT_GRID_COUNT, 1, 100000, 1, &ok);
    if(ok)
        map->setGridCount(gridCount);

    QMessageBox::StandardButton result = QMessageBox::question(this, QString("Map Fog of War"), QString("Do you want to add a Fog of War onto your map?"));
    if(result == QMessageBox::Yes)
    {
        LayerFow* fowLayer = new LayerFow(QString("FoW"));
        map->getLayerScene().appendLayer(fowLayer);
    }

    addNewObject(map);
}

void MainWindow::newMedia()
{
    if(!_campaign)
        return;

    bool ok = false;
    QString mediaName = QInputDialog::getText(this, QString("Enter Media Entry Name"), QString("New Media"), QLineEdit::Normal, QString(), &ok);
    if(!ok)
        return;

    QString filename = QFileDialog::getOpenFileName(this, QString("Select Media File..."));
    if(filename.isEmpty())
        return;

    Layer* mediaLayer;
    QImageReader reader(filename);
    if(reader.canRead())
    {
        mediaLayer = new LayerImage(QString("Media Image: ") + QFileInfo(filename).fileName(), filename);
    }
    else
    {
        QMessageBox::StandardButton result = QMessageBox::question(this, QString("Video"), QString("Is the selected media file a video?"));
        if(result != QMessageBox::Yes)
            return;

        mediaLayer = new LayerVideo(QString("Media Video: ") + QFileInfo(filename).fileName(), filename);
    }

    Map* mediaMap = dynamic_cast<Map*>(MapFactory().createObject(DMHelper::CampaignType_Map, -1, mediaName, false));
    if(!mediaMap)
    {
        delete mediaLayer;
        return;
    }

    mediaMap->getLayerScene().appendLayer(mediaLayer);
    mediaMap->getLayerScene().setScale(DMHelper::STARTING_GRID_SCALE);

    addNewObject(mediaMap);
}

void MainWindow::newAudioEntry()
{
    if(!_campaign)
        return;

    addNewAudioObject(QFileDialog::getOpenFileName(this, QString("Select local audio file")));
}

void MainWindow::newSyrinscapeEntry()
{
    if(!_campaign)
        return;

    QString syrinscapeInstructions("To add a link to a Syrinscape sound:\n\n1) Hit the '+' key or select ""3rd party app integration"" ENABLE in the settings menu\n2) Little pluses will appear next to all the MOODs and OneShots\n3) Click one of these pluses to copy a URI shortcut to the clipboard\n4) Paste this URI into the text box here:\n");

    bool ok = false;
    QString urlName = QInputDialog::getText(this, QString("Enter Syrinscape Audio URI"), syrinscapeInstructions, QLineEdit::Normal, QString(), &ok);
    if((!ok)||(urlName.isEmpty()))
        return;

    addNewAudioObject(urlName);
}

void MainWindow::newSyrinscapeOnlineEntry()
{
    if(!_campaign)
        return;

    QString syrinscapeInstructions("To add a link to a Syrinscape Online sound:\n\n1) Open the desired sound clip in the Syrinscape online player master control\n2) Open the menu (top right) and click 'Show Remote Control Links'\n3) Click the play icon to copy the play link\n4) Paste this URI into the text box here:\n");

    bool ok = false;
    QString urlName = QInputDialog::getText(this, QString("Enter Syrinscape Online Audio URI"), syrinscapeInstructions, QLineEdit::Normal, QString(), &ok);
    if((!ok)||(urlName.isEmpty()))
        return;

    addNewAudioObject(urlName);
}

void MainWindow::newYoutubeEntry()
{
    if(!_campaign)
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
    if((!_campaign)||(!_treeModel))
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

    // Check that the object to be removed is not being published
    if(_pubWindow)
    {
        QUuid publishId = _pubWindow->getObjectId();
        if((!publishId.isNull()) && (publishId == removeObject->getID()))
        {
            if((!_ribbon) || (!_ribbon->getPublishRibbon()))
                return;

            _ribbon->getPublishRibbon()->cancelPublish();
        }
            /*
        {
            CampaignObjectFrame* objectFrame = dynamic_cast<CampaignObjectFrame*>(ui->stackedWidgetEncounter->currentWidget());
            if(objectFrame)
                objectFrame->publishClicked(false);
        }
        */
    }

    QUuid nextObjectId;
    if(parentObject->getObjectType() != DMHelper::CampaignType_Campaign)
    {
        nextObjectId = parentObject->getID();
    }
    else
    {
        const QList<CampaignObjectBase*> campaignChildren = _campaign->getChildObjects();
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

//    delete _campaign->removeObject(removeObject->getID());
    _campaign->removeObject(removeObject->getID());
    removeObject->deleteLater();
    updateCampaignTree();
}

void MainWindow::showNotes()
{
    if(!_campaign)
        return;

    CampaignNotesDialog dlg(_campaign->getNotes(), this);
    QScreen* primary = QGuiApplication::primaryScreen();
    if(primary)
        dlg.resize(primary->availableSize().width() / 2, primary->availableSize().height() / 2);
    if(dlg.exec() == QDialog::Accepted)
        _campaign->setNotes(dlg.getNotes());
}

void MainWindow::addNote()
{
    if(!_campaign)
        return;

    QInputDialog inputDlg(this);
    inputDlg.setInputMode(QInputDialog::TextInput);
    inputDlg.setWindowTitle(QString("New Note"));
    inputDlg.setLabelText(QString("Note:"));
    QScreen* primary = QGuiApplication::primaryScreen();
    if(primary)
        inputDlg.resize(primary->availableSize().width() / 2, inputDlg.height());

    inputDlg.exec();
    QString newNote = inputDlg.textValue();
    if(!newNote.isEmpty())
        _campaign->addNote(newNote);
}

void MainWindow::editCurrentItem()
{
    if((!_campaign)||(!_treeModel))
        return;

    QModelIndex index = ui->treeView->currentIndex();
    QStandardItem* editItem = _treeModel->itemFromIndex(index);
    int type = editItem->data(DMHelper::TreeItemData_Type).toInt();

    if(editItem->isEditable())
    {
        ui->treeView->edit(index);
    }
    else if(type == DMHelper::TreeType_Map)
    {
        Map* map = dynamic_cast<Map*>(_campaign->getObjectById(QUuid(editItem->data(DMHelper::TreeItemData_ID).toString())));

        if(map)
        {
            bool ok = false;
            QString mapName = QInputDialog::getText(this, QString("Enter Map Name"), QString("New Map"), QLineEdit::Normal, QString(), &ok);
            if(ok)
                map->setName(mapName);
        }
    }
}

void MainWindow::setCurrentItemIcon()
{
    CampaignObjectBase* currentObject = ui->treeView->currentCampaignObject();
    if(!currentObject)
        return;

    QString newIconFileName = QFileDialog::getOpenFileName(this, QString("Select Icon"));
    if(newIconFileName.isEmpty())
        return;

    QImageReader reader(newIconFileName);
    if(reader.canRead())
        currentObject->setIconFile(newIconFileName);
}

void MainWindow::clearCurrentItemIcon()
{
    CampaignObjectBase* currentObject = ui->treeView->currentCampaignObject();
    if(!currentObject)
        return;

    currentObject->setIconFile(QString());
}

void MainWindow::exportCurrentItem()
{
    if((!_campaign)||(!_treeModel))
        return;

    QModelIndex index = ui->treeView->currentIndex();
    QStandardItem* exportItem = _treeModel->itemFromIndex(index);
    if(!exportItem)
        return;

    QUuid exportId(exportItem->data(DMHelper::TreeItemData_ID).toString());

    ExportDialog dlg(*_campaign, exportId);
    dlg.resize(width() * 3 / 4, height() * 9 / 10);
    dlg.exec();

    qDebug() << "[MainWindow] Export complete";
}

void MainWindow::addNewObject(CampaignObjectBase* newObject)
{
    if(!_campaign || !_treeModel || !newObject)
        return;

    CampaignObjectBase* currentObject = ui->treeView->currentCampaignObject();
    if(!currentObject)
        currentObject = _campaign;

    qDebug() << "[MainWindow] Adding object " << newObject->getName() << " (" << newObject->getID() << "), to object " << currentObject->getName() << " (" << currentObject->getID() << ")";

    currentObject->setExpanded(true);
    currentObject->addObject(newObject);

    updateCampaignTree();

    selectItem(newObject->getID());
}

void MainWindow::clearDirty()
{
    _dirty = false;
    setWindowModified(_dirty);
}

void MainWindow::setDirty()
{
    _dirty = true;
    setWindowModified(_dirty);
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
        if(!_pubWindow->isVisible())
            _pubWindow->show();
    }
    else
    {
        _pubWindow->hide();
    }
}

void MainWindow::linkActivated(const QUrl & link)
{
    QString path = link.path();
    if(path.startsWith(QString("DMHelper@")))
    {
        QString linkName = path.remove(0, 9);
        CampaignTreeItem* item = _treeModel->getObjectItemByName(linkName);
        if(item)
            selectIndex(item->index());
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

    if(Bestiary::Instance()->isDirty())
    {
        qDebug() << "[MainWindow] Existing bestiary is unsaved!";
        QMessageBox::StandardButton result = QMessageBox::critical(this,
                                                                   QString("Unsaved Bestiary"),
                                                                   QString("The current bestiary has not been saved. Would you like to save it before loading a new bestiary? If you don't. you may lose monster data!"),
                                                                   QMessageBox::Yes | QMessageBox::No);
        if(result == QMessageBox::Yes)
        {
            QString bestiaryFileName = QFileDialog::getSaveFileName(this, QString("Save Bestiary"), QString(), QString("XML files (*.xml)"));
            if(!bestiaryFileName.isEmpty())
            {
                if(Bestiary::Instance()->writeBestiary(bestiaryFileName))
                    qDebug() << "[MainWindow] Bestiary file writing complete: " << bestiaryFileName;
                else
                    qDebug() << "[MainWindow] ERROR: Bestiary file writing failed: " << bestiaryFileName;
            }
        }
    }

    disconnect(Bestiary::Instance(), SIGNAL(changed()), &_bestiaryDlg, SLOT(dataChanged()));

    QString bestiaryFileName = _options->getBestiaryFileName();
    if(!Bestiary::Instance()->readBestiary(bestiaryFileName))
    {
        qDebug() << "[MainWindow] ERROR: Bestiary reading failed: " << bestiaryFileName;
        return;
    }

    // Bestiary file seems ok, make a backup
    _options->backupFile(bestiaryFileName);

    _bestiaryDlg.dataChanged();
    if(!_options->getLastMonster().isEmpty() && Bestiary::Instance()->exists(_options->getLastMonster()))
        _bestiaryDlg.setMonster(_options->getLastMonster());
    else
        _bestiaryDlg.setMonster(Bestiary::Instance()->getFirstMonsterClass());

    connect(Bestiary::Instance(), SIGNAL(changed()), &_bestiaryDlg, SLOT(dataChanged()));

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

    if(Spellbook::Instance()->isDirty())
    {
        qDebug() << "[MainWindow] Existing spellbook is unsaved!";
        QMessageBox::StandardButton result = QMessageBox::critical(this,
                                                                   QString("Unsaved Spellbook"),
                                                                   QString("The current spellbook has not been saved. Would you like to save it before loading a new spellbook? If you don't. you may lose spell data!"),
                                                                   QMessageBox::Yes | QMessageBox::No);
        if(result == QMessageBox::Yes)
        {
            QString spellbookFileName = QFileDialog::getSaveFileName(this, QString("Save Spellbook"), QString(), QString("XML files (*.xml)"));
            if(!spellbookFileName.isEmpty())
            {
                if(Spellbook::Instance()->writeSpellbook(spellbookFileName))
                    qDebug() << "[MainWindow] Spellbook file writing complete: " << spellbookFileName;
                else
                    qDebug() << "[MainWindow] ERROR: Spellbook file writing failed: " << spellbookFileName;
            }
        }
    }

    disconnect(Spellbook::Instance(), SIGNAL(changed()), &_spellDlg, SLOT(dataChanged()));

    QString spellbookFileName = _options->getSpellbookFileName();
    if(!Spellbook::Instance()->readSpellbook(spellbookFileName))
    {
        qDebug() << "[MainWindow] ERROR: Spellbook reading failed: " << spellbookFileName;
        return;
    }

    // Spellbook file seems ok, make a backup
    _options->backupFile(spellbookFileName);

    _spellDlg.dataChanged();
    if(!_options->getLastSpell().isEmpty() && Spellbook::Instance()->exists(_options->getLastSpell()))
        _spellDlg.setSpell(_options->getLastSpell());
    else
        _spellDlg.setSpell(Spellbook::Instance()->getFirstSpell());

    connect(Spellbook::Instance(), SIGNAL(changed()), &_spellDlg, SLOT(dataChanged()));

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
    if(!_initialized)
    {
        if((_options) && (!_recoveryMode))
        {
            // Implement any one-time initialization here
            bool firstStart = !_options->doDataSettingsExist();
            if(firstStart)
            {
                LegalDialog dlg;
                dlg.exec();
                _options->setUpdatesEnabled(dlg.isUpdatesEnabled());
                _options->setStatisticsAccepted(dlg.isStatisticsAccepted());
            }

            checkForUpdates(true);

            if((_options->getMRUHandler()) && (_options->getMRUHandler()->getMRUList().count() == 1))
                openCampaign(_options->getMRUHandler()->getMRUList().first());

            QString versionString = QString("%1.%2.%3").arg(DMHelper::DMHELPER_MAJOR_VERSION)
                                                       .arg(DMHelper::DMHELPER_MINOR_VERSION)
                                                       .arg(DMHelper::DMHELPER_ENGINEERING_VERSION);
            if(_options->getLastAppVersion() != versionString)
            {
                WhatsNewDialog* whatsNewDlg = new WhatsNewDialog(QString(":/img/data/whatsnew.txt"), QString("What's New"), this);
                whatsNewDlg->show();
                whatsNewDlg->move((frameGeometry().center() - whatsNewDlg->rect().center()) / 2);
            }

            if(firstStart)
            {
                WhatsNewDialog* firstStartDlg = new WhatsNewDialog(QString(":/img/data/firststart.txt"), QString("Welcome to DMHelper!"), this);
                firstStartDlg->show();
                firstStartDlg->move((frameGeometry().center() - firstStartDlg->rect().center()) / 2);
            }
        }

        _initialized = true;
        if(_options)
            _options->setLoading(false);
    }

    int ribbonHeight = RibbonFrame::getRibbonHeight();
    int iconSize =  ribbonHeight * 4 / 5;
    ui->btnBack->setMinimumWidth(ribbonHeight);
    ui->btnBack->setMaximumWidth(ribbonHeight);
    ui->btnBack->setMinimumHeight(ribbonHeight);
    ui->btnBack->setMaximumHeight(ribbonHeight);
    ui->btnBack->setIconSize(QSize(iconSize, iconSize));
    ui->btnForwards->setMinimumWidth(ribbonHeight);
    ui->btnForwards->setMaximumWidth(ribbonHeight);
    ui->btnForwards->setMinimumHeight(ribbonHeight);
    ui->btnForwards->setMaximumHeight(ribbonHeight);
    ui->btnForwards->setIconSize(QSize(iconSize, iconSize));

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

    if((Bestiary::Instance()) && (Bestiary::Instance()->isDirty()))
        writeBestiary();
    if((Spellbook::Instance()) && (Spellbook::Instance()->isDirty()))
        writeSpellbook();

    _options->setLastMonster(_bestiaryDlg.getMonster() ? _bestiaryDlg.getMonster()->getName() : "");
    _options->setLastSpell(_spellDlg.getSpell() ? _spellDlg.getSpell()->getName() : "");
    _options->writeSettings();

    qApp->quit();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
}

void MainWindow::mousePressEvent(QMouseEvent * event)
{
    _mouseDownPos = event->pos();
    _mouseDown = true;

    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent * event)
{
    _mouseDown = false;
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent * event)
{
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(!event)
        return;

    if(event->modifiers() == Qt::AltModifier)
    {
        if(event->key() == Qt::Key_Left)
            _activeItems->backwards();
        else if(event->key() == Qt::Key_Right)
            _activeItems->forwards();
        return;
    }

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

    if((data->hasUrls()) &&
       (data->urls().count() == 1) &&
       (data->urls().constFirst().isLocalFile()))
    {
        QString filename = data->urls().constFirst().toLocalFile();
        QMimeType mimeType = QMimeDatabase().mimeTypeForFile(filename);
        if((mimeType.isValid()) &&
           ((mimeType.name().startsWith("image/")) ||
            (mimeType.name().startsWith("video/")) ||
            (mimeType.name().startsWith("text/")) ||
            (mimeType.suffixes().contains("xml"))))
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

    if((data->hasUrls()) &&
        (data->urls().count() == 1) &&
        (data->urls().constFirst().isLocalFile()))
    {
        QString filename = data->urls().constFirst().toLocalFile();
        QMimeType mimeType = QMimeDatabase().mimeTypeForFile(filename);
        if(mimeType.isValid())
        {
            if(mimeType.name().startsWith("image/"))
            {
                QImageReader reader(filename);
                if(reader.canRead())
                    newMap(new LayerImage(QString("Map Image: ") + QFileInfo(filename).fileName(), filename));

                event->acceptProposedAction();
                return;
            }
            else if(mimeType.name().startsWith("video/"))
            {
                newMap(new LayerVideo(QString("Map Video: ") + QFileInfo(filename).fileName(), filename));
                event->acceptProposedAction();
                return;
            }
            else if(mimeType.suffixes().contains("xml")) // XML first because it is a form of text
            {
                openCampaign(filename);
                event->acceptProposedAction();
                return;
            }
            else if(mimeType.name().startsWith("text/"))
            {
                //_bestiaryDlg.addMonsterText(filename);
                event->acceptProposedAction();
                return;
            }
        }
    }

    event->ignore();
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

    _ribbonTabBattleMap = new RibbonTabBattleMap(this);
    _ribbonTabBattleMap->hide();
    _ribbonTabBattleView = new RibbonTabBattleView(this);
    _ribbonTabBattleView->hide();
    _ribbonTabBattle = new RibbonTabBattle(this);
    _ribbonTabBattle->hide();
    _ribbonTabText = new RibbonTabText(this);
    _ribbonTabText->hide();
    _ribbonTabMap = new RibbonTabMap(this);
    _ribbonTabMap->hide();
    _ribbonTabWorldMap = new RibbonTabWorldMap(this);
    _ribbonTabWorldMap->hide();
    _ribbonTabAudio = new RibbonTabAudio(this);
    _ribbonTabAudio->hide();

    connect(_ribbon->getPublishRibbon(), SIGNAL(playersWindowClicked(bool)), this, SLOT(showPublishWindow(bool)));
    QShortcut* publishShortcut = new QShortcut(QKeySequence(tr("Ctrl+P", "Publish")), this);
    connect(publishShortcut, SIGNAL(activated()), _ribbon, SLOT(clickPublish()));

    _ribbon->setCurrentIndex(0);
    setMenuWidget(_ribbon);
}

void MainWindow::connectBattleView(bool toBattle)
{
    if((!_ribbonTabBattleView) || (_ribbonTabBattleView->getIsBattle() == toBattle))
        return;

    _ribbonTabBattleView->setIsBattle(toBattle);

    if(toBattle)
    {
        // Battle
        connect(_ribbonTabBattleView, SIGNAL(zoomInClicked()), _battleFrame, SLOT(zoomIn()));
        connect(_ribbonTabBattleView, SIGNAL(zoomOutClicked()), _battleFrame, SLOT(zoomOut()));
        connect(_ribbonTabBattleView, SIGNAL(zoomFullClicked()), _battleFrame, SLOT(zoomFit()));
        connect(_ribbonTabBattleView, SIGNAL(zoomSelectClicked(bool)), _battleFrame, SLOT(zoomSelect(bool)));
        connect(_battleFrame, SIGNAL(zoomSelectToggled(bool)), _ribbonTabBattleView, SLOT(setZoomSelect(bool)));
        connect(_ribbonTabBattleView, &RibbonTabBattleView::cameraCoupleClicked, _battleFrame, &BattleFrame::setCameraCouple);
        connect(_ribbonTabBattleView, SIGNAL(cameraZoomClicked()), _battleFrame, SLOT(setCameraMap()));
        connect(_ribbonTabBattleView, SIGNAL(cameraSelectClicked(bool)), _battleFrame, SLOT(setCameraSelect(bool)));
        connect(_ribbonTabBattleView, &RibbonTabBattleView::cameraVisibleClicked, _battleFrame, &BattleFrame::setCameraVisible);
        connect(_battleFrame, SIGNAL(cameraSelectToggled(bool)), _ribbonTabBattleView, SLOT(setCameraSelect(bool)));
        connect(_ribbonTabBattleView, SIGNAL(cameraEditClicked(bool)), _battleFrame, SLOT(setCameraEdit(bool)));
        connect(_battleFrame, SIGNAL(cameraEditToggled(bool)), _ribbonTabBattleView, SLOT(setCameraEdit(bool)));
        connect(_ribbonTabBattleView, SIGNAL(heightChanged(bool, qreal)), _battleFrame, SLOT(setDistanceHeight(bool, qreal)));
        connect(_ribbonTabBattleView, SIGNAL(pointerClicked(bool)), _battleFrame, SLOT(setPointerOn(bool)));
        connect(_battleFrame, SIGNAL(pointerToggled(bool)), _ribbonTabBattleView, SLOT(setPointerOn(bool)));
        connect(_ribbonTabBattleView, &RibbonTabBattleView::distanceClicked, _battleFrame, &BattleFrame::setDistance);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::freeDistanceClicked, _battleFrame, &BattleFrame::setFreeDistance);
        //connect(_ribbonTabBattleView, &RibbonTabBattleView::distanceScaleChanged, _battleFrame, &BattleFrame::setDistanceScale);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::distanceLineColorChanged, _battleFrame, &BattleFrame::setDistanceLineColor);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::distanceLineTypeChanged, _battleFrame, &BattleFrame::setDistanceLineType);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::distanceLineWidthChanged, _battleFrame, &BattleFrame::setDistanceLineWidth);
        connect(_battleFrame, &BattleFrame::distanceToggled, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceOn);
        connect(_battleFrame, &BattleFrame::freeDistanceToggled, _ribbonTabBattleView, &RibbonTabBattleView::setFreeDistanceOn);
        connect(_battleFrame, &BattleFrame::distanceChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistance);
        //connect(_battleFrame, &BattleFrame::distanceLineColorChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceLineColor);
        //connect(_battleFrame, &BattleFrame::distanceLineTypeChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceLineType);
        //connect(_battleFrame, &BattleFrame::distanceLineWidthChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceLineWidth);

        // Map
        disconnect(_ribbonTabBattleView, SIGNAL(zoomInClicked()), _mapFrame, SLOT(zoomIn()));
        disconnect(_ribbonTabBattleView, SIGNAL(zoomOutClicked()), _mapFrame, SLOT(zoomOut()));
        disconnect(_ribbonTabBattleView, SIGNAL(zoomFullClicked()), _mapFrame, SLOT(zoomFit()));
        disconnect(_ribbonTabBattleView, SIGNAL(zoomSelectClicked(bool)), _mapFrame, SLOT(zoomSelect(bool)));
        disconnect(_mapFrame, SIGNAL(zoomSelectChanged(bool)), _ribbonTabBattleView, SLOT(setZoomSelect(bool)));
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::cameraCoupleClicked, _mapFrame, &MapFrame::setCameraCouple);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::cameraZoomClicked, _mapFrame, &MapFrame::setCameraMap);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::cameraVisibleClicked, _mapFrame, &MapFrame::setCameraVisible);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::cameraSelectClicked, _mapFrame, &MapFrame::setCameraSelect);
        disconnect(_mapFrame, &MapFrame::cameraSelectToggled, _ribbonTabBattleView, &RibbonTabBattleView::setCameraSelect);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::cameraEditClicked, _mapFrame, &MapFrame::setCameraEdit);
        disconnect(_mapFrame, &MapFrame::cameraEditToggled, _ribbonTabBattleView, &RibbonTabBattleView::setCameraEdit);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::pointerClicked, _mapFrame, &MapFrame::setPointerOn);
        disconnect(_mapFrame, &MapFrame::pointerToggled, _ribbonTabBattleView, &RibbonTabBattleView::setPointerOn);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::distanceClicked, _mapFrame, &MapFrame::setDistance);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::freeDistanceClicked, _mapFrame, &MapFrame::setFreeDistance);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::distanceScaleChanged, _mapFrame, &MapFrame::setDistanceScale);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::distanceLineColorChanged, _mapFrame, &MapFrame::setDistanceLineColor);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::distanceLineTypeChanged, _mapFrame, &MapFrame::setDistanceLineType);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::distanceLineWidthChanged, _mapFrame, &MapFrame::setDistanceLineWidth);
        disconnect(_mapFrame, &MapFrame::showDistanceChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceOn);
        disconnect(_mapFrame, &MapFrame::showFreeDistanceChanged, _ribbonTabBattleView, &RibbonTabBattleView::setFreeDistanceOn);
        disconnect(_mapFrame, &MapFrame::distanceScaleChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceScale);
        disconnect(_mapFrame, &MapFrame::distanceChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistance);
        disconnect(_mapFrame, &MapFrame::distanceLineColorChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceLineColor);
        disconnect(_mapFrame, &MapFrame::distanceLineTypeChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceLineType);
        disconnect(_mapFrame, &MapFrame::distanceLineWidthChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceLineWidth);
    }
    else
    {
        // Battle
        disconnect(_ribbonTabBattleView, SIGNAL(zoomInClicked()), _battleFrame, SLOT(zoomIn()));
        disconnect(_ribbonTabBattleView, SIGNAL(zoomOutClicked()), _battleFrame, SLOT(zoomOut()));
        disconnect(_ribbonTabBattleView, SIGNAL(zoomFullClicked()), _battleFrame, SLOT(zoomFit()));
        disconnect(_ribbonTabBattleView, SIGNAL(zoomSelectClicked(bool)), _battleFrame, SLOT(zoomSelect(bool)));
        disconnect(_battleFrame, SIGNAL(zoomSelectToggled(bool)), _ribbonTabBattleView, SLOT(setZoomSelect(bool)));
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::cameraCoupleClicked, _battleFrame, &BattleFrame::setCameraCouple);
        disconnect(_ribbonTabBattleView, SIGNAL(cameraZoomClicked()), _battleFrame, SLOT(setCameraMap()));
        disconnect(_ribbonTabBattleView, SIGNAL(cameraSelectClicked(bool)), _battleFrame, SLOT(setCameraSelect(bool)));
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::cameraVisibleClicked, _battleFrame, &BattleFrame::setCameraVisible);
        disconnect(_battleFrame, SIGNAL(cameraSelectToggled(bool)), _ribbonTabBattleView, SLOT(setCameraSelect(bool)));
        disconnect(_ribbonTabBattleView, SIGNAL(cameraEditClicked(bool)), _battleFrame, SLOT(setCameraEdit(bool)));
        disconnect(_battleFrame, SIGNAL(cameraEditToggled(bool)), _ribbonTabBattleView, SLOT(setCameraEdit(bool)));
        disconnect(_ribbonTabBattleView, SIGNAL(heightChanged(bool, qreal)), _battleFrame, SLOT(setDistanceHeight(bool, qreal)));
        disconnect(_ribbonTabBattleView, SIGNAL(pointerClicked(bool)), _battleFrame, SLOT(setPointerOn(bool)));
        disconnect(_battleFrame, SIGNAL(pointerToggled(bool)), _ribbonTabBattleView, SLOT(setPointerOn(bool)));
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::distanceClicked, _battleFrame, &BattleFrame::setDistance);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::freeDistanceClicked, _battleFrame, &BattleFrame::setFreeDistance);
        //disconnect(_ribbonTabBattleView, &RibbonTabBattleView::distanceScaleChanged, _battleFrame, &BattleFrame::setDistanceScale);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::distanceLineColorChanged, _battleFrame, &BattleFrame::setDistanceLineColor);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::distanceLineTypeChanged, _battleFrame, &BattleFrame::setDistanceLineType);
        disconnect(_ribbonTabBattleView, &RibbonTabBattleView::distanceLineWidthChanged, _battleFrame, &BattleFrame::setDistanceLineWidth);
        disconnect(_battleFrame, &BattleFrame::distanceToggled, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceOn);
        disconnect(_battleFrame, &BattleFrame::freeDistanceToggled, _ribbonTabBattleView, &RibbonTabBattleView::setFreeDistanceOn);
        disconnect(_battleFrame, &BattleFrame::distanceChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistance);
        //disconnect(_battleFrame, &BattleFrame::distanceLineColorChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceLineColor);
        //disconnect(_battleFrame, &BattleFrame::distanceLineTypeChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceLineType);
        //disconnect(_battleFrame, &BattleFrame::distanceLineWidthChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceLineWidth);

        // Map
        connect(_ribbonTabBattleView, SIGNAL(zoomInClicked()), _mapFrame, SLOT(zoomIn()));
        connect(_ribbonTabBattleView, SIGNAL(zoomOutClicked()), _mapFrame, SLOT(zoomOut()));
        connect(_ribbonTabBattleView, SIGNAL(zoomFullClicked()), _mapFrame, SLOT(zoomFit()));
        connect(_ribbonTabBattleView, SIGNAL(zoomSelectClicked(bool)), _mapFrame, SLOT(zoomSelect(bool)));
        connect(_mapFrame, SIGNAL(zoomSelectChanged(bool)), _ribbonTabBattleView, SLOT(setZoomSelect(bool)));
        connect(_ribbonTabBattleView, &RibbonTabBattleView::cameraCoupleClicked, _mapFrame, &MapFrame::setCameraCouple);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::cameraZoomClicked, _mapFrame, &MapFrame::setCameraMap);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::cameraVisibleClicked, _mapFrame, &MapFrame::setCameraVisible);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::cameraSelectClicked, _mapFrame, &MapFrame::setCameraSelect);
        connect(_mapFrame, &MapFrame::cameraSelectToggled, _ribbonTabBattleView, &RibbonTabBattleView::setCameraSelect);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::cameraEditClicked, _mapFrame, &MapFrame::setCameraEdit);
        connect(_mapFrame, &MapFrame::cameraEditToggled, _ribbonTabBattleView, &RibbonTabBattleView::setCameraEdit);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::pointerClicked, _mapFrame, &MapFrame::setPointerOn);
        connect(_mapFrame, &MapFrame::pointerToggled, _ribbonTabBattleView, &RibbonTabBattleView::setPointerOn);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::distanceClicked, _mapFrame, &MapFrame::setDistance);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::freeDistanceClicked, _mapFrame, &MapFrame::setFreeDistance);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::distanceScaleChanged, _mapFrame, &MapFrame::setDistanceScale);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::distanceLineColorChanged, _mapFrame, &MapFrame::setDistanceLineColor);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::distanceLineTypeChanged, _mapFrame, &MapFrame::setDistanceLineType);
        connect(_ribbonTabBattleView, &RibbonTabBattleView::distanceLineWidthChanged, _mapFrame, &MapFrame::setDistanceLineWidth);
        connect(_mapFrame, &MapFrame::showDistanceChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceOn);
        connect(_mapFrame, &MapFrame::showFreeDistanceChanged, _ribbonTabBattleView, &RibbonTabBattleView::setFreeDistanceOn);
        connect(_mapFrame, &MapFrame::distanceScaleChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceScale);
        connect(_mapFrame, &MapFrame::distanceChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistance);
        connect(_mapFrame, &MapFrame::distanceLineColorChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceLineColor);
        connect(_mapFrame, &MapFrame::distanceLineTypeChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceLineType);
        connect(_mapFrame, &MapFrame::distanceLineWidthChanged, _ribbonTabBattleView, &RibbonTabBattleView::setDistanceLineWidth);
    }
}

bool MainWindow::doSaveCampaign(QString defaultFile)
{
    if(!_campaign)
        return true;

    _campaign->validateCampaignIds();
    if(!_campaign->isValid())
    {
        QMessageBox::StandardButton result = QMessageBox::critical(this,
                                                                   QString("Invalid Campaign"),
                                                                   QString("An invalid structure has been detected in the open campaign. Saving may corrupt the file and lead to data loss.\n\nIt is strongly recommended to back up your campaign file before saving!\n\nDo you wish to save now?"),
                                                                   QMessageBox::Yes | QMessageBox::No);
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

    if(_campaignFileName.isEmpty())
    {
        _campaignFileName = QFileDialog::getSaveFileName(this, QString("Save Campaign"), defaultFile, QString("XML files (*.xml)"));
        if(_campaignFileName.isEmpty())
            return false;
    }

    qDebug() << "[MainWindow] Saving Campaign: " << _campaignFileName;

    QDomDocument doc("DMHelperXML");

    QDomElement root = doc.createElement("root");
    doc.appendChild(root);

    QFileInfo fileInfo(_campaignFileName);
    QDir targetDir(fileInfo.absoluteDir());
    _campaign->outputXML(doc, root, targetDir, false);

    CampaignObjectBase* currentObject = ui->treeView->currentCampaignObject();
    if(currentObject)
    {
        QDomElement campaignElement = root.firstChildElement(QString("campaign"));
        if(!campaignElement.isNull())
            campaignElement.setAttribute("lastElement", currentObject->getID().toString());
    }

    QFile file(_campaignFileName);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "[MainWindow] Unable to open campaign file for writing: " << _campaignFileName;
        qDebug() << "       Error " << file.error() << ": " << file.errorString();
        QFileInfo info(file);
        qDebug() << "       Full filename: " << info.absoluteFilePath();

        _campaignFileName.clear();
        return false;
    }

    QTextStream ts(&file);
    ts.setEncoding(QStringConverter::Utf8);
    ts << doc.toString();

    file.close();

    clearDirty();

    qDebug() << "[MainWindow] Campaign saved: " << _campaignFileName;

    if(_options->getMRUHandler())
        _options->getMRUHandler()->addMRUFile(_campaignFileName);

    // Optionally save Bestiary and Spellbook here
    if((Bestiary::Instance()) && (Bestiary::Instance()->isDirty()))
    {
        if(QMessageBox::critical(this,
                                 QString("Save Bestiary"),
                                 QString("The Bestiary has been changed. Would you like to save it as well?"),
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            writeBestiary();
    }

    if((Spellbook::Instance()) && (Spellbook::Instance()->isDirty()))
    {
        if(QMessageBox::critical(this,
                                 QString("Save Spellbook"),
                                 QString("The Spellbook has been changed. Would you like to save it as well?"),
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            writeSpellbook();
    }

    return true;
}

void MainWindow::deleteCampaign()
{
    if(_pubWindow)
        _pubWindow->setRenderer(nullptr);

    if(_treeModel)
    {
        // Deselect the current object
        handleTreeItemSelected(QModelIndex(), ui->treeView->currentIndex());
        // Clear the tree model
        _treeModel->clear();
    }

    if(_campaign)
    {
        Campaign* oldCampaign = _campaign;
        _campaign = nullptr;
        emit campaignLoaded(nullptr);

        // Clear the campaign itself
        delete oldCampaign;
    }

    // Ensure the file name is removed
    _campaignFileName.clear();
}

void MainWindow::enableCampaignMenu()
{
    _ribbonTabCampaign->setCampaignEnabled(_campaign != nullptr);
}

bool MainWindow::selectIndex(const QModelIndex& index)
{
    if((!index.isValid()) || (!_treeModel))
        return false;

    ui->treeView->setCurrentIndex(index);
    activateWindow();
    return true;
}

bool MainWindow::selectItem(QUuid itemId)
{
    if((!_treeModel) || (itemId.isNull()))
        return false;

    return selectIndex(_treeModel->getObjectIndex(itemId));
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

    if(!Bestiary::Instance()->isDirty())
    {
        qDebug() << "[MainWindow] Bestiary has not been changed, no file will be written";
        return;
    }

    QString bestiaryFileName = _options->getBestiaryFileName();
    if(bestiaryFileName.isEmpty())
    {
        bestiaryFileName = QFileDialog::getSaveFileName(this, QString("Save Bestiary"), QString(), QString("XML files (*.xml)"));
        if(bestiaryFileName.isEmpty())
            return;

        _options->setBestiaryFileName(bestiaryFileName);
    }

    if(Bestiary::Instance()->writeBestiary(bestiaryFileName))
        qDebug() << "[MainWindow] Bestiary file writing complete: " << bestiaryFileName;
    else
        qDebug() << "[MainWindow] ERROR: Bestiary file writing failed: " << bestiaryFileName;
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

    if(!Spellbook::Instance()->isDirty())
    {
        qDebug() << "[MainWindow] Spellbook has not been changed, no file will be written";
        return;
    }

    QString spellbookFileName = _options->getSpellbookFileName();
    if(spellbookFileName.isEmpty())
    {
        spellbookFileName = QFileDialog::getSaveFileName(this, QString("Save Spellbook"), QString(), QString("XML files (*.xml)"));
        if(spellbookFileName.isEmpty())
            return;

        _options->setSpellbookFileName(spellbookFileName);
    }

    if(Spellbook::Instance()->writeSpellbook(spellbookFileName))
        qDebug() << "[MainWindow] Spellbook file writing complete: " << spellbookFileName;
    else
        qDebug() << "[MainWindow] ERROR: Spellbook file writing failed: " << spellbookFileName;
}

CampaignObjectBase* MainWindow::newEncounter(int encounterType, const QString& dialogTitle, const QString& dialogText)
{
    if((!_campaign)||(!_treeModel))
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
    if(!_campaign || audioFile.isEmpty())
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

Layer* MainWindow::selectMapFile()
{
    QString filename = QFileDialog::getOpenFileName(this, QString("Select Map File..."));
    if(filename.isEmpty())
        return nullptr;

    QMimeType mimeType = QMimeDatabase().mimeTypeForFile(filename);
    if(mimeType.isValid())
    {
        if(mimeType.name().startsWith("image/"))
        {
            QImageReader reader(filename);
            if(reader.canRead())
                return new LayerImage(QString("Map Image: ") + QFileInfo(filename).fileName(), filename);
        }
        else if(mimeType.name().startsWith("video/"))
        {
            return new LayerVideo(QString("Map Video: ") + QFileInfo(filename).fileName(), filename);
        }
    }

    return nullptr;
}

void MainWindow::openCampaign(const QString& filename)
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
    in.setEncoding(QStringConverter::Utf8);
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

    // Character template compatibility check
    int majorVersion = campaignElement.attribute("majorVersion", QString::number(0)).toInt();
    int minorVersion = campaignElement.attribute("minorVersion", QString::number(0)).toInt();
    if((majorVersion < 2) || ((majorVersion == 2) && (minorVersion < 4)))
    {
        qDebug() << "[Campaign] WARNING: Campaign file is an older format, informing user of character template conversion.";
        QMessageBox::StandardButton result = QMessageBox::critical(this,
                                                                   QString("Campaign file version check"),
                                                                   QString("PLEASE READ: IMPORTANT!") + QChar::LineFeed + QChar::LineFeed +
                                                                       QString("Starting with version 3.3, DMHelper has a more flexible character data and UI template system to support different game systems. As a result, some of the previously built-in 5E assumptions and math for characters, such as ability modifiers, saving throws and proficiency modifiers are no longer automatically applied.") + QChar::LineFeed + QChar::LineFeed +
                                                                       QString("DM Helper will try to update the math to transition to the new format, but we encourage you to double-check the PC and NPC stats. ") + QChar::LineFeed + QChar::LineFeed +
                                                                       QString("If you are using the automatic D&D Beyond importer, it has been updated to reflect the new formats and has even gotten a few improvements along the way. We would recommend updating your D&D Beyond characters in the app.") + QChar::LineFeed + QChar::LineFeed +
                                                                       QString("Once you save the campaign file again, it will be stored in the new format going forward.") + QChar::LineFeed + QChar::LineFeed +
                                                                       QString("Do you want to continue opening this campaign file?"),
                                                                   QMessageBox::Yes | QMessageBox::No);
        if(result == QMessageBox::No)
        {
            qDebug() << "[Campaign] INFO: User chose not to open campaign file due to version incompatibility: " << majorVersion << "." << minorVersion << ", " << filename;
            return;
        }
    }

    QUuid lastElementId = QUuid(campaignElement.attribute(QString("lastElement")));

    _campaignFileName = filename;
    QFileInfo fileInfo(_campaignFileName);
    QDir::setCurrent(fileInfo.absolutePath());
    _campaign = new Campaign();

    Bestiary::Instance()->startBatchProcessing();
    _campaign->inputXML(campaignElement, false);
    _campaign->postProcessXML(campaignElement, false);
    Bestiary::Instance()->finishBatchProcessing();

    if(!_campaign->isValid())
    {
        QMessageBox::StandardButton result = QMessageBox::critical(this,
                                                                   QString("Invalid Campaign"),
                                                                   QString("The loaded campaign has an invalid structure: there is a high risk of data loss and/or program malfunction! Would you like to continue?"),
                                                                   QMessageBox::Yes | QMessageBox::No);
        if(result == QMessageBox::No)
        {
            QMessageBox::information(this,
                                     QString("Invalid Campaign"),
                                     QString("The campaign has not been opened."));
            qDebug() << "[MainWindow] Invalid campaign discarded";
            delete _campaign;
            _campaign = nullptr;
            return;
        }
    }

    // The campaign file seems good, back it up!
    _options->backupFile(_campaignFileName);

    selectItem(DMHelper::TreeType_Campaign, QUuid());
    emit campaignLoaded(_campaign);
    if(!lastElementId.isNull())
        selectItem(lastElementId);

    if(_options->getMRUHandler())
        _options->getMRUHandler()->addMRUFile(filename);

    clearDirty();
}

void MainWindow::handleCampaignLoaded(Campaign* campaign)
{
    qDebug() << "[MainWindow] Campaign Loaded: " << _campaignFileName;

    // don't need this, the setcampaign below should do it: updateCampaignTree();
    updateMapFiles();
    updateClock();

    _activeItems->clear();
    _treeModel->setCampaign(campaign);

    ui->treeView->setMinimumWidth(ui->treeView->sizeHint().width());

    if(campaign)
    {
        QModelIndex firstIndex = _treeModel->index(0, 0);
        if(firstIndex.isValid())
            selectIndex(firstIndex); //ui->treeView->setCurrentIndex(firstIndex); // Activate the first entry in the tree
        else
            ui->stackedWidgetEncounter->setCurrentFrame(DMHelper::CampaignType_Base); // ui->stackedWidgetEncounter->setCurrentIndex(0);
        connect(campaign, &Campaign::dirty, this, &MainWindow::setDirty);
        connect(campaign, &Campaign::nameChanged, this, &MainWindow::setDirty);

        _characterFrame->loadCharacterUITemplate(campaign->getRuleset().getCharacterUIFile());
        connect(&campaign->getRuleset(), &Ruleset::initiativeRuleChanged, _battleFrame, &BattleFrame::initiativeRuleChanged);
        connect(&campaign->getRuleset(), &Ruleset::initiativeRuleChanged, _battleFrame, &BattleFrame::initiativeRuleChanged);
        connect(&campaign->getRuleset(), &Ruleset::characterUIFileChanged, _characterFrame, &CharacterTemplateFrame::loadCharacterUITemplate);

        connect(campaign, &Campaign::nameChanged, [=](CampaignObjectBase* object, const QString& name) {Q_UNUSED(object); setWindowTitle(QString("DMHelper - ") + name + QString("[*]")); });
        setWindowTitle(QString("DMHelper - ") + campaign->getName() + QString("[*]"));

        _ribbon->setCurrentIndex(1); // Shift to the Campaign tab
        QList<CampaignObjectBase*> parties = campaign->getChildObjectsByType(DMHelper::CampaignType_Party);
        for(CampaignObjectBase* party : parties)
            _ribbonTabWorldMap->registerPartyIcon(dynamic_cast<Party*>(party));
    }
    else
    {
        setWindowTitle(QString("DMHelper [*]"));
        ui->stackedWidgetEncounter->setEnabled(true);
        // Deactivate the currently selected object
        deactivateObject();
        activateWidget(DMHelper::CampaignType_WelcomeScreen);// ui->stackedWidgetEncounter->setCurrentIndex(DMHelper::EncounterType_WelcomeScreen);
        setRibbonToType(DMHelper::CampaignType_WelcomeScreen);
        _ribbon->setCurrentIndex(0); // Shift to the File tab
        _ribbonTabCampaign->setAddPCButton(false);
        _ribbonTabWorldMap->clearPartyIcons();
    }

    enableCampaignMenu();
}

void MainWindow::updateCampaignTree()
{
    qDebug() << "[MainWindow] Updating Campaign Tree";
    if(_treeModel)
        _treeModel->refresh();
}

void MainWindow::updateMapFiles()
{
    if(!_campaign)
        return;

    QFileInfo fileInfo(_campaignFileName);
    QDir sourceDir(fileInfo.absoluteDir());

    QList<Map*> mapList = _campaign->findChildren<Map*>();
    for(Map* map : mapList)
    {
        if((map) && (!map->getFileName().isEmpty()))
            map->setFileName(QDir::cleanPath(sourceDir.absoluteFilePath(map->getFileName())));
    }
}

void MainWindow::updateClock()
{
    if(!_timeAndDateFrame)
        return;

    if(_campaign)
    {
        connect(_timeAndDateFrame, SIGNAL(dateChanged(BasicDate)), _campaign, SLOT(setDate(BasicDate)));
        connect(_campaign, SIGNAL(dateChanged(BasicDate)), _timeAndDateFrame, SLOT(setDate(BasicDate)));
        connect(_timeAndDateFrame, SIGNAL(timeChanged(QTime)), _campaign, SLOT(setTime(QTime)));
        connect(_campaign, SIGNAL(timeChanged(QTime)), _timeAndDateFrame, SLOT(setTime(QTime)));

        _timeAndDateFrame->setDate(_campaign->getDate());
        _timeAndDateFrame->setTime(_campaign->getTime());
    }
    else
    {
        _timeAndDateFrame->setDate(BasicDate(1, 1, 0));
        _timeAndDateFrame->setTime(QTime(0, 0));
    }
}

void MainWindow::handleCustomContextMenu(const QPoint& point)
{
    // TODO: PROPERLY!
    if(!_treeModel)
        return;

    QModelIndex index = ui->treeView->indexAt(point);
    if(!index.isValid())
        return;

    CampaignTreeItem* campaignItem = _treeModel->campaignItemFromIndex(index);
    if(!campaignItem)
        return;

    CampaignObjectBase* campaignObject = campaignItem->getCampaignItemObject();
    if(!campaignObject)
        return;

    QMenu* contextMenu = new QMenu(ui->treeView);

    // New text entry
    QAction* addTextEntry = new QAction(QIcon(":/img/data/icon_newtextencounter.png"), QString("New Entry"), contextMenu);
    connect(addTextEntry, SIGNAL(triggered()), this, SLOT(newTextEncounter()));
    contextMenu->addAction(addTextEntry);

    // New linked text entry
    QAction* addLinkedEntry = new QAction(QIcon(":/img/data/icon_newlink.png"), QString("New Linked Entry"), contextMenu);
    connect(addLinkedEntry, SIGNAL(triggered()), this, SLOT(newLinkedText()));
    contextMenu->addAction(addLinkedEntry);

    // New media
    QAction* addMedia = new QAction(QIcon(":/img/data/icon_newmedia.png"), QString("New Media"), contextMenu);
    connect(addMedia, SIGNAL(triggered()), this, SLOT(newMedia()));
    contextMenu->addAction(addMedia);

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

    QAction* setIconItem = new QAction(QIcon(":/img/data/icon_contentscrollingtext.png"), QString("Set Icon..."), contextMenu);
    connect(setIconItem, SIGNAL(triggered()), this, SLOT(setCurrentItemIcon()));
    contextMenu->addAction(setIconItem);

    if(!campaignObject->getIconFile().isEmpty())
    {
        QAction* clearIconItem = new QAction(QIcon(":/img/data/icon_contentscrollingtext.png"), QString("Clear Icon"), contextMenu);
        connect(clearIconItem, SIGNAL(triggered()), this, SLOT(clearCurrentItemIcon()));
        contextMenu->addAction(clearIconItem);
    }

    if(campaignItem->isEditable())
    {
        QAction* editItem = new QAction(QIcon(":/img/data/icon_edit.png"), QString("Edit Item"), contextMenu);
        connect(editItem, SIGNAL(triggered()), this, SLOT(editCurrentItem()));
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
    if((!item) || (!_campaign))
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
    CampaignTreeItem* item = _treeModel->campaignItemFromIndex(current);
    CampaignObjectBase* itemObject = nullptr;

    if(item)
    {
        // Add the item to the undo stack for IDs
        _activeItems->push(item->getCampaignItemId());

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
    CampaignTreeItem* item = _treeModel->campaignItemFromIndex(index);
    if(!item)
        return;

    CampaignObjectBase* object = item->getCampaignItemObject();
    if(!object)
        return;

    object->setExpanded(expanded);
}

void MainWindow::handleEditSettings()
{
    _options->editSettings(_campaign);
}

void MainWindow::handleOpenDMScreen()
{
    if(!_dmScreenDlg)
        _dmScreenDlg = createDialog(new DMScreenTabWidget(_options->getEquipmentFileName(), this), QSize(width() * 9 / 10, height() * 9 / 10));

    _dmScreenDlg->exec();
}

void MainWindow::handleOpenTables()
{
    if(!_tableDlg)
        _tableDlg = createDialog(new CustomTableFrame(_options->getTablesDirectory(), this), QSize(width() * 9 / 10, height() * 9 / 10));

    _tableDlg->exec();
}

void MainWindow::handleOpenSoundboard()
{
    if(!_soundDlg)
    {
        SoundboardFrame* soundboard = new SoundboardFrame(this);
        connect(this, SIGNAL(campaignLoaded(Campaign*)), soundboard, SLOT(setCampaign(Campaign*)));
        connect(this, SIGNAL(audioTrackAdded(AudioTrack*)), soundboard, SLOT(addTrackToTree(AudioTrack*)));
        connect(soundboard, SIGNAL(trackCreated(CampaignObjectBase*)), this, SLOT(addNewObject(CampaignObjectBase*)));
        // TODO:    connect(soundboard, SIGNAL(_dirty()), this, SLOT(setDirty()));
        _soundDlg = createDialog(soundboard, QSize(width() * 9 / 10, height() * 9 / 10));
    }

    _soundDlg->exec();
}

void MainWindow::handleOpenCalendar()
{
    if((!_calendarDlg) || (!_timeAndDateFrame))
    {
        _timeAndDateFrame = new TimeAndDateFrame(this);
        _calendarDlg = createDialog(_timeAndDateFrame, QSize(width() / 2, height() * 9 / 10));
        updateClock();
    }

    _calendarDlg->exec();
}

void MainWindow::handleOpenCountdown()
{
    if(!_countdownDlg)
        _countdownDlg = createDialog(new CountdownFrame(this));

    _countdownDlg->exec();
}

void MainWindow::handleOpenGlobalSearch()
{
    if((!_globalSearchDlg) || (!_globalSearchFrame))
    {
        _globalSearchFrame = new GlobalSearchFrame(this);
        _globalSearchDlg = createDialog(_globalSearchFrame, QSize(width() / 2, height() * 9 / 10));
        connect(_globalSearchFrame, &GlobalSearchFrame::frameAccept, _globalSearchDlg, &QDialog::accept);
        connect(_globalSearchFrame, &GlobalSearchFrame::campaignObjectSelected, this, &MainWindow::selectItemFromStack);
        connect(_globalSearchFrame, &GlobalSearchFrame::monsterSelected, this, &MainWindow::openMonster);
        connect(_globalSearchFrame, &GlobalSearchFrame::spellSelected, this, &MainWindow::openSpell);
        connect(_globalSearchFrame, &GlobalSearchFrame::toolSelected, this, &MainWindow::openQuickref);
    }

    _globalSearchFrame->setCampaign(_campaign);
    _globalSearchDlg->exec();
}

void MainWindow::handleAutoSaveExpired()
{
    if((_campaign) && (_dirty) && (_options->getAutoSave()))
        saveCampaign();

    handleAutoSaveChanged();
}

void MainWindow::handleAutoSaveChanged()
{
    if(!_campaign)
        return;

    if(_options->getAutoSave())
    {
        if(!_autoSaveTimer)
        {
            _autoSaveTimer = new QTimer(this);
            _autoSaveTimer->setSingleShot(true);
            connect(_autoSaveTimer, &QTimer::timeout, this, &MainWindow::handleAutoSaveExpired);
        }

        _autoSaveTimer->start(AUTOSAVE_TIMER_INTERVAL);

    }
    else
    {
        if(_autoSaveTimer)
            _autoSaveTimer->stop();
    }
}

void MainWindow::handleAnimationStarted()
{
    if(_pubWindow)
        _pubWindow->setBackgroundColor();
    _animationFrameCount = DMHelper::ANIMATION_TIMER_PREVIEW_FRAMES;
}

bool MainWindow::selectItemFromStack(const QUuid& itemId)
{
    if((!_treeModel) || (itemId.isNull()))
        return false;

    QModelIndex itemIndex = _treeModel->getObjectIndex(itemId);
    if(!itemIndex.isValid())
        return false;

    ui->treeView->setCurrentIndex(itemIndex);
    activateWindow();
    return true;
}

void MainWindow::openBestiary()
{
    qDebug() << "[MainWindow] Opening Bestiary";
    if(!Bestiary::Instance())
        return;

    if(Bestiary::Instance()->count() == 0)
    {
        qDebug() << "[MainWindow]    ...Bestiary is empty, creating a first monster";
        _bestiaryDlg.createNewMonster();
    }
    else
    {
        _bestiaryDlg.setFocus();
        _bestiaryDlg.show();
        _bestiaryDlg.activateWindow();
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

    QString filename = QFileDialog::getOpenFileName(this, QString("Select exported file for import"), QString(), QString("XML files (*.xml)"));
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
        in.setEncoding(QStringConverter::Utf8);
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

        Bestiary::Instance()->inputXML(root, filename);
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
        _spellDlg.createNewSpell();
    }
    else
    {
        _spellDlg.setFocus();
        _spellDlg.show();
        _spellDlg.activateWindow();
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

void MainWindow::openHelpDialog()
{
    qDebug() << "[MainWindow] Opening Help Dialog";

    HelpDialog dlg;
    connect(&dlg, &HelpDialog::openGettingStarted, this, &MainWindow::openGettingStarted);
    connect(&dlg, &HelpDialog::openUsersGuide, this, &MainWindow::openUsersGuide);
    connect(&dlg, &HelpDialog::openBackupDirectory, this, &MainWindow::openBackupDirectory);
    connect(&dlg, &HelpDialog::openLogsDirectory, this, &MainWindow::openLogsDirectory);
    dlg.exec();
}

void MainWindow::openBackupDirectory()
{
    QString backupPath = _options->getStandardDirectory("backup");
    if(!backupPath.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(backupPath));
}

void MainWindow::openLogsDirectory()
{
    QString logsPath = DMHLogger::getLogDirPath();
    if(!logsPath.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(logsPath));
}

void MainWindow::openRandomMarkets()
{
    RandomMarketDialog dlg(_options->getShopsFileName());
    dlg.exec();
}

void MainWindow::configureGridLock()
{
    ConfigureLockedGridDialog dlg;
    QScreen* primary = QGuiApplication::primaryScreen();
    if(primary)
        dlg.resize(primary->availableSize().width() * 3 / 4, primary->availableSize().height() * 2 / 3);

    dlg.setGridScale(_options->getGridLockScale());

    if(dlg.exec() == QDialog::Accepted)
        _options->setGridLockScale(dlg.getGridScale());
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

        Layer* selectedLayer = model->getLayerScene().getSelectedLayer();
        LayerGrid* gridLayer = dynamic_cast<LayerGrid*>(model->getLayerScene().getNearest(selectedLayer, DMHelper::LayerType_Grid));
        if(gridLayer)
            _ribbonTabBattleMap->setGridConfig(gridLayer->getConfig());
            /*
        {
            //_ribbonTabBattleMap->setGridOn(model->getGridOn());
            _ribbonTabBattleMap->setGridType(layer->getConfig().getGridType());
            _ribbonTabBattleMap->setGridScale(layer->getConfig().getGridScale());
            _ribbonTabBattleMap->setGridAngle(layer->getConfig().getGridAngle());
            _ribbonTabBattleMap->setGridXOffset(layer->getConfig().getGridOffsetX());
            _ribbonTabBattleMap->setGridYOffset(layer->getConfig().getGridOffsetY());
            _ribbonTabBattleMap->setGridWidth(layer->getConfig().getGridPen().width());
            _ribbonTabBattleMap->setGridColor(layer->getConfig().getGridPen().color());
        }
        */
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
    CampaignObjectFrame* objectFrame = dynamic_cast<CampaignObjectFrame*>(ui->stackedWidgetEncounter->currentWidget());
    if(objectFrame)
    {
        disconnect(_ribbon->getPublishRibbon(), &PublishButtonProxy::layersClicked, objectFrame, &CampaignObjectFrame::editLayers);
        disconnect(_ribbon->getPublishRibbon(), SIGNAL(clicked(bool)), objectFrame, SLOT(publishClicked(bool)));
        disconnect(_ribbon->getPublishRibbon(), SIGNAL(rotationChanged(int)), objectFrame, SLOT(setRotation(int)));
        disconnect(_ribbon->getPublishRibbon(), SIGNAL(colorChanged(const QColor&)), objectFrame, SLOT(setBackgroundColor(const QColor&)));
        disconnect(_ribbon->getPublishRibbon(), &PublishButtonProxy::clicked, ui->treeView, &CampaignTree::publishCurrent);

        disconnect(objectFrame, SIGNAL(setPublishEnabled(bool, bool)), _ribbon->getPublishRibbon(), SLOT(setPublishEnabled(bool, bool)));
        disconnect(objectFrame, SIGNAL(checkedChanged(bool)), _ribbon->getPublishRibbon(), SLOT(setChecked(bool)));
        disconnect(objectFrame, SIGNAL(checkableChanged(bool)), _ribbon->getPublishRibbon(), SLOT(setCheckable(bool)));
        disconnect(objectFrame, SIGNAL(rotationChanged(int)), _ribbon->getPublishRibbon(), SLOT(setRotation(int)));
        disconnect(objectFrame, SIGNAL(backgroundColorChanged(const QColor&)), _ribbon->getPublishRibbon(), SLOT(setColor(const QColor&)));

        objectFrame->deactivateObject();
    }
}

void MainWindow::activateWidget(int objectType, CampaignObjectBase* object)
{
    CampaignObjectFrame* objectFrame = ui->stackedWidgetEncounter->setCurrentFrame(objectType);
    if(objectFrame)
    {
        connect(_ribbon->getPublishRibbon(), &PublishButtonProxy::layersClicked, objectFrame, &CampaignObjectFrame::editLayers);
        connect(_ribbon->getPublishRibbon(), SIGNAL(clicked(bool)), objectFrame, SLOT(publishClicked(bool)));
        connect(_ribbon->getPublishRibbon(), SIGNAL(rotationChanged(int)), objectFrame, SLOT(setRotation(int)));
        connect(_ribbon->getPublishRibbon(), SIGNAL(colorChanged(const QColor&)), objectFrame, SLOT(setBackgroundColor(const QColor&)));
        connect(_ribbon->getPublishRibbon(), &PublishButtonProxy::clicked, ui->treeView, &CampaignTree::publishCurrent);

        connect(objectFrame, SIGNAL(setPublishEnabled(bool, bool)), _ribbon->getPublishRibbon(), SLOT(setPublishEnabled(bool, bool)));
        connect(objectFrame, SIGNAL(checkedChanged(bool)), _ribbon->getPublishRibbon(), SLOT(setChecked(bool)));
        connect(objectFrame, SIGNAL(checkableChanged(bool)), _ribbon->getPublishRibbon(), SLOT(setCheckable(bool)));
        connect(objectFrame, SIGNAL(rotationChanged(int)), _ribbon->getPublishRibbon(), SLOT(setRotation(int)));
        connect(objectFrame, SIGNAL(backgroundColorChanged(const QColor&)), _ribbon->getPublishRibbon(), SLOT(setColor(const QColor&)));

        objectFrame->activateObject(object, _pubWindow ? _pubWindow->getRenderer() : nullptr);
        if(_ribbon && _ribbon->getPublishRibbon())
        {
            objectFrame->setRotation(_ribbon->getPublishRibbon()->getRotation());
            _ribbon->getPublishRibbon()->setChecked(object && _pubWindow && (object->getID() == _pubWindow->getObjectId()));
        }
    }
}

void MainWindow::setRibbonToType(int objectType)
{
    switch(objectType)
    {
        case DMHelper::CampaignType_Battle:
        case DMHelper::CampaignType_BattleContent:
            _ribbon->enableTab(_ribbonTabBattleView); // note: order is important vs map and reuse
            connectBattleView(true);
            _ribbon->enableTab(_ribbonTabBattleMap);
            _ribbon->enableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabMap);
            _ribbon->disableTab(_ribbonTabWorldMap);
            _ribbon->disableTab(_ribbonTabText);
            _ribbon->disableTab(_ribbonTabAudio);
            break;
        case DMHelper::CampaignType_Map:
            _ribbon->enableTab(_ribbonTabBattleView); // note: order is important vs battle and reuse
            connectBattleView(false);
            _ribbon->enableTab(_ribbonTabMap);
            _ribbon->enableTab(_ribbonTabWorldMap);
            _ribbon->disableTab(_ribbonTabBattleMap);
            _ribbon->disableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabText);
            _ribbon->disableTab(_ribbonTabAudio);
            break;
        case DMHelper::CampaignType_Campaign:
        case DMHelper::CampaignType_Text:
        case DMHelper::CampaignType_LinkedText:
            _ribbon->enableTab(_ribbonTabText);
            _ribbon->disableTab(_ribbonTabBattleMap);
            _ribbon->disableTab(_ribbonTabBattleView);
            _ribbon->disableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabMap);
            _ribbon->disableTab(_ribbonTabWorldMap);
            _ribbon->disableTab(_ribbonTabAudio);
            break;
        case DMHelper::CampaignType_AudioTrack:
            _ribbon->enableTab(_ribbonTabAudio);
            _ribbon->disableTab(_ribbonTabBattleMap);
            _ribbon->disableTab(_ribbonTabBattleView);
            _ribbon->disableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabMap);
            _ribbon->disableTab(_ribbonTabWorldMap);
            _ribbon->disableTab(_ribbonTabText);
            break;
        case DMHelper::CampaignType_Party:
        case DMHelper::CampaignType_Placeholder:
        case DMHelper::CampaignType_Base:
        case DMHelper::CampaignType_WelcomeScreen:
        case DMHelper::CampaignType_Combatant:
        default:
            _ribbon->disableTab(_ribbonTabBattleMap);
            _ribbon->disableTab(_ribbonTabBattleView);
            _ribbon->disableTab(_ribbonTabBattle);
            _ribbon->disableTab(_ribbonTabMap);
            _ribbon->disableTab(_ribbonTabWorldMap);
            _ribbon->disableTab(_ribbonTabText);
            _ribbon->disableTab(_ribbonTabAudio);
            break;
    }
}
