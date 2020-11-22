#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "campaignobjectbase.h"
#include "bestiarydialog.h"
#include "spellbookdialog.h"
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
class EncounterScrollingTextEdit;
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
class RibbonMain;
class RibbonTabFile;
class RibbonTabCampaign;
class RibbonTabBestiary;
class RibbonTabMap;
class RibbonTabBattle;
class RibbonTabScrolling;
class RibbonTabText;
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
    void importItem();
    void newParty();
    void newTextEncounter();
    void newBattleEncounter();
    void newScrollingTextEncounter();
    void newMap();
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
    void readSpellbook();

signals:
    void campaignLoaded(Campaign* campaign);
    void dispatchPublishImage(QImage img);
    void dispatchPublishImage(QImage img, QColor color);
    void dispatchAnimateImage(QImage img);

    void cancelSelect();

    void characterChanged(QUuid id);

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

    bool selectItem(QUuid itemId);
    bool selectItem(int itemType, QUuid itemId);
    bool selectItem(int itemType, QUuid itemId, QUuid adventureId);
    QStandardItem* findItem(QStandardItem* parent, int itemType, QUuid itemId);
    QStandardItem* findItem(QStandardItem* parent, QUuid itemId);
    QStandardItem* findParentbyType(QStandardItem* child, int parentType);
    void setIndexExpanded(bool expanded, const QModelIndex& index);

    // Bestiary
    void writeBestiary();
    void writeSpellbook();

    CampaignObjectBase* newEncounter(int encounterType, const QString& dialogTitle, const QString& dialogText);

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
    void handleAnimationPreview(QImage img);

    // Bestiary
    void openBestiary();
    void exportBestiary();
    void importBestiary();

    void openSpellbook();
    void exportSpellbook();
    void importSpellbook();

    void openAboutDialog();

    void openTextPublisher();
    void openTextTranslator();
    void openRandomMarkets();

    QDialog* createDialog(QWidget* contents, const QSize& dlgSize = QSize());

    void connectTextToText();
    void connectTextToScroll();

    void battleModelChanged(BattleDialogModel* model);
    void activateObject(CampaignObjectBase* object);
    void deactivateObject();
    void activateWidget(int objectType, CampaignObjectBase* object = nullptr);
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
    EncounterScrollingTextEdit* _scrollingTextEdit;

    CampaignTreeModel* treeModel;
    QMap<QString, QUuid> treeIndexMap;
    QVBoxLayout* characterLayout;
    Campaign* campaign;
    QString campaignFileName;

    OptionsContainer* _options;

    BestiaryDialog bestiaryDlg;
    SpellbookDialog spellDlg;

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

    RibbonMain* _ribbon;
    RibbonTabFile* _ribbonTabFile;
    RibbonTabCampaign* _ribbonTabCampaign;
    RibbonTabBestiary* _ribbonTabTools;
    RibbonTabMap* _ribbonTabMap;
    RibbonTabBattle* _ribbonTabBattle;
    RibbonTabScrolling* _ribbonTabScrolling;
    RibbonTabText* _ribbonTabText;
    MapEditFrame* _ribbonTabMiniMap;
};

#endif // MAINWINDOW_H
