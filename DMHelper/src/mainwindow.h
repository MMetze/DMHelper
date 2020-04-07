#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "characterdialog.h"
#include "bestiarydialog.h"
#include "dmconstants.h"
#ifdef INCLUDE_CHASE_SUPPORT
#include "chasedialog.h"
#endif
#include "optionscontainer.h"
#include <QMainWindow>
#include <QLabel>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QUuid>

class PublishWindow;
class ScrollTabWidget;
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
class PublishFrame;
class DMHelperRibbon;
class RibbonTabFile;
class RibbonTabCampaign;
class RibbonTabBestiary;
class RibbonTabHelp;
class RibbonTabMap;
class RibbonTabBattle;
class BattleDialogModel;
class MapEditFrame;
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

    void openCharacter(QUuid id);
    void openMonster(const QString& monsterClass);

    void newCharacter();
    void importCharacter();
    //void TEST_DISCORD();
    void importItem();
    void newNPC();
    void importNPC();
    //void removeCurrentCharacter();
    void newAdventure();
    void newTextEncounter();
    void newBattleEncounter();
    void newScrollingTextEncounter();
    void newMap();
    void editCurrentMap();
    void removeCurrentItem();
    void editCurrentItem();
    void exportCurrentItem();

    void clearDirty();
    void setDirty();

    void checkForUpdates(bool silentUpdate = false);

    void showPublishWindow(bool visible = true);

    void linkActivated(const QUrl & link);

    // Bestiary
    void readBestiary();

signals:
    void campaignLoaded(Campaign* campaign);
    void dispatchPublishImage(QImage img, QColor color);
    void dispatchAnimateImage(QImage img);

    void cancelSelect();

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

    Encounter* notesFromIndex(const QModelIndex & index);
    Character* characterFromIndex(const QModelIndex & index);
    Adventure* adventureFromIndex(const QModelIndex & index);
    Encounter* encounterFromIndex(const QModelIndex & index);
    Map* mapFromIndex(const QModelIndex & index);
    bool selectItem(int itemType, QUuid itemId);
    bool selectItem(int itemType, QUuid itemId, QUuid adventureId);
    QStandardItem* findItem(QStandardItem* parent, int itemType, QUuid itemId);
    QStandardItem* findItem(QStandardItem* parent, QUuid itemId);
    QStandardItem* findParentbyType(QStandardItem* child, int parentType);
    void setIndexExpanded(bool expanded, const QModelIndex& index);

    // Bestiary
    void writeBestiary();

    void newEncounter(int encounterType);

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
    void handleEncounterTextChanged();
    void handleStartNewBattle();
    void handleLoadBattle();
    void handleDeleteBattle();

    void handleAnimationStarted(QColor color);
    void handleAnimationPreview(QImage img);

    // Bestiary
    void openBestiary();
    void exportBestiary();
    void importBestiary();

    void openAboutDialog();

    void openTextPublisher();
    void openTextTranslator();
    void openRandomMarkets();

    QDialog* createDialog(QWidget* contents);

    void battleModelChanged(BattleDialogModel* model);
    void activateObject(CampaignObjectBase* object);
    void activateWidget(int widgetId);
    int getWidgetFromType(int objectType);
    void setRibbonToType(int objectType);

#ifdef INCLUDE_CHASE_SUPPORT
    void startChase();
    void handleChaseComplete();
#endif

private:
    Ui::MainWindow *ui;

    PublishWindow* pubWindow;
    ScrollTabWidget* previewTab;
    PublishFrame* previewFrame;
    QDialog* previewDlg;
    QDialog* dmScreenDlg;
    QDialog* tableDlg;
    QDialog* quickRefDlg;
    QDialog* soundDlg;
    TimeAndDateFrame* timeAndDateFrame;
    QDialog* calendarDlg;
    QDialog* countdownDlg;

    EncounterTextEdit* encounterTextEdit;

    CampaignTreeModel* treeModel;
    QMap<QString, QModelIndex> treeIndexMap;
    QVBoxLayout* characterLayout;
    Campaign* campaign;
    QString campaignFileName;

    //QUuid currentCharacter;

    OptionsContainer* _options;

    BestiaryDialog bestiaryDlg;

#ifdef INCLUDE_CHASE_SUPPORT
    ChaseDialog* chaseDlg;
#endif

    BattleDialogManager* _battleDlgMgr;

    AudioPlayer* _audioPlayer;

#ifdef INCLUDE_NETWORK_SUPPORT
    NetworkController* _networkController;
#endif

    bool mouseDown;
    QPoint mouseDownPos;

    QAction* undoAction;
    QAction* redoAction;

    bool initialized;
    bool dirty;
    int _animationFrameCount;

    DMHelperRibbon* _ribbon;
    RibbonTabFile* _ribbonTabFile;
    RibbonTabCampaign* _ribbonTabCampaign;
    RibbonTabBestiary* _ribbonTabTools;
    RibbonTabHelp* _ribbonTabHelp;
    RibbonTabMap* _ribbonTabMap;
    RibbonTabBattle* _ribbonTabBattle;
    MapEditFrame* _ribbonTabMiniMap;
};

#endif // MAINWINDOW_H
