#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "campaignobjectbase.h"
#include "bestiarydialog.h"
#include "spellbookdialog.h"
#include "dmconstants.h"
#include "optionscontainer.h"
#include "campaigntreeactivestack.h"
#include <QMainWindow>
#include <QLabel>
#include <QList>
#include <QMultiMap>
#include <QUuid>

class PublishWindow;
class Campaign;
class Characterv2;
class Adventure;
class Encounter;
class EncounterTextEdit;
class TimeAndDateFrame;
class GlobalSearchFrame;
class Map;
class Layer;
class MRUHandler;
class QStandardItem;
class CampaignTreeModel;
class QVBoxLayout;
class QItemSelection;
class BattleDialogManager;
class AudioPlayer;
class AudioTrack;
class PublishFrame;
class QuickRefFrame;
class RibbonMain;
class RibbonTabFile;
class RibbonTabCampaign;
class RibbonTabTools;
class RibbonTabBattleMap;
class RibbonTabBattleView;
class RibbonTabBattle;
class RibbonTabText;
class BattleDialogModel;
class RibbonTabMap;
class RibbonTabWorldMap;
class RibbonTabAudio;
class BattleFrame;
class MapFrame;
class CharacterTemplateFrame;
#ifdef INCLUDE_NETWORK_SUPPORT
class NetworkController;
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static const int DEFAULT_MRU_FILE_COUNT = 10;

public slots:
    void newCampaign();
    bool saveCampaign();
    void saveCampaignAs();
    void openFileDialog();
    bool closeCampaign();
    void openDiceDialog();

    void openEncounter(QUuid id);
    void openCharacter(QUuid id);
    void openMonster(const QString& monsterClass);
    void openSpell(const QString& spellName);
    void openQuickref(const QString& quickRefSection);

    void newCharacter();
    void importCharacter();
    void importItem();
    void newParty();
    void newTextEncounter();
    void newLinkedText();
    void newBattleEncounter();
    void newMap(Layer* imageLayer = nullptr);
    void newMedia();
    void newAudioEntry();
    void newSyrinscapeEntry();
    void newSyrinscapeOnlineEntry();
    void newYoutubeEntry();
    void removeCurrentItem();
    void showNotes();
    void addNote();
    void editCurrentItem();
    void setCurrentItemIcon();
    void clearCurrentItemIcon();
    void exportCurrentItem();
    void addNewObject(CampaignObjectBase* newObject);

    void clearDirty();
    void setDirty();

    void checkForUpdates(bool silentUpdate = false);

    void showPublishWindow(bool visible = true);

    void linkActivated(const QUrl & link);

    // Bestiary
    void readBestiary();
    void readSpellbook();
    void readQuickRef();

signals:
    void campaignLoaded(Campaign* campaign);
    void dispatchPublishImage(QImage img);
    void dispatchPublishImage(QImage img, const QColor& color);

    void cancelSelect();

    void characterChanged(QUuid id);
    void audioTrackAdded(AudioTrack* track);

    void openGettingStarted();
    void openUsersGuide();

protected:
    virtual void showEvent(QShowEvent * event);
    virtual void closeEvent(QCloseEvent * event);
    virtual void resizeEvent(QResizeEvent *event);

    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void mouseMoveEvent(QMouseEvent * event);

    virtual void keyPressEvent(QKeyEvent *event);

    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

    void setupRibbonBar();
    void connectBattleView(bool toBattle);

    bool doSaveCampaign(QString defaultFile);
    void deleteCampaign();
    void enableCampaignMenu();

    bool selectIndex(const QModelIndex& index);
    bool selectItem(QUuid itemId);
    bool selectItem(int itemType, QUuid itemId);
    bool selectItem(int itemType, QUuid itemId, QUuid adventureId);

    // Bestiary
    void writeBestiary();
    void writeSpellbook();

    CampaignObjectBase* newEncounter(int encounterType, const QString& dialogTitle, const QString& dialogText);
    void addNewAudioObject(const QString& audioFile);
    Layer* selectMapFile();

protected slots:
    void openCampaign(const QString& filename);
    void handleCampaignLoaded(Campaign* campaign);
    void updateCampaignTree();
    void updateMapFiles();
    void updateClock();
    void handleCustomContextMenu(const QPoint& point);
    void handleTreeItemChanged(QStandardItem * item);
    void handleTreeItemSelected(const QModelIndex & current, const QModelIndex & previous);
    void handleTreeItemDoubleClicked(const QModelIndex & index);
    void handleTreeItemExpanded(const QModelIndex & index);
    void handleTreeItemCollapsed(const QModelIndex & index);
    void handleTreeStateChanged(const QModelIndex & index, bool expanded);
    void handleEditSettings();

    void handleOpenDMScreen();
    void handleOpenTables();
    void handleOpenSoundboard();
    void handleOpenCalendar();
    void handleOpenCountdown();
    void handleOpenGlobalSearch();

    void handleAutoSaveExpired();
    void handleAutoSaveChanged();
    void handleAnimationStarted();

    bool selectItemFromStack(const QUuid& itemId);

    // Bestiary
    void openBestiary();
    void exportBestiary();
    void importBestiary();

    void openSpellbook();
    void exportSpellbook();
    void importSpellbook();

    void openAboutDialog();
    void openHelpDialog();
    void openBackupDirectory();
    void openLogsDirectory();
    void openRandomMarkets();
    void configureGridLock();

    QDialog* createDialog(QWidget* contents, const QSize& dlgSize = QSize());

    void battleModelChanged(BattleDialogModel* model);
    void activateObject(CampaignObjectBase* object);
    void deactivateObject();
    void activateWidget(int objectType, CampaignObjectBase* object = nullptr);
    void setRibbonToType(int objectType);

private:
    Ui::MainWindow *ui;

    PublishWindow* _pubWindow;
    QDialog* _dmScreenDlg;
    QDialog* _tableDlg;
    QuickRefFrame* _quickRefFrame;
    QDialog* _quickRefDlg;
    QDialog* _soundDlg;
    TimeAndDateFrame* _timeAndDateFrame;
    QDialog* _calendarDlg;
    QDialog* _countdownDlg;
    GlobalSearchFrame* _globalSearchFrame;
    QDialog* _globalSearchDlg;

    EncounterTextEdit* _encounterTextEdit;

    CampaignTreeModel* _treeModel;
    CampaignTreeActiveStack* _activeItems;
    QVBoxLayout* _characterLayout;
    Campaign* _campaign;
    QString _campaignFileName;
    QTimer* _autoSaveTimer;

    OptionsContainer* _options;

    BestiaryDialog _bestiaryDlg;
    SpellbookDialog _spellDlg;

    BattleDialogManager* _battleDlgMgr;

    AudioPlayer* _audioPlayer;

#ifdef INCLUDE_NETWORK_SUPPORT
    NetworkController* _networkController;
#endif

    bool _mouseDown;
    QPoint _mouseDownPos;

    QAction* _undoAction;
    QAction* _redoAction;

    bool _initialized;
    bool _dirty;
    int _animationFrameCount;

    RibbonMain* _ribbon;
    RibbonTabFile* _ribbonTabFile;
    RibbonTabCampaign* _ribbonTabCampaign;
    RibbonTabTools* _ribbonTabTools;
    RibbonTabBattleMap* _ribbonTabBattleMap;
    RibbonTabBattleView* _ribbonTabBattleView;
    RibbonTabBattle* _ribbonTabBattle;
    RibbonTabText* _ribbonTabText;
    RibbonTabMap* _ribbonTabMap;
    RibbonTabWorldMap* _ribbonTabWorldMap;
    RibbonTabAudio* _ribbonTabAudio;

    BattleFrame* _battleFrame;
    MapFrame* _mapFrame;
    CharacterTemplateFrame* _characterFrame;
};

#endif // MAINWINDOW_H
