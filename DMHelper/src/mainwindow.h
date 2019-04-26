#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "characterdialog.h"
#include "bestiarydialog.h"
#include "dmscreendialog.h"
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
class QStandardItemModel;
class QVBoxLayout;
class QItemSelection;
class BattleDialogManager;
class AudioPlayer;
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
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static const int DEFAULT_MRU_FILE_COUNT = 4;

public slots:
    void newCampaign();
    bool saveCampaign();
    void saveCampaignAs();
    void openFileDialog();
    bool closeCampaign();
    void openDiceDialog();

    void openCharacter(int id);
    void openMonster(const QString& monsterClass);

    void newCharacter();
    void importCharacter();
    void newNPC();
    void removeCurrentCharacter();
    void newAdventure();
    void newTextEncounter();
    void newBattleEncounter();
    void newScrollingTextEncounter();
    void newMap();
    void editCurrentMap();
    void removeCurrentItem();
    void editCurrentItem();

    void clearDirty();
    void setDirty();

    void showPublishWindow();

    void linkActivated(const QUrl & link);

    // Bestiary
    void readBestiary();

signals:
    void openPreview();
    void campaignLoaded(Campaign* campaign);
    void dispatchPublishImage(QImage img);
    void dispatchAnimateImage(QImage img);

protected:
    virtual void showEvent(QShowEvent * event);
    virtual void closeEvent(QCloseEvent * event);
    virtual void resizeEvent(QResizeEvent *event);

    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void mouseMoveEvent(QMouseEvent * event);

    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

    void deleteCampaign();
    void enableCampaignMenu();

    Encounter* notesFromIndex(const QModelIndex & index);
    Character* characterFromIndex(const QModelIndex & index);
    Adventure* adventureFromIndex(const QModelIndex & index);
    Encounter* encounterFromIndex(const QModelIndex & index);
    Map* mapFromIndex(const QModelIndex & index);
    bool selectItem(int itemType, int itemId);
    bool selectItem(int itemType, int itemId, int adventureId);
    QStandardItem* findItem(QStandardItem* parent, int itemType, int itemId);
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

    // Bestiary
    void openBestiary();
    void openAboutDialog();

    void openDMScreen();
    void openTextPublisher();
    void openTextTranslator();

#ifdef INCLUDE_CHASE_SUPPORT
    void startChase();
    void handleChaseComplete();
#endif

private:
    Ui::MainWindow *ui;

    PublishWindow* pubWindow;
    ScrollTabWidget* previewTab;
    TimeAndDateFrame* timeAndDateFrame;

    EncounterTextEdit* encounterTextEdit;

    QStandardItemModel* treeModel;
    QMap<QString, QModelIndex> treeIndexMap;
    QVBoxLayout* characterLayout;
    Campaign* campaign;
    QString campaignFileName;

    int currentCharacter;

    OptionsContainer* _options;

    BestiaryDialog bestiaryDlg;

    DMScreenDialog dmScreenDlg;

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
};

#endif // MAINWINDOW_H
