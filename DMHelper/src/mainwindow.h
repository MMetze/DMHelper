#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "campaignobjectbase.h"
#include "bestiarydialog.h"
#include "spellbookdialog.h"
#include "dmconstants.h"
#include "optionscontainer.h"
#include <QMainWindow>
#include <QLabel>
#include <QFileInfo>
#include <QList>
#include <QMultiMap>
#include <QUuid>

class PublishWindow;
class Campaign;
class Character;
class Adventure;
class Encounter;
class EncounterTextEdit;
class TimeAndDateFrame;
class Map;
class MRUHandler;
class QStandardItem;
class CampaignTreeModel;
class QVBoxLayout;
class QItemSelection;
class BattleDialogManager;
class AudioPlayer;
class AudioTrack;
class PublishFrame;
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

    static const int DEFAULT_MRU_FILE_COUNT = 4;

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

    void newCharacter();
    void importCharacter();
    void importItem();
    void newParty();
    void newTextEncounter();
    void newBattleEncounter();
    void newMap();
    void newAudioEntry();
    void newSyrinscapeEntry();
    void newYoutubeEntry();
    void removeCurrentItem();
    void editCurrentItem();
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

protected:
    virtual void showEvent(QShowEvent * event);
    virtual void closeEvent(QCloseEvent * event);
    virtual void resizeEvent(QResizeEvent *event);

    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void mouseMoveEvent(QMouseEvent * event);

    virtual void keyPressEvent(QKeyEvent *event);

    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

    void setupRibbonBar();

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

protected slots:
    void openFile(const QString& filename);
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

    void handleAnimationStarted();

    // Bestiary
    void openBestiary();
    void exportBestiary();
    void importBestiary();

    void openSpellbook();
    void exportSpellbook();
    void importSpellbook();

    void openAboutDialog();
    void openRandomMarkets();

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
    QDialog* _quickRefDlg;
    QDialog* _soundDlg;
    TimeAndDateFrame* _timeAndDateFrame;
    QDialog* _calendarDlg;
    QDialog* _countdownDlg;

    EncounterTextEdit* _encounterTextEdit;

    CampaignTreeModel* _treeModel;
    QVBoxLayout* _characterLayout;
    Campaign* _campaign;
    QString _campaignFileName;

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
};

#endif // MAINWINDOW_H
