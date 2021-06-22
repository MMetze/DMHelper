#include "battleframe.h"
#include "ui_battleframe.h"
#include "monster.h"
#include "widgetmonster.h"
#include "widgetmonsterinternal.h"
#include "widgetcharacter.h"
#include "widgetcharacterinternal.h"
#include "monsterclass.h"
#include "dmconstants.h"
#include "bestiary.h"
#include "encounterbattle.h"
#include "battledialoglogger.h"
#include "battledialoglogview.h"
#include "unselectedpixmap.h"
#include "map.h"
#include "campaign.h"
#include "character.h"
#include "mapselectdialog.h"
#include "combatantdialog.h"
#include "selectzoom.h"
#include "battledialogmodel.h"
#include "battledialogmodelcharacter.h"
#include "battledialogmodelmonsterbase.h"
#include "battledialogmodelmonsterclass.h"
#include "battledialoggraphicsscene.h"
#include "battlecombatantframe.h"
#include "dicerolldialogcombatants.h"
#include "itemselectdialog.h"
#include "videoplayer.h"
#include "camerarect.h"
#include "battleframemapdrawer.h"
#include "battleframestate.h"
#include "combatantrolloverframe.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QMenu>
#include <QAction>
#include <QDrag>
#include <QMimeData>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QtGlobal>
#include <QPen>
#include <QBrush>
#include <QTimer>
#include <QTime>
#include <QScrollBar>
#include <QInputDialog>
#include <QtMath>
#include <QUuid>
#include <QPixmap>
#include <QScreen>

//#define BATTLE_DIALOG_PROFILE_RENDER
//#define BATTLE_DIALOG_PROFILE_RENDER_TEXT
//#define BATTLE_DIALOG_PROFILE_PRESCALED_BACKGROUND
//#define BATTLE_DIALOG_LOG_MOVEMENT

//#define BATTLE_DIALOG_LOG_VIDEO

const qreal SELECTED_PIXMAP_SIZE = 800.0;
const qreal ACTIVE_PIXMAP_SIZE = 800.0;
const qreal COUNTDOWN_TIMER = 0.05;
const qreal COMPASS_SCALE = 0.4;
const int ROTATION_TIMER = 50;

#ifdef BATTLE_DIALOG_PROFILE_RENDER
    QTime tProfile;
    int tBasicPrep = 0;
    int tVideoPrep = 0;
    int tVideoRender = 0;
    int tPrescaledPrep = 0;
    int tPrescaledRender = 0;
    int tContent = 0;
    int tAdditionalContent = 0;
#endif

BattleFrame::BattleFrame(QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::BattleFrame),
    _battle(nullptr),
    _model(nullptr),
    _combatantLayout(nullptr),
    _logger(nullptr),
    _combatantWidgets(),
    _combatantIcons(),
    _stateMachine(),
    _contextMenuCombatant(nullptr),
    _mouseDown(false),
    _mouseDownPos(),
    _hoverFrame(nullptr),
    _publishMouseDown(false),
    _publishMouseDownPos(),
    _scene(nullptr),
    _background(nullptr),
    _fow(nullptr),
    _activePixmap(nullptr),
    _activeScale(1.0),
    _selectedScale(1.0),
    _compassPixmap(nullptr),
    _movementPixmap(nullptr),
    _publishRect(nullptr),
    _publishRectValue(),
    _cameraCoupled(false),
    _includeHeight(false),
    _pitchHeight(0.0),
    _countdownTimer(nullptr),
    _countdown(0.0),
    _publishing(false),
    _publishTimer(nullptr),
    _prescaledBackground(),
    _fowImage(),
    _combatantFrame(),
    _countdownFrame(),
    _targetSize(),
    _targetLabelSize(),
    _mapDrawer(nullptr),
    _showOnDeck(true),
    _showCountdown(true),
    _countdownDuration(15),
    _countdownColor(0,0,0),
    _pointerFile(),
    _activeFile(),
    _rubberBandRect(),
    _scale(1.0),
    _rotation(0),
    _moveRadius(0.0),
    _moveStart(),
    _videoPlayer(nullptr),
    _bwFoWImage(),
    _sourceRect(),
    _videoSize()
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(0,5);
    ui->splitter->setStretchFactor(1,0);

    _scene = new BattleDialogGraphicsScene(this);
    ui->graphicsView->setScene(_scene);

    _combatantLayout = new QVBoxLayout;
    _combatantLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ui->scrollAreaWidgetContents->setLayout(_combatantLayout);

    ui->scrollArea->setAcceptDrops(true);
    ui->scrollArea->installEventFilter(this);

    ui->graphicsView->installEventFilter(this);

    _publishTimer = new QTimer(this);
    _publishTimer->setSingleShot(false);
    connect(_publishTimer, SIGNAL(timeout()),this,SLOT(executeAnimateImage()));

    _countdownTimer = new QTimer(this);
    _countdownTimer->setSingleShot(false);
    connect(_countdownTimer, SIGNAL(timeout()),this,SLOT(countdownTimerExpired()));

    _mapDrawer = new BattleFrameMapDrawer(this);
    connect(_mapDrawer, &BattleFrameMapDrawer::fowChanged, this, &BattleFrame::updateFowImage);

    connect(ui->graphicsView,SIGNAL(rubberBandChanged(QRect,QPointF,QPointF)),this,SLOT(handleRubberBandChanged(QRect,QPointF,QPointF)));

    connect(ui->btnSort, SIGNAL(clicked()), this, SLOT(sort()));
    connect(ui->btnNext, SIGNAL(clicked()), this, SLOT(next()));

    connect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
    connect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));

    connect(_scene, SIGNAL(effectChanged(QGraphicsItem*)), this, SLOT(handleEffectChanged(QGraphicsItem*)));
    connect(_scene, SIGNAL(effectRemoved(QGraphicsItem*)), this, SLOT(handleEffectRemoved(QGraphicsItem*)));
    connect(_scene, SIGNAL(applyEffect(QGraphicsItem*)), this, SLOT(handleApplyEffect(QGraphicsItem*)));
    connect(_scene, SIGNAL(distanceChanged(const QString&)), this, SIGNAL(distanceChanged(const QString&)));
    connect(_scene, SIGNAL(combatantHover(BattleDialogModelCombatant*, bool)), this, SLOT(handleCombatantHover(BattleDialogModelCombatant*, bool)));
    connect(_scene, SIGNAL(itemChanged(QGraphicsItem*)), this, SLOT(handleItemChanged(QGraphicsItem*)));
    setEditMode();

    // CombatantFrame
    connect(ui->frameCombatant, &BattleCombatantFrame::conditionsChanged, this, &BattleFrame::updateCombatantWidget);
    connect(ui->frameCombatant, &BattleCombatantFrame::conditionsChanged, this, &BattleFrame::updateCombatantIcon);

    // State Machine
    connect(&_stateMachine, SIGNAL(enterState(BattleFrameState*)), this, SLOT(stateUpdated()));
    connect(&_stateMachine, &BattleFrameStateMachine::stateChanged, _scene, &BattleDialogGraphicsScene::setInputMode);
    prepareStateMachine();

    setBattle(nullptr);
    setMapCursor();

    qDebug() << "[Battle Frame] created";
}

BattleFrame::~BattleFrame()
{
    qDebug() << "[Battle Frame] being destroyed: " << _combatantLayout->count() << " layouts and " << _combatantWidgets.count() << " widgets";

    VideoPlayer* deletePlayer = _videoPlayer;
    _videoPlayer = nullptr;
    delete deletePlayer;

    QLayoutItem *child;
    while ((child = _combatantLayout->takeAt(0)) != nullptr)
    {
        delete child;
    }

    QMapIterator<BattleDialogModelCombatant*, CombatantWidget*> i(_combatantWidgets);
    while(i.hasNext())
    {
        i.next();
        i.value()->deleteLater();
    }
    _combatantWidgets.clear();

    delete ui;

    qDebug() << "[Battle Frame] destroyed.";
}

void BattleFrame::activateObject(CampaignObjectBase* object)
{
    EncounterBattle* battle = dynamic_cast<EncounterBattle*>(object);
    if(!battle)
        return;

    if(_battle != nullptr)
    {
        qDebug() << "[BattleFrame] ERROR: New battle object activated without deactivating the existing battle object first!";
        deactivateObject();
    }

    setBattle(battle);

    emit checkableChanged(true);
}

void BattleFrame::deactivateObject()
{
    if(!_battle)
    {
        qDebug() << "[BattleFrame] WARNING: Invalid (nullptr) battle object deactivated!";
        return;
    }

    setBattle(nullptr);
}

void BattleFrame::setBattle(EncounterBattle* battle)
{
    _battle = battle;
    setModel(_battle == nullptr ? nullptr : _battle->getBattleDialogModel());
}

EncounterBattle* BattleFrame::getBattle()
{
    return _battle;
}

void BattleFrame::setBattleMap()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set battle map, no battle model set!";
        return;
    }

    if(_model->isMapChanged())
    {
        disconnect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
        disconnect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
        replaceBattleMap();
        connect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
        connect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
    }
    else
    {
        resizeBattleMap();
    }
}

void BattleFrame::addCombatant(BattleDialogModelCombatant* combatant)
{
    qDebug() << "[Battle Frame] combatant added, type " << combatant->getCombatantType() << ", init " << combatant->getInitiative() << ", pos " << combatant->getPosition();

    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to add combatant, no battle model is set!";
        return;
    }

    _model->appendCombatant(combatant);
    createCombatantWidget(combatant);
    createCombatantIcon(combatant);
}

void BattleFrame::addCombatants(QList<BattleDialogModelCombatant*> combatants)
{
    for(int i = 0; i < combatants.count(); ++i)
    {
        addCombatant(combatants.at(i));
    }
}

QList<BattleDialogModelCombatant*> BattleFrame::getCombatants() const
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to get the combatant list, no battle model is set!";
        return QList<BattleDialogModelCombatant*>();
    }

    return _model->getCombatantList();
}

QList<BattleDialogModelCombatant*> BattleFrame::getLivingCombatants() const
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to get living combatant, no battle model is set!";
        return QList<BattleDialogModelCombatant*>();
    }

    QList<BattleDialogModelCombatant*> result;
    for(int i = 0; i < _model->getCombatantCount(); ++i)
    {
        if((_model->getCombatant(i)) && (_model->getCombatant(i)->getHitPoints() > 0))
        {
            result.append(_model->getCombatant(i));
        }
    }

    qDebug() << "[Battle Frame] " << result.count() << " living combatants found.";

    return result;
}

BattleDialogModelCombatant* BattleFrame::getFirstLivingCombatant() const
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to get the first living combatant, no battle model is set!";
        return nullptr;
    }

    for(int i = 0; i < _model->getCombatantCount(); ++i)
    {
        if((_model->getCombatant(i)) && (_model->getCombatant(i)->getHitPoints() > 0))
        {
            qDebug() << "[Battle Frame] first living combatants found: " << _model->getCombatant(i)->getName();
            return _model->getCombatant(i);
        }
    }

    qDebug() << "[Battle Frame] No first living combatants found.";

    return nullptr;
}

QList<BattleDialogModelCombatant*> BattleFrame::getMonsters() const
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to get the monster list, no battle model is set!";
        return QList<BattleDialogModelCombatant*>();
    }

    QList<BattleDialogModelCombatant*> result;
    for(int i = 0; i < _model->getCombatantCount(); ++i)
    {
        if((_model->getCombatant(i)) && (_model->getCombatant(i)->getCombatantType() == DMHelper::CombatantType_Monster))
        {
            result.append(_model->getCombatant(i));
        }
    }

    qDebug() << "[Battle Frame] " << result.count() << " monster combatants found.";

    return result;
}

QList<BattleDialogModelCombatant*> BattleFrame::getLivingMonsters() const
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to get the list of living monsters, no battle model is set!";
        return QList<BattleDialogModelCombatant*>();
    }

    QList<BattleDialogModelCombatant*> result;
    for(int i = 0; i < _model->getCombatantCount(); ++i)
    {
        if((_model->getCombatant(i)) && (_model->getCombatant(i)->getCombatantType() == DMHelper::CombatantType_Monster) && (_model->getCombatant(i)->getHitPoints() > 0))
        {
            result.append(_model->getCombatant(i));
        }
    }

    qDebug() << "[Battle Frame] " << result.count() << " living monster combatants found.";

    return result;
}

void BattleFrame::recreateCombatantWidgets()
{
    qDebug() << "[Battle Frame] recreating combatant widgets";
    clearCombatantWidgets();
    buildCombatantWidgets();
    qDebug() << "[Battle Frame] combatant widgets recreated";
}

QRect BattleFrame::viewportRect()
{
    return ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect().toAlignedRect();
}

QPoint BattleFrame::viewportCenter()
{
    QPoint combatantPos = viewportRect().topLeft();
    combatantPos += QPoint(viewportRect().width() / 2, viewportRect().height() / 2);
    return combatantPos;
}

BattleFrameMapDrawer* BattleFrame::getMapDrawer() const
{
    return _mapDrawer;
}

void BattleFrame::clear()
{
    setBattle(nullptr);
}

void BattleFrame::sort()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to sort combatants, no battle model is set!";
        return;
    }

    // OPTIMIZE: can this be optimized?
    qDebug() << "[Battle Frame] sorting combatant widgets";
    clearCombatantWidgets();
    _model->sortCombatants();
    buildCombatantWidgets();
    setActiveCombatant(_model->getActiveCombatant());
    ui->scrollArea->setFocus();
    qDebug() << "[Battle Frame] combatant widgets sorted";
}

void BattleFrame::next()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to select the next combatant, no battle model is set!";
        return;
    }

    BattleDialogModelCombatant* activeCombatant = _model->getActiveCombatant();
    if(!activeCombatant)
        return;

    qDebug() << "[Battle Frame] Looking for next combatant (current combatant " << activeCombatant << ")...";
    BattleDialogModelCombatant* nextCombatant = getNextCombatant(activeCombatant);

    if(!nextCombatant)
    {
        qDebug() << "[Battle Frame] ... no next combatant found.";
        return;
    }

    int activeInitiative = activeCombatant->getInitiative();
    int nextInitiative = nextCombatant->getInitiative();

    if(_model->getShowLairActions())
    {
        if((activeInitiative >= 20) && (nextInitiative < 20))
        {
            qDebug() << "[Battle Frame] Triggering Lair Action request.";
            QMessageBox::information(this, QString("Lair Action"), QString("The legendary creature(s) can now use one of their lair action options. It cannot do so while incapacitated, surprised or otherwise unable to take actions."));
        }
    }

    if((_logger) && (activeInitiative < nextInitiative))
        _logger->newRound();

    setActiveCombatant(nextCombatant);
    qDebug() << "[Battle Frame] ... next combatant found: " << nextCombatant;
}

void BattleFrame::setTargetSize(const QSize& targetSize)
{
    qDebug() << "[Battle Frame] Target size being set to: " << targetSize;

    if(targetSize == _targetSize)
        return;

    _targetSize = targetSize;

    if(_videoPlayer)
    {
        resetVideoSizes();
        _videoPlayer->targetResized(_videoSize);
    }
    else
    {
        createPrescaledBackground();
    }
}

void BattleFrame::setTargetLabelSize(const QSize& targetSize)
{
    qDebug() << "[Battle Frame] Target label size being set to: " << targetSize;

    if(targetSize == _targetLabelSize)
        return;

    _targetLabelSize = targetSize;
}

void BattleFrame::publishWindowMouseDown(const QPointF& position)
{
    if(!_model)
        return;

    QPointF newPosition;
    if(!convertPublishToScene(position, newPosition))
        return;

    if(_scene)
    {
        QList<QGraphicsItem *> itemList = _scene->items(newPosition);
        for(QGraphicsItem* graphicsItem : itemList)
        {
            QGraphicsPixmapItem* pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(graphicsItem);
            if((pixmapItem) && ((pixmapItem->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable))
            {
                BattleDialogModelCombatant* selectedCombatant = _combatantIcons.key(pixmapItem, nullptr);
                if(selectedCombatant)
                {
                    setUniqueSelection(selectedCombatant);
                    _model->setSelectedCombatant(selectedCombatant);
                    _publishMouseDown = true;
                    _publishMouseDownPos = newPosition;
                    startMovement(pixmapItem, selectedCombatant->getSpeed());
                }
            }
        }
    }
}

void BattleFrame::publishWindowMouseMove(const QPointF& position)
{
    if((!_model) || (!_model->getSelectedCombatant()) || (!_publishMouseDown))
        return;

    QPointF newPosition;
    if(!convertPublishToScene(position, newPosition))
        return;

    if(newPosition == _publishMouseDownPos)
        return;

    QGraphicsPixmapItem* pixmapItem = _combatantIcons.value(_model->getSelectedCombatant());
    pixmapItem->setPos(newPosition);
    updateMovement(pixmapItem);
}

void BattleFrame::publishWindowMouseRelease(const QPointF& position)
{
    Q_UNUSED(position);

    if(!_model)
        return;

    endMovement();
    _model->setSelectedCombatant(nullptr);
    _publishMouseDown = false;
}

void BattleFrame::setGridScale(int gridScale)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set the grid scale, no battle model is set!";
        return;
    }

    if(_scene)
    {
        _model->setGridScale(gridScale);

        qreal scaleFactor;

        QMapIterator<BattleDialogModelCombatant*, QGraphicsPixmapItem*> i(_combatantIcons);
        while(i.hasNext())
        {
            i.next();
            if(i.value())
            {
                qreal combatantScaleFactor = 1.0;
                if(_combatantIcons.key(i.value(), nullptr))
                    combatantScaleFactor = _combatantIcons.key(i.value(), nullptr)->getSizeFactor();
                scaleFactor = (static_cast<qreal>(gridScale-2)) * combatantScaleFactor / static_cast<qreal>(qMax(i.value()->pixmap().width(),i.value()->pixmap().height()));
                i.value()->setScale(scaleFactor);
            }
        }

        if(_activePixmap)
        {
            scaleFactor = static_cast<qreal>(gridScale) * _activeScale / ACTIVE_PIXMAP_SIZE;
            qreal oldScaleFactor = _activePixmap->scale();
            _activePixmap->setScale(scaleFactor);
            _activePixmap->setPos(_activePixmap->pos() * scaleFactor/oldScaleFactor);
        }

        _scene->updateBattleContents();

        ui->graphicsView->update();
        createPrescaledBackground();
    }
}

void BattleFrame::setXOffset(int xOffset)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set the X offset, no battle model is set!";
        return;
    }

    if(_scene)
    {
        _model->setGridOffsetX(xOffset);
        _scene->updateBattleContents();
        ui->graphicsView->update();
        createPrescaledBackground();
    }
}

void BattleFrame::setYOffset(int yOffset)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set the Y offset, no battle model is set!";
        return;
    }

    if(_scene)
    {
        _model->setGridOffsetY(yOffset);
        _scene->updateBattleContents();
        ui->graphicsView->update();
        createPrescaledBackground();
    }
}

void BattleFrame::setGridVisible(bool gridVisible)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to turn the grid on or off, no battle model is set!";
        return;
    }

    if((_scene) && (gridVisible != _model->getGridOn()))
    {
        _model->setGridOn(gridVisible);
        _scene->setGridVisibility(gridVisible);
        ui->graphicsView->invalidateScene();
        createPrescaledBackground();
    }
}

void BattleFrame::setShowOnDeck(bool showOnDeck)
{
    _showOnDeck = showOnDeck;
    createPrescaledBackground();
}

void BattleFrame::setShowCountdown(bool showCountdown)
{
    _showCountdown = showCountdown;
    createPrescaledBackground();
}

void BattleFrame::setCountdownDuration(int countdownDuration)
{
    _countdownDuration = countdownDuration;
}

void BattleFrame::setPointerFile(const QString& filename)
{
    if(!_scene)
        return;

    if(_pointerFile != filename)
    {
        _pointerFile = filename;
        QPixmap pointerPixmap = getPointerPixmap();
        _scene->setPointerPixmap(pointerPixmap);
        emit pointerChanged(QCursor(pointerPixmap.scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), 0, 0));
    }
}

void BattleFrame::setSelectedIcon(const QString& selectedIcon)
{
    if(!_scene)
        return;

    _scene->setSelectedIcon(selectedIcon);
}

void BattleFrame::setActiveIcon(const QString& activeIcon)
{
    if(_activeFile != activeIcon)
    {
        _activeFile = activeIcon;
        createActiveIcon();
    }
}

void BattleFrame::createActiveIcon()
{
    if((!_scene) || (!_model))
        return;

    if(_activePixmap)
    {
        QGraphicsPixmapItem* deletePixmap = _activePixmap;
        _activePixmap = nullptr;
        delete deletePixmap;
    }

    QPixmap activePmp;
    if((_activeFile.isEmpty()) || (!activePmp.load(_activeFile)))
        activePmp.load(QString(":/img/data/active.png"));

    _activePixmap = _scene->addPixmap(activePmp);
    _activePixmap->setTransformationMode(Qt::SmoothTransformation);
    _activePixmap->setScale(static_cast<qreal>(_model->getGridScale()) * _activeScale / ACTIVE_PIXMAP_SIZE);
    _activePixmap->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    _activePixmap->hide();
}

void BattleFrame::setCombatantFrame(const QString& combatantFrame)
{
    if(_combatantFile != combatantFrame)
    {
        _combatantFile = combatantFrame;
        createCombatantFrame();
    }
}

void BattleFrame::createCombatantFrame()
{
    if((_combatantFile.isEmpty()) || (!_combatantFrame.load(_combatantFile)))
        _combatantFrame.load(QString(":/img/data/combatant_frame.png"));
}

void BattleFrame::setCountdownFrame(const QString& countdownFrame)
{
    if(_countdownFile != countdownFrame)
    {
        _countdownFile = countdownFrame;
        createCountdownFrame();
    }
}

void BattleFrame::createCountdownFrame()
{
    if((_countdownFile.isEmpty()) || (!_countdownFrame.load(_countdownFile)))
        _countdownFrame.load(QString(":/img/data/countdown_frame.png"));
}

void BattleFrame::zoomIn()
{
    if(!_background)
        return;

    setScale(1.1);
}

void BattleFrame::zoomOut()
{
    if(!_background)
        return;

    setScale(0.9);
}

void BattleFrame::zoomFit()
{
    if(!_background)
        return;

    ui->graphicsView->fitInView(_background, Qt::KeepAspectRatio);
    setScale(1.0);
}

void BattleFrame::zoomSelect(bool enabled)
{
    Q_UNUSED(enabled);

    if(!_background)
        return;

    _stateMachine.toggleState(DMHelper::BattleFrameState_ZoomSelect);
}

void BattleFrame::cancelSelect()
{
    qDebug() << "[BattleFrame] cancelSelect";
    _stateMachine.deactivateState();
}

void BattleFrame::selectBattleMap()
{
    if((!_battle) || (!_battle->getBattleDialogModel()))
        return;

    qDebug() << "[Battle Frame] Selecting a new map...";

    Map* battleMap = selectRelatedMap();
    if(!battleMap)
        return;

    battleMap->initialize();
    QImage fowImage = battleMap->getGrayImage();

    SelectZoom zoomDlg(fowImage, this);
    zoomDlg.exec();
    if(zoomDlg.result() == QDialog::Accepted)
    {
        qDebug() << "[Battle Frame] Setting new map to: " << battleMap->getID() << " " << battleMap->getName();
        _battle->getBattleDialogModel()->setMap(battleMap, zoomDlg.getZoomRect());
        setBattleMap();
    }
}

void BattleFrame::reloadMap()
{
    if(_model)
        _model->setBackgroundImage(QImage());

    updateMap();
}

void BattleFrame::addMonsters()
{
    if(!_battle)
        return;

    qDebug() << "[Battle Frame] Adding monsters ...";

    QPointF combatantPos = viewportCenter();

    CombatantDialog combatantDlg(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(&combatantDlg, SIGNAL(openMonster(QString)), this, SIGNAL(monsterSelected(QString)));

    int result = combatantDlg.exec();
    if(result == QDialog::Accepted)
    {
        MonsterClass* monsterClass = combatantDlg.getMonsterClass();
        if(monsterClass == nullptr)
        {
            qDebug() << "[Battle Frame] ... invalid/unknown monster class found - not able to add monster combatant";
            return;
        }

        QString baseName = combatantDlg.getName();
        int monsterCount = combatantDlg.getCount();
        int localHP = combatantDlg.getLocalHitPoints().isEmpty() ? 0 : combatantDlg.getLocalHitPoints().toInt();

        qDebug() << "[Battle Dialog Manager] ... adding " << monsterCount << " monsters of name " << baseName;

        for(int i = 0; i < monsterCount; ++i)
        {
            BattleDialogModelMonsterClass* monster = new BattleDialogModelMonsterClass(monsterClass);
            if(monsterCount == 1)
            {
                monster->setMonsterName(baseName);
            }
            else
            {
                monster->setMonsterName(baseName + QString("#") + QString::number(i+1));
            }
            monster->setHitPoints(localHP == 0 ? monsterClass->getHitDice().roll() : localHP);
            monster->setInitiative(Dice::d20() + Combatant::getAbilityMod(monsterClass->getDexterity()));
            monster->setPosition(combatantPos);
            addCombatant(monster);
        }

        recreateCombatantWidgets();
    }
    else
    {
        qDebug() << "[Battle Dialog Manager] ... add monsters dialog cancelled";
    }
}

void BattleFrame::addCharacter()
{
    if((!_battle) || (!_model))
        return;

    Campaign* campaign = dynamic_cast<Campaign*>(_battle->getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return;

    qDebug() << "[Battle Frame] Adding a character to the battle...";

    QList<Character*> characterList;
    QList<Character*> allCharacters = campaign->findChildren<Character*>();
    for(Character* character : allCharacters)
    {
        if((!_model->isCombatantInList(character)) &&
           (character->isInParty()))
            characterList.append(character);
    }

    if(characterList.isEmpty())
    {
        QMessageBox::information(this, QString("Add Character"), QString("No further characters could be found to add to the current battle."));
        qDebug() << "[Battle Dialog Manager] ...no characters found to add";
        return;
    }

    selectAddCharacter(characterList, QString("Select a Character"), QString("Select Character:"));
}

void BattleFrame::addNPC()
{
    if((!_battle) || (!_model))
        return;

    Campaign* campaign = dynamic_cast<Campaign*>(_battle->getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return;

    qDebug() << "[Battle Frame] Adding an NPC to the battle...";

    QList<Character*> characterList;
    QList<Character*> allCharacters = campaign->findChildren<Character*>();
    for(Character* character : allCharacters)
    {
        if((!_model->isCombatantInList(character)) &&
           (!character->isInParty()))
            characterList.append(character);
    }

    if(characterList.isEmpty())
    {
        QMessageBox::information(this, QString("Add NPC"), QString("No further NPCs could be found to add to the current battle."));
        qDebug() << "[Battle Dialog Manager] ...no NPCs found to add";
        return;
    }

    selectAddCharacter(characterList, QString("Select an NPC"), QString("Select NPC:"));
}

void BattleFrame::addObject()
{
    if(_scene)
        _scene->addEffectObject();
}

void BattleFrame::castSpell()
{
    if(_scene)
        _scene->castSpell();
}

void BattleFrame::addEffectRadius()
{
    if(_scene)
        _scene->addEffectRadius();
}

void BattleFrame::addEffectCone()
{
    if(_scene)
        _scene->addEffectCone();
}

void BattleFrame::addEffectCube()
{
    if(_scene)
        _scene->addEffectCube();
}

void BattleFrame::addEffectLine()
{
    if(_scene)
        _scene->addEffectLine();
}

void BattleFrame::setCameraCouple(bool couple)
{
    if((!_publishRect) || (_cameraCoupled == couple))
        return;

    _cameraCoupled = couple;

    if(couple)
        setCameraToView();

    emit cameraCoupleChanged(couple);
}

void BattleFrame::cancelCameraCouple()
{
    setCameraCouple(false);
}

void BattleFrame::setCameraMap()
{
    if((!_publishRect) || (!_scene))
        return;

    QRectF sceneRect = _scene->sceneRect();
    _publishRect->setCameraRect(sceneRect);
}

void BattleFrame::setCameraSelect(bool enabled)
{
    Q_UNUSED(enabled);
    _stateMachine.toggleState(DMHelper::BattleFrameState_CameraSelect);
}

void BattleFrame::setCameraEdit(bool enabled)
{
    Q_UNUSED(enabled);
    _stateMachine.toggleState(DMHelper::BattleFrameState_CameraEdit);
}

void BattleFrame::setDistance(bool enabled)
{
    Q_UNUSED(enabled);
    _stateMachine.toggleState(DMHelper::BattleFrameState_Distance);
}

void BattleFrame::setDistanceHeight(bool heightEnabled, qreal height)
{
    if(_scene)
        _scene->setDistanceHeight(heightEnabled ? height : 0.0);
}

void BattleFrame::setShowHeight(bool showHeight)
{
    _includeHeight = showHeight;
}

void BattleFrame::setHeight(qreal height)
{
    _pitchHeight = height;
}

void BattleFrame::setFoWEdit(bool enabled)
{
    Q_UNUSED(enabled);
    _stateMachine.toggleState(DMHelper::BattleFrameState_FoWEdit);
}

void BattleFrame::setFoWSelect(bool enabled)
{
    Q_UNUSED(enabled);
    _stateMachine.toggleState(DMHelper::BattleFrameState_FoWSelect);
}

void BattleFrame::setPointerOn(bool enabled)
{
    Q_UNUSED(enabled);
    _stateMachine.toggleState(DMHelper::BattleFrameState_Pointer);
}

void BattleFrame::keyPressEvent(QKeyEvent * e)
{
    switch(e->key())
    {
        case Qt::Key_Space:
            next();
            return;
        case Qt::Key_Escape:
            cancelSelect();
            return;
        default:
            break;
    }

    QFrame::keyPressEvent(e);
}

bool BattleFrame::eventFilter(QObject *obj, QEvent *event)
{
    if(_model)
    {
        CombatantWidget* widget = dynamic_cast<CombatantWidget*>(obj);

        if(widget)
        {
            if(event->type() == QEvent::MouseButtonPress)
            {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                _mouseDownPos = mouseEvent->pos();
                _mouseDown = true;
                qDebug() << "[Battle Frame] combatant widget mouse down " << _mouseDownPos.x() << "," << _mouseDownPos.y();
            }
            else if(event->type() == QEvent::MouseMove)
            {
                if(_mouseDown)
                {
                    // Mouse moved with button down on a combatant widget --> drag the widget order
                    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                    if((mouseEvent->pos() - _mouseDownPos).manhattanLength() > QApplication::startDragDistance())
                    {
                        BattleDialogModelCombatant* combatant = _combatantWidgets.key(widget, nullptr);
                        if(combatant)
                        {
                            int index = _model->getCombatantList().indexOf(combatant);
                            if(index >= 0)
                            {
                                qDebug() << "[Battle Frame] starting combatant widget drag: index " << index << ": " << combatant->getName() << ", (" << reinterpret_cast<quint64>(widget) << ") " << mouseEvent->pos().x() << "," << mouseEvent->pos().y();
                                QDrag *drag = new QDrag(this);
                                QMimeData *mimeData = new QMimeData;

                                QPixmap px(widget->size());
                                widget->render(&px);

                                QByteArray encodedData;
                                QDataStream stream(&encodedData, QIODevice::WriteOnly);
                                stream << index;
                                mimeData->setData(QString("application/vnd.dmhelper.combatant"), encodedData);
                                drag->setMimeData(mimeData);
                                drag->setPixmap(px);
                                drag->exec(Qt::CopyAction | Qt::MoveAction);
                            }
                        }
                    }
                }
            }
            else if(event->type() == QEvent::MouseButtonRelease)
            {
                qDebug() << "[Battle Frame] combatant widget mouse released: " << _combatantWidgets.key(widget, nullptr);
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                BattleDialogModelCombatant* selected = _combatantWidgets.key(widget, nullptr);
                if(mouseEvent->modifiers().testFlag(Qt::ShiftModifier) ||
                   mouseEvent->modifiers().testFlag(Qt::ControlModifier) ||
                   mouseEvent->modifiers().testFlag(Qt::AltModifier))
                    setSelectedCombatant(selected);
                else
                    setUniqueSelection(selected);
                _mouseDown = false;
            }
            else if(event->type() == QEvent::HoverEnter)
            {
                if((!_mouseDown) && (_combatantLayout) && (widget->getCombatant()) && (widget->getCombatant()->getCombatantType() == DMHelper::CombatantType_Monster))
                {
                    if(_hoverFrame)
                        removeRollover();

                    // Mouse moved without button down on a combatant widget --> roll-over popup for this widget
                    _hoverFrame = new CombatantRolloverFrame(widget, this);
                    connect(_hoverFrame, SIGNAL(hoverEnded()), this, SLOT(removeRollover()));
                    QPoint framePos(ui->splitter->widget(1)->x() + _combatantLayout->contentsMargins().left() + 6 - _hoverFrame->width(),
                                    ui->scrollArea->y() + widget->y() - ui->scrollArea->verticalScrollBar()->value());
                    _hoverFrame->move(framePos);
                    _hoverFrame->show();
                }
            }
            else if(event->type() == QEvent::HoverLeave)
            {
                if(_hoverFrame)
                    _hoverFrame->triggerClose();
            }
        }
        else
        {
            if(event->type() == QEvent::DragEnter)
            {
                QDragEnterEvent* dragEnterEvent = dynamic_cast<QDragEnterEvent*>(event);
                if(dragEnterEvent)
                {
                    const QMimeData* mimeData = dragEnterEvent->mimeData();
                    if((mimeData)&&(mimeData->hasFormat(QString("application/vnd.dmhelper.combatant"))))
                    {
                        qDebug() << "[Battle Frame] combatant widget drag enter accepted";
                        dragEnterEvent->accept();
                        return true;
                    }
                    else
                    {
                        qDebug() << "[Battle Frame] unknown drag enter ignored";
                        dragEnterEvent->ignore();
                    }
                }
            }
            else if(event->type() == QEvent::DragMove)
            {
                QDragMoveEvent* dragMoveEvent = dynamic_cast<QDragMoveEvent*>(event);
                if(dragMoveEvent)
                {
                    const QMimeData* mimeData = dragMoveEvent->mimeData();
                    if((mimeData)&&(mimeData->hasFormat(QString("application/vnd.dmhelper.combatant"))))
                    {
                        QByteArray encodedData = mimeData->data(QString("application/vnd.dmhelper.combatant"));
                        QDataStream stream(&encodedData, QIODevice::ReadOnly);
                        int index;
                        stream >> index;

                        QWidget* draggedWidget = _combatantWidgets.value(_model->getCombatant(index));
                        int currentIndex = _combatantLayout->indexOf(draggedWidget);

                        QWidget* targetWidget = findCombatantWidgetFromPosition(dragMoveEvent->pos());

                        if((draggedWidget)&&(targetWidget)&&(draggedWidget != targetWidget))
                        {
                            int targetIndex = _combatantLayout->indexOf(targetWidget);
                            QLayoutItem* item = _combatantLayout->takeAt(currentIndex);
                            _combatantLayout->insertItem(targetIndex, item);
                        }
                    }
                }
            }
            else if(event->type() == QEvent::DragLeave)
            {
                qDebug() << "[Battle Frame] combatant drag left";
                reorderCombatantWidgets();
            }
            else if(event->type() == QEvent::Drop)
            {
                QDropEvent* dropEvent = dynamic_cast<QDropEvent*>(event);
                if(dropEvent)
                {
                    qDebug() << "[Battle Frame] combatant widget drag dropped (" << dropEvent->pos().x() << "," << dropEvent->pos().y() << ")";

                    const QMimeData* mimeData = dropEvent->mimeData();
                    if((mimeData)&&(mimeData->hasFormat(QString("application/vnd.dmhelper.combatant"))))
                    {
                        QByteArray encodedData = mimeData->data(QString("application/vnd.dmhelper.combatant"));
                        QDataStream stream(&encodedData, QIODevice::ReadOnly);
                        int index;
                        stream >> index;

                        QWidget* draggedWidget = _combatantWidgets.value(_model->getCombatant(index));
                        int currentIndex = _combatantLayout->indexOf(draggedWidget);

                        if(currentIndex != index)
                        {
                            BattleDialogModelCombatant* combatant = _model->removeCombatant(index);
                            _model->insertCombatant(currentIndex, combatant);
                            qDebug() << "[Battle Frame] combatant widget drag dropped: index " << index << ": " << combatant->getName() << " (" << reinterpret_cast<quint64>(draggedWidget) << "), from pos " << index << " to pos " << currentIndex;
                        }
                    }
                }
                reorderCombatantWidgets();
            }
            else if(event->type() == QEvent::MouseButtonPress)
            {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                _mouseDownPos = mouseEvent->pos();
                _mouseDown = true;
            }
            else if(event->type() == QEvent::MouseButtonRelease)
            {
                setSelectedCombatant(nullptr);
                _mouseDown = false;
            }
        }
    }

    return QObject::eventFilter(obj,event);
}

void BattleFrame::resizeEvent(QResizeEvent *event)
{
    qDebug() << "[Battle Frame] resized: " << event->size().width() << "x" << event->size().height();
    if((_model) && (_background))
    {
        ui->graphicsView->fitInView(_model->getMapRect(), Qt::KeepAspectRatio);
    }
    QFrame::resizeEvent(event);
}

void BattleFrame::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    qDebug() << "[Battle Frame]shown (" << isVisible() << ")";
    if((_model) && (_background))
    {
        ui->graphicsView->fitInView(_model->getMapRect(), Qt::KeepAspectRatio);
    }

    QScreen* primary = QGuiApplication::primaryScreen();
    if(!primary)
        return;

    int ribbonHeight = primary->availableSize().height() / 15;
    QFontMetrics metrics = ui->lblNext->fontMetrics();

    int labelHeight = metrics.height() + (ribbonHeight / 10);
    int iconDim = ribbonHeight - labelHeight;
    int newWidth = qMax(metrics.horizontalAdvance(ui->lblCountdown->text()), iconDim);

    ui->lblNext->setMinimumWidth(newWidth);
    ui->lblNext->setMaximumWidth(newWidth);
    ui->lblNext->setMinimumHeight(labelHeight);
    ui->lblNext->setMaximumHeight(labelHeight);

    ui->lblSort->setMinimumWidth(newWidth);
    ui->lblSort->setMaximumWidth(newWidth);
    ui->lblSort->setMinimumHeight(labelHeight);
    ui->lblSort->setMaximumHeight(labelHeight);

    ui->lblRound->setMinimumWidth(newWidth);
    ui->lblRound->setMaximumWidth(newWidth);
    ui->lblRound->setMinimumHeight(labelHeight);
    ui->lblRound->setMaximumHeight(labelHeight);

    ui->edtRounds->setMinimumWidth(newWidth);
    ui->edtRounds->setMaximumWidth(newWidth);
    ui->edtRounds->setMinimumHeight(iconDim);
    ui->edtRounds->setMaximumHeight(iconDim);

    ui->lblCountdown->setMinimumWidth(newWidth);
    ui->lblCountdown->setMaximumWidth(newWidth);
    ui->lblCountdown->setMinimumHeight(labelHeight);
    ui->lblCountdown->setMaximumHeight(labelHeight);

    ui->edtCountdown->setMinimumWidth(newWidth);
    ui->edtCountdown->setMaximumWidth(newWidth);
    ui->edtCountdown->setMinimumHeight(iconDim);
    ui->edtCountdown->setMaximumHeight(iconDim);

    ui->btnNext->setMinimumWidth(newWidth);
    ui->btnNext->setMaximumWidth(newWidth);
    ui->btnNext->setMinimumHeight(iconDim);
    ui->btnNext->setMaximumHeight(iconDim);

    ui->btnSort->setMinimumWidth(newWidth);
    ui->btnSort->setMaximumWidth(newWidth);
    ui->btnSort->setMinimumHeight(iconDim);
    ui->btnSort->setMaximumHeight(iconDim);

    int iconSize = qMin(newWidth, iconDim) * 4 / 5;
    ui->btnNext->setIconSize(QSize(iconSize, iconSize));
    ui->btnSort->setIconSize(QSize(iconSize, iconSize));
}

void BattleFrame::setCompassVisibility(bool visible)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set compass visibility, no battle model is set!";
        return;
    }

    _model->setShowCompass(visible);
    qDebug() << "[Battle Frame] show compass checked changed: Visibility=" << visible;
    if(_compassPixmap)
        _compassPixmap->setVisible(visible);
}

void BattleFrame::updateCombatantVisibility()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to update combatant visibility, no battle model is set!";
        return;
    }

    qDebug() << "[Battle Frame] show alive/dead checked updated: Alive=" << _model->getShowAlive() << ", Dead=" << _model->getShowDead();
    setCombatantVisibility(_model->getShowAlive(), _model->getShowDead(), true);
}

void BattleFrame::updateEffectLayerVisibility()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to update effect layer visibility, no battle model is set!";
        return;
    }

    qDebug() << "[Battle Frame] show effects checked changed: " << _model->getShowEffects();
    setEffectLayerVisibility(_model->getShowEffects());
}

void BattleFrame::updateMap()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to update the map, no battle model is set!";
        return;
    }

    if((!_background) || (!_fow) || (!_model->getMap()))
    {
        qDebug() << "[Battle Frame] No map found to be updated for the current battle model";
        return;
    }

    qDebug() << "[Battle Frame] Updating map " << _model->getMap()->getFileName() << " rect=" << _model->getMapRect().left() << "," << _model->getMapRect().top() << ", " << _model->getMapRect().width() << "x" << _model->getMapRect().height();
    _model->getMap()->initialize();
    if(_model->getMap()->isInitialized())
    {
        qDebug() << "[Battle Frame] Initializing battle map image";
        if(_model->getBackgroundImage().isNull())
            _model->setBackgroundImage(_model->getMap()->getBackgroundImage());
        _background->setPixmap((QPixmap::fromImage(_model->getBackgroundImage())));
        _fowImage = QPixmap::fromImage(_model->getMap()->getFoWImage());
        _mapDrawer->setMap(_model->getMap(), &_fowImage);

        createPrescaledBackground();
    }
    else
    {
        qDebug() << "[Battle Frame] Initializing battle map video";
        createVideoPlayer(true);
    }
}

void BattleFrame::updateRounds()
{
    if(_logger)
        ui->edtRounds->setText(QString::number(_logger->getRounds()));
}

void BattleFrame::updateVideoBackground()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to update the video background, no battle model is set!";
        return;
    }

    qDebug() << "[Battle Frame] Initializing battle map video background image";
    if((!_videoPlayer) || (!_videoPlayer->getImage()))
        return;

    QImage battleImage = _videoPlayer->getImage()->copy();
    _model->setBackgroundImage(battleImage);
    _background->setPixmap((QPixmap::fromImage(battleImage)));

    QImage fowImage = QImage(_videoPlayer->getImage()->size(), QImage::Format_ARGB32);
    fowImage.fill(QColor(0,0,0,128));
    _model->getMap()->setExternalFoWImage(fowImage);
    _fowImage = QPixmap::fromImage(_model->getMap()->getFoWImage());
    _mapDrawer->setMap(_model->getMap(), &_fowImage);

    if(!doSceneContentsExist())
        createSceneContents();
    qDebug() << "[Battle Frame] Battle map video background image initialized.";
}

void BattleFrame::handleContextMenu(BattleDialogModelCombatant* combatant, const QPoint& position)
{
    if(!combatant)
        return;

    qDebug() << "[Battle Frame] context menu opened for " << combatant->getName() << " at " << position.x() << "x" << position.y();

    _contextMenuCombatant = combatant;

    QMenu* contextMenu = new QMenu(ui->scrollArea);

    QAction* activateItem = new QAction(QString("Activate"), contextMenu);
    connect(activateItem,SIGNAL(triggered()),this,SLOT(activateCombatant()));
    contextMenu->addAction(activateItem);

    QAction* removeItem = new QAction(QString("Remove"), contextMenu);
    connect(removeItem,SIGNAL(triggered()),this,SLOT(removeCombatant()));
    contextMenu->addAction(removeItem);

    QAction* damageItem = new QAction(QString("Damage"), contextMenu);
    connect(damageItem,SIGNAL(triggered()),this,SLOT(damageCombatant()));
    contextMenu->addAction(damageItem);

    contextMenu->exec(position);
    delete contextMenu;

    _contextMenuCombatant = nullptr;
}

void BattleFrame::showStatistics()
{
    if((_model) && (_logger))
    {
        BattleDialogLogView logView(*_model, *_logger);
        logView.exec();
    }
}

void BattleFrame::publishClicked(bool checked)
{
    if(_publishing == checked)
        return;

    qDebug() << "[Battle Frame] publishing toggled (" << checked << ")";
    _publishing = checked;

    if(_publishRect)
        _publishRect->setPublishing(checked);

    if(_publishing)
    {
        createPrescaledBackground();
        publishImage();
    }
    else
    {
        _publishTimer->stop();
    }
}

void BattleFrame::setRotation(int rotation)
{
    if(_rotation == rotation)
        return;

    _rotation = rotation;
    createPrescaledBackground();
}

void BattleFrame::setBackgroundColor(QColor color)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set the background color, no battle model is set!";
        return;
    }

    _model->setBackgroundColor(color);
}

void BattleFrame::reloadObject()
{
    recreateCombatantWidgets();
    reloadMap();
}

void BattleFrame::handleEffectChanged(QGraphicsItem* effectItem)
{
#ifdef BATTLE_DIALOG_LOG_MOVEMENT
    qDebug() << "[Battle Frame] Handle effect changed for " << effectItem;
#endif

    BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(effectItem);
    if((!effect) || (!effect->getEffectActive()))
    {
#ifdef BATTLE_DIALOG_LOG_MOVEMENT
        qDebug() << "[Battle Frame] Not iterating on handling of effect changed for " << effectItem;
#endif
        return;
    }

    for(QGraphicsPixmapItem* item : _combatantIcons.values())
    {
        if(item)
        {
            // OPTIMIZE: Optimize to only remove effects if not still relevant
            removeEffectsFromItem(item);

            if(isItemInEffect(item, effectItem))
            {
                applyEffectToItem(item, effect);
            }
        }
    }
}

void BattleFrame::handleEffectRemoved(QGraphicsItem* effectItem)
{
    Q_UNUSED(effectItem);

    for(QGraphicsPixmapItem* item : _combatantIcons.values())
    {
        if(item)
        {
            // OPTIMIZE: Optimize to only remove effects if not still relevant
            removeEffectsFromItem(item);
        }
    }
}

void BattleFrame::handleCombatantMoved(BattleDialogModelCombatant* combatant)
{
    if((!_scene) || (!combatant))
        return;

#ifdef BATTLE_DIALOG_LOG_MOVEMENT
    qDebug() << "[Battle Frame] Handle Combatant Moved for " << combatant;
#endif

    QGraphicsPixmapItem* item = _combatantIcons.value(combatant, nullptr);
    if(!item)
        return;

    // Todo: optimize this to only make changes when needed
    removeEffectsFromItem(item);

    QList<QGraphicsItem*> effectItems = _scene->getEffectItems();

    for(QGraphicsItem* effectItem : effectItems)
    {
        if(effectItem)
        {
            BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(effectItem);
            if((effect) && (effect->getEffectActive()) && (effect->getEffectVisible()))
            {
                QAbstractGraphicsShapeItem* abstractEffect = dynamic_cast<QAbstractGraphicsShapeItem*>(effectItem);
                if(isItemInEffect(item, abstractEffect))
                {
                    applyEffectToItem(item, effect);
                    return;
                }
            }
        }
    }
}

void BattleFrame::handleCombatantSelected(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    QGraphicsPixmapItem* item = _combatantIcons.value(combatant, nullptr);
    if(!item)
        return;

    CombatantWidget* widget = _combatantWidgets.value(combatant, nullptr);
    if(!widget)
        return;

    item->setSelected(combatant->getSelected());
    widget->setSelected(combatant->getSelected());
}

void BattleFrame::handleCombatantHover(BattleDialogModelCombatant* combatant, bool hover)
{
    if(!combatant)
        return;

    CombatantWidget* widget = _combatantWidgets.value(combatant, nullptr);
    if(!widget)
        return;

    widget->setHover(hover);
}

void BattleFrame::handleApplyEffect(QGraphicsItem* effect)
{
    if(!effect)
        return;

    QList<BattleDialogModelCombatant*> combatantList;

    QList<QGraphicsPixmapItem*> iconPixmaps = _combatantIcons.values();
    for(int i = 0; i < iconPixmaps.count(); ++i)
    {
        QGraphicsPixmapItem* item = iconPixmaps.at(i);
        if((item) && (isItemInEffect(item, effect)))
        {
            BattleDialogModelCombatant* combatant = _combatantIcons.key(item, nullptr);
            if(combatant)
                combatantList.append(combatant);
        }
    }

    if(combatantList.isEmpty())
    {
        QMessageBox::information(this, QString("Apply Effect"), QString("No target combatants were found for the selected effect."));
        return;
    }

    DiceRollDialogCombatants* dlg = new DiceRollDialogCombatants(Dice(1,20,0), combatantList, 15, this);
    connect(dlg, SIGNAL(selectCombatant(BattleDialogModelCombatant*)), this, SLOT(setSelectedCombatant(BattleDialogModelCombatant*)));
    connect(dlg, SIGNAL(combatantChanged(BattleDialogModelCombatant*)),this, SLOT(updateCombatantWidget(BattleDialogModelCombatant*)));
    connect(dlg, SIGNAL(hitPointsChanged(BattleDialogModelCombatant*,int)), this, SLOT(updateCombatantVisibility()));
    connect(dlg, SIGNAL(hitPointsChanged(BattleDialogModelCombatant*,int)), this, SLOT(registerCombatantDamage(BattleDialogModelCombatant*, int)));
    dlg->resize(800, 600);

    dlg->fireAndForget();
}

void BattleFrame::handleItemMouseDown(QGraphicsPixmapItem* item)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to handle item mouse movement, no battle model is set!";
        return;
    }

    BattleDialogModelCombatant* combatant = _combatantIcons.key(item, nullptr);
    if(combatant)
    {
        startMovement(_combatantIcons.value(combatant), combatant->getSpeed());
        _model->setSelectedCombatant(combatant);
        ui->frameCombatant->setCombatant(combatant);

        CombatantWidget* widget = _combatantWidgets.value(combatant, nullptr);
        if(widget)
            ui->scrollArea->ensureWidgetVisible(widget);
    }
}

void BattleFrame::handleItemMoved(QGraphicsPixmapItem* item, bool* result)
{
    Q_UNUSED(result);

    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible handle the item movement, no battle model is set!";
        return;
    }

    updateMovement(item);
}

void BattleFrame::handleItemMouseUp(QGraphicsPixmapItem* item)
{
    Q_UNUSED(item);

    if(!_model)
        return;

    endMovement();
    _model->setSelectedCombatant(nullptr);
}

void BattleFrame::handleItemChanged(QGraphicsItem* item)
{
    if((_publishRect) && (_publishRect == item))
    {
        updateCameraRect();
        createPrescaledBackground();
    }
}

void BattleFrame::handleItemMouseDoubleClick(QGraphicsPixmapItem* item)
{
    if(!item)
        return;

    BattleDialogModelCombatant* combatant = _combatantIcons.key(item);
    if(!combatant)
        return;

    CombatantWidget* widget = _combatantWidgets.value(combatant, nullptr);
    if(!widget)
        return;

    widget->selectCombatant();
}

void BattleFrame::removeCombatant()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to remove combatant, no battle model is set!";
        return;
    }

    if(!_contextMenuCombatant)
        return;

    qDebug() << "[Battle Frame] removing combatant " << _contextMenuCombatant->getName();
    removeRollover();

    // Check the active combatant highlight
    if(_contextMenuCombatant == _model->getActiveCombatant())
    {
        if(_model->getCombatantCount() <= 1)
        {
            _model->setActiveCombatant(nullptr);
            qDebug() << "[Battle Frame] removed combatant has highlight, highlight removed";
        }
        else
        {
            next();
        }
    }
    else if(_contextMenuCombatant == ui->frameCombatant->getCombatant())
    {
        ui->frameCombatant->setCombatant(nullptr);
    }

    // Find the index of the removed item
    int i = _model->getCombatantList().indexOf(_contextMenuCombatant);

    // Delete the widget for the combatant
    QLayoutItem *child = _combatantLayout->takeAt(i);
    if(child != nullptr)
    {
        qDebug() << "[Battle Frame] deleting combatant widget: " << reinterpret_cast<quint64>(child->widget());
        child->widget()->deleteLater();
        delete child;
    }

    // Remove the combatant
    BattleDialogModelCombatant* combatant = _model->removeCombatant(i);
    if(combatant)
    {
        qDebug() << "[Battle Frame] removing combatant from list " << combatant->getName();

        // Remove the widget from the list of widgets
        // TODO: should this be above with deleting the widget for the combatant?
        _combatantWidgets.remove(combatant);

        // Remove the icon from the list of icons and delete it from the scene
        QGraphicsPixmapItem* item = _combatantIcons.take(combatant);
        if(item)
        {
            delete item;
        }

        delete combatant;
    }
}

void BattleFrame::activateCombatant()
{
    if(!_contextMenuCombatant)
        return;

    qDebug() << "[Battle Frame] activating combatant " << _contextMenuCombatant->getName();
    setActiveCombatant(_contextMenuCombatant);
}

void BattleFrame::damageCombatant()
{
    if(!_contextMenuCombatant)
        return;

    int damage = QInputDialog::getInt(this, QString("Damage Combatant"), QString("Please enter the amount of damage to be done: "));

    _contextMenuCombatant->setHitPoints(_contextMenuCombatant->getHitPoints() - damage);
    updateCombatantWidget(_contextMenuCombatant);
    updateCombatantVisibility();

    registerCombatantDamage(_contextMenuCombatant, -damage);
}

void BattleFrame::setSelectedCombatant(BattleDialogModelCombatant* selected)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to select combatant, no battle model is set!";
        return;
    }

    if(!selected)
    {
        setUniqueSelection(nullptr);
        return;
    }

    CombatantWidget* combatantWidget = nullptr;
    QGraphicsPixmapItem* selectedItem = nullptr;
    if(selected)
    {
        bool isSelected = selected->getSelected();
        combatantWidget = getWidgetFromCombatant(selected);
        selectedItem = _combatantIcons.value(selected, nullptr);
        if(combatantWidget)
        {
            combatantWidget->setSelected(!isSelected);
        }

        if(selectedItem)
            selectedItem->setSelected(!isSelected);

        ui->frameCombatant->setCombatant(selected);
    }
}

void BattleFrame::setUniqueSelection(BattleDialogModelCombatant* selected)
{
    if(!_model)
        return;

    for(int i = 0; i < _model->getCombatantCount(); ++i)
    {
        BattleDialogModelCombatant* combatant = _model->getCombatant(i);
        if(combatant)
            combatant->setSelected(combatant == selected);
    }

    ui->frameCombatant->setCombatant(selected);
}

void BattleFrame::updateCombatantWidget(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    CombatantWidget* widget = _combatantWidgets.value(combatant);
    if(!widget)
        return;

    widget->updateData();
    if(ui->frameCombatant->getCombatant() == combatant)
        ui->frameCombatant->setCombatant(combatant);
}

void BattleFrame::updateCombatantIcon(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    QGraphicsPixmapItem* item = _combatantIcons.value(combatant, nullptr);
    if(!item)
        return;

    QPixmap pix = combatant->getIconPixmap(DMHelper::PixmapSize_Battle);
    if(combatant->hasCondition(Combatant::Condition_Unconscious))
    {
        QImage originalImage = pix.toImage();
        QImage grayscaleImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
        pix = QPixmap::fromImage(grayscaleImage);
    }
    Combatant::drawConditions(&pix, combatant->getConditions());
    item->setPixmap(pix);
    item->setOffset(-static_cast<qreal>(pix.width())/2.0, -static_cast<qreal>(pix.height())/2.0);

    QString itemTooltip = QString("<b>") + combatant->getName() + QString("</b>");
    QStringList conditionString = Combatant::getConditionString(combatant->getConditions());
    if(conditionString.count() > 0)
        itemTooltip += QString("<p>") + conditionString.join(QString("<br/>"));
    item->setToolTip(itemTooltip);
}

void BattleFrame::registerCombatantDamage(BattleDialogModelCombatant* combatant, int damage)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to register combatant damage, no battle model is set!";
        return;
    }

    if((!combatant) || (!_model->getActiveCombatant()))
        return;

    if((combatant->getHitPoints() <= 0) && (!combatant->hasCondition(Combatant::Condition_Unconscious)))
    {
        combatant->applyConditions(Combatant::Condition_Unconscious);
        updateCombatantIcon(combatant);
    }
    else if((combatant->getHitPoints() > 0) && (combatant->hasCondition(Combatant::Condition_Unconscious)))
    {
        combatant->removeConditions(Combatant::Condition_Unconscious);
        updateCombatantIcon(combatant);
    }

    if(_logger)
        _logger->damageDone(_model->getActiveCombatant()->getID(), combatant->getID(), damage);
}

void BattleFrame::publishImage()
{
    if((_model) && (_publishing))
    {
        if(!_publishTimer->isActive())
        {
            emit showPublishWindow();
            if(!_model->getMap()->isInitialized())
            {
                createVideoPlayer(false);
            }

            // OPTIMIZE: optimize this to be faster, doing only changes?
            _publishTimer->start(DMHelper::ANIMATION_TIMER_DURATION);
            emit animationStarted(_battle);
            qDebug() << "[Battle Frame] publish timer activated";
        }
    }
}

void BattleFrame::executeAnimateImage()
{
    QImage pub;
    getImageForPublishing(pub);
    emit animateImage(pub);
}

void BattleFrame::updateHighlights()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to update highlights, no battle model is set!";
        return;
    }

    QGraphicsPixmapItem* item;

    if(_activePixmap)
    {
        item = _combatantIcons.value(_model->getActiveCombatant(), nullptr);
        if(item)
        {
            moveRectToPixmap(_activePixmap, item);
            _activePixmap->show();
        }
        else
        {
            _activePixmap->hide();
        }
    }
}

void BattleFrame::countdownTimerExpired()
{
    if(_countdown > 0.0)
    {
        _countdown -= (static_cast<qreal>(_countdownFrame.height() - 10) / static_cast<qreal>(_countdownDuration)) * COUNTDOWN_TIMER;
        if(_countdown <= 0.0)
        {
            _countdown = 0.0;
            _countdownTimer->stop();
        }
    }

    updateCountdownText();
}

void BattleFrame::updateCountdownText()
{
    int countdownInt = static_cast<int>(_countdown);
    ui->edtCountdown->setText(QString::number(countdownInt));

    int halfMaxVal = (_countdownFrame.height() - 10) / 2;
    if(halfMaxVal <= 0)
        return;

    if(countdownInt > halfMaxVal)
    {
        _countdownColor.setRed(196 - (196 * (countdownInt - halfMaxVal) / halfMaxVal));
        _countdownColor.setGreen(196);
    }
    else
    {
        _countdownColor.setRed(196);
        _countdownColor.setGreen(196 * countdownInt / halfMaxVal);
    }

    QString style = "color: " + _countdownColor.name() + ";";
    ui->edtCountdown->setStyleSheet(style);
}

void BattleFrame::createPrescaledBackground()
{
    qDebug() << "[Battle Frame] Creating Prescaled Background";

    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to create a prescaled background, no battle model is set!";
        return;
    }

    if((!_model->getMap()) || (!_publishing))
        return;

    if(_videoPlayer)
    {
        QSize oldVideoSize = _videoSize;
        resetVideoSizes();
        if(_videoSize != oldVideoSize)
            _videoPlayer->restartPlayer();
        return;
    }

    QRect sourceRect;
    QRect viewportScene = _publishRectValue.isValid() ? getCameraRect().toRect() : ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect().toAlignedRect();
    QRect sceneRect = ui->graphicsView->sceneRect().toRect();
    sourceRect = viewportScene.intersected(sceneRect);

#ifdef BATTLE_DIALOG_PROFILE_PRESCALED_BACKGROUND
    qDebug() << "[Battle Frame][PROFILE] prescaled background being created";
    QTime t;
    t.start();
#endif

    QImage battleMap = _model->getMap()->getPublishImage().copy(sourceRect);
    battleMap.convertTo(QImage::Format_ARGB32_Premultiplied);

    QSize imageSize = getRotatedTargetBackgroundSize(battleMap.size());
    qreal scaleFactor = static_cast<qreal>(imageSize.width()) / static_cast<qreal>(battleMap.width());
    if((scaleFactor * static_cast<qreal>(battleMap.height())) > static_cast<qreal>(imageSize.height()))
        scaleFactor = static_cast<qreal>(imageSize.height()) / static_cast<qreal>(battleMap.height());

    _prescaledBackground = QPixmap::fromImage(battleMap.transformed(QTransform().rotate(_rotation).scale(scaleFactor,scaleFactor), Qt::SmoothTransformation));

#ifdef BATTLE_DIALOG_PROFILE_PRESCALED_BACKGROUND
    qDebug() << "[Battle Frame][PROFILE] " << t.elapsed() << "; prescaled background created";
#endif

    qDebug() << "[Battle Frame] Prescaled Background created";
}

void BattleFrame::handleRubberBandChanged(QRect rubberBandRect, QPointF fromScenePoint, QPointF toScenePoint)
{
    Q_UNUSED(fromScenePoint);
    Q_UNUSED(toScenePoint);

    if((_stateMachine.getCurrentStateId() != DMHelper::BattleFrameState_ZoomSelect) &&
       (_stateMachine.getCurrentStateId() != DMHelper::BattleFrameState_CameraSelect) &&
       (_stateMachine.getCurrentStateId() != DMHelper::BattleFrameState_FoWSelect))
        return;

    if(rubberBandRect.isNull())
    {
        if(_stateMachine.getCurrentStateId() == DMHelper::BattleFrameState_ZoomSelect)
        {
            ui->graphicsView->fitInView(ui->graphicsView->mapToScene(_rubberBandRect).boundingRect(), Qt::KeepAspectRatio);
            _scale = ui->graphicsView->transform().m11();
        }
        else if(_stateMachine.getCurrentStateId() == DMHelper::BattleFrameState_CameraSelect)
        {
            if(_publishRect)
                _publishRect->setCameraRect(ui->graphicsView->mapToScene(_rubberBandRect).boundingRect());
        }
        else if(_stateMachine.getCurrentStateId() == DMHelper::BattleFrameState_FoWSelect)
        {
            if(_mapDrawer)
                _mapDrawer->drawRect(ui->graphicsView->mapToScene(_rubberBandRect).boundingRect().toRect());
        }

        if(_scene)
            _scene->clearSelection();

        cancelSelect();
    }
    else
    {
        _rubberBandRect = rubberBandRect;
    }
}

void BattleFrame::setCombatantVisibility(bool aliveVisible, bool deadVisible, bool widgetsIncluded)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set combatant visibility, no battle model is set!";
        return;
    }

    for(int i = 0; i < _model->getCombatantCount(); ++i)
    {
        BattleDialogModelCombatant* combatant = _model->getCombatant(i);
        if(combatant)
        {
            bool vis = ((combatant->getHitPoints() > 0) || (combatant->getCombatantType() == DMHelper::CombatantType_Character)) ? aliveVisible : deadVisible;

            if(widgetsIncluded)
            {
                QWidget* widget = _combatantLayout->itemAt(i)->widget();
                if(widget)
                {
                    widget->setVisible(vis);
                }
            }

            QGraphicsPixmapItem* item = _combatantIcons.value(combatant);
            if(item)
                item->setVisible(vis);

            // Set the visibility of the active rect
            if((_activePixmap) && (combatant == _model->getActiveCombatant()))
                _activePixmap->setVisible(vis);
        }
    }
}

void BattleFrame::setEffectLayerVisibility(bool visibility)
{
    // NOTE: if effect layer visibility is used, need to sync with setGridVisibility usage
    if(_scene)
    {
        _scene->setEffectVisibility(visibility);
        ui->graphicsView->invalidateScene();
    }
}

void BattleFrame::setPublishVisibility(bool publish)
{
#ifdef BATTLE_DIALOG_LOG_VIDEO
    qDebug() << "[Battle Frame] Setting publish visibility: " << publish;
#endif
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set publish visibility, no battle model is set!";
        return;
    }

    // The background will be rendered separately for the publish image
    if(_background)
        _background->setVisible(!publish);

    // Don't render the camera rect for the publish image
    if(_publishRect)
        _publishRect->setDraw(!publish);

    // Don't render invisible individual effects
    if((_scene) && (_model->getShowEffects()))
        _scene->setEffectVisibility(true, !publish);

    // The grid is rendered separately for videos
    if((_model->getGridOn()) && (_videoPlayer) && (_scene))
        _scene->setGridVisibility(!publish);

    // Render the pointer if and only if the pointer is active
    if((publish) && (_stateMachine.getCurrentStateId() == DMHelper::BattleFrameState_Pointer))
        _scene->setPointerVisibility(true);
    else
        _scene->setPointerVisibility(false);

    // Iterate through the combatants and set those combatants to invisible which are not to be shown
    QList<BattleDialogModelCombatant*> combatantList = _model->getCombatantList();

    for(BattleDialogModelCombatant* combatant : combatantList)
    {
        if(combatant)
        {
            if((!combatant->getShown())||(!combatant->getKnown()))
            {
                QGraphicsPixmapItem* item = _combatantIcons.value(combatant);
                if(item)
                {
                    UnselectedPixmap* unselectedItem = dynamic_cast<UnselectedPixmap*>(item);
                    if(unselectedItem)
                    {
                        unselectedItem->setDraw(!publish);
                        // Since we aren't making the item invisible, we have to manually make it's children invisible
                        if(item->childItems().count() > 0)
                        {
                            for(QGraphicsItem* child : item->childItems())
                            {
                                if((child) && (child->data(BattleDialogItemChild_Index).toInt() != BattleDialogItemChild_Area))
                                    child->setVisible(!publish);
                            }
                        }
                    }
                    else
                    {
                        item->setVisible(!publish);
                    }

                    if((combatant == _model->getSelectedCombatant()) && (_movementPixmap))
                        _movementPixmap->setVisible(!publish);

                    if((_activePixmap) && (combatant == _model->getActiveCombatant()))
                        _activePixmap->setVisible(!publish);
                }
            }
        }
    }

#ifdef BATTLE_DIALOG_LOG_VIDEO
    qDebug() << "[Battle Frame] Publish visibility set";
#endif
}

void BattleFrame::setGridOnlyVisibility(bool gridOnly)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set grid only visibility, no battle model is set!";
        return;
    }

    if(!_model->getGridOn())
        return;

    if(_background)
        _background->setVisible(!gridOnly);

    if(_publishRect)
        _publishRect->setDraw(!gridOnly);

    setEffectLayerVisibility((!gridOnly) && (_model->getShowEffects()));

    if(gridOnly)
    {
        if(_activePixmap)
            _activePixmap->setVisible(false);
        if(_movementPixmap)
            _movementPixmap->setVisible(false);
        setCombatantVisibility(false, false, true);
    }
    else
    {
        setCombatantVisibility(_model->getShowAlive(), _model->getShowDead(), true);
    }
}

void BattleFrame::setMapCursor()
{
}

void BattleFrame::setCameraSelectable(bool selectable)
{
    if(_publishRect)
        _publishRect->setCameraSelectable(selectable);
}

void BattleFrame::setScale(qreal s)
{
    _scale = s;
    ui->graphicsView->scale(s,s);
    createPrescaledBackground();
    setMapCursor();
    storeViewRect();

    if(_scene)
        _scene->scaleBattleContents();
}

void BattleFrame::storeViewRect()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to store view rect, no battle model is set!";
        return;
    }

    _model->setMapRect(ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect().toAlignedRect());

    if(_cameraCoupled)
        setCameraToView();

    if(_compassPixmap)
    {
        QPoint origin = ui->graphicsView->mapFromScene(QPoint(0,0));
        _compassPixmap->setPos(ui->graphicsView->mapToScene(qMax(0, origin.x()), qMax(0, origin.y())));
        if(ui->graphicsView->transform().m11() > 0.0)
        {
            _compassPixmap->setScale(COMPASS_SCALE/ui->graphicsView->transform().m11());
        }
    }
}

void BattleFrame::setModel(BattleDialogModel* model)
{
    qDebug() << "[Battle Frame] Setting battle model to: " << model;

    _model = model;

    if(_scene)
        _scene->setModel(model);

    ui->btnSort->setEnabled(_model != nullptr);
    ui->btnNext->setEnabled(_model != nullptr);
    ui->edtRounds->setEnabled(_model != nullptr);
    ui->edtCountdown->setEnabled(_model != nullptr);
    updatePublishEnable();
    ui->graphicsView->setEnabled(_model != nullptr);

    if(!_model)
    {
        clearBattleFrame();
        cleanupBattleMap();
        clearCombatantWidgets();
    }
    else
    {
        emit backgroundColorChanged(_model->getBackgroundColor());

        connect(_model, SIGNAL(showAliveChanged(bool)), this, SLOT(updateCombatantVisibility()));
        connect(_model, SIGNAL(showDeadChanged(bool)), this, SLOT(updateCombatantVisibility()));
        connect(_model, SIGNAL(showEffectsChanged(bool)), this, SLOT(updateEffectLayerVisibility()));

        setBattleMap();
        recreateCombatantWidgets();

        if(!_logger)
        {
            _logger = new BattleDialogLogger(this);
            connect(_logger, SIGNAL(roundsChanged()), this, SLOT(updateRounds()));
            updateRounds();
        }
    }

    emit modelChanged(_model);
}

Map* BattleFrame::selectRelatedMap()
{
    if(!_battle)
        return nullptr;

    Campaign* campaign = dynamic_cast<Campaign*>(_battle->getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return nullptr;

    CampaignObjectBase* parentObject = dynamic_cast<CampaignObjectBase*>(_battle->parent());
    if(!parentObject)
        return nullptr;

    // TODO: Check what happens if separator is selected
    MapSelectDialog mapSelectDlg;

    QList<Map*> allMaps = campaign->findChildren<Map*>();
    for(Map* map : allMaps)
    {
        if(map)
        {
            if(map->getParentById(parentObject->getID()) != nullptr)
                mapSelectDlg.prependMap(map);
            else
                mapSelectDlg.appendMap(map);
        }
    }

    if(mapSelectDlg.exec() != QDialog::Accepted)
        return nullptr;
    else
        return mapSelectDlg.getSelectedMap();
}

void BattleFrame::selectAddCharacter(QList<Character*> characters, const QString& title, const QString& label)
{
    if(characters.isEmpty())
        return;

    ItemSelectDialog characterSelectDlg;
    characterSelectDlg.setWindowTitle(title);
    characterSelectDlg.setLabel(label);

    QList<Character*>::iterator i;
    for(i = characters.begin(); i != characters.end(); ++i)
    {
        Character* character = *i;
        if(character != nullptr)
            characterSelectDlg.addItem(character->getName(), QVariant::fromValue(character));
    }

    if(characterSelectDlg.getItemCount() > 0)
    {
        if(characterSelectDlg.exec() == QDialog::Accepted)
        {
            Character* selectedCharacter = characterSelectDlg.getSelectedData().value<Character*>();
            if(selectedCharacter)
            {
                BattleDialogModelCharacter* newCharacter = new BattleDialogModelCharacter(selectedCharacter);
                newCharacter->setPosition(viewportCenter());
                addCombatant(newCharacter);
                recreateCombatantWidgets();
                qDebug() << "[Battle Frame] ...character " << selectedCharacter->getName() << " added.";
            }
        }
        else
        {
            qDebug() << "[Battle Frame] ...add character dialog cancelled";
        }
    }
}

void BattleFrame::setEditMode()
{
    qDebug() << "[Battle Frame] FoW Edit Mode set. Mode: " << _stateMachine.getCurrentStateId();

    if(_stateMachine.getCurrentStateId() == DMHelper::BattleFrameState_FoWEdit)
    {
        disconnect(_scene, SIGNAL(itemMouseDown(QGraphicsPixmapItem*)), this, SLOT(handleItemMouseDown(QGraphicsPixmapItem*)));
        disconnect(_scene, SIGNAL(itemMouseUp(QGraphicsPixmapItem*)), this, SLOT(handleItemMouseUp(QGraphicsPixmapItem*)));
        disconnect(_scene, SIGNAL(itemMouseDoubleClick(QGraphicsPixmapItem*)), this, SLOT(handleItemMouseDoubleClick(QGraphicsPixmapItem*)));
        disconnect(_scene, SIGNAL(itemMoved(QGraphicsPixmapItem*, bool*)), this, SLOT(handleItemMoved(QGraphicsPixmapItem*, bool*)));
        disconnect(_scene, SIGNAL(mapMouseUp()), this, SLOT(mapClickDetected()));

        connect(_scene, SIGNAL(battleMousePress(const QPointF&)), _mapDrawer, SLOT(handleMouseDown(const QPointF&)));
        connect(_scene, SIGNAL(battleMouseMove(const QPointF&)), _mapDrawer, SLOT(handleMouseMoved(const QPointF&)));
        connect(_scene, SIGNAL(battleMouseRelease(const QPointF&)), _mapDrawer, SLOT(handleMouseUp(const QPointF&)));

        if(_model)
        {
            QRect viewSize = ui->graphicsView->mapFromScene(QRect(0, 0, _model->getGridScale(), _model->getGridScale())).boundingRect();
            _mapDrawer->setScale(_model->getGridScale(), viewSize.width());
//            TODO _mapDrawer->setSize(ui->spinSize->value());
        }
    }
    else
    {
        disconnect(_scene, SIGNAL(battleMousePress(const QPointF&)), _mapDrawer, SLOT(handleMouseDown(const QPointF&)));
        disconnect(_scene, SIGNAL(battleMouseMove(const QPointF&)), _mapDrawer, SLOT(handleMouseMoved(const QPointF&)));
        disconnect(_scene, SIGNAL(battleMouseRelease(const QPointF&)), _mapDrawer, SLOT(handleMouseUp(const QPointF&)));

        connect(_scene, SIGNAL(itemMouseDown(QGraphicsPixmapItem*)), this, SLOT(handleItemMouseDown(QGraphicsPixmapItem*)));
        connect(_scene, SIGNAL(itemMouseUp(QGraphicsPixmapItem*)), this, SLOT(handleItemMouseUp(QGraphicsPixmapItem*)));
        connect(_scene, SIGNAL(itemMouseDoubleClick(QGraphicsPixmapItem*)), this, SLOT(handleItemMouseDoubleClick(QGraphicsPixmapItem*)));
        connect(_scene, SIGNAL(itemMoved(QGraphicsPixmapItem*, bool*)), this, SLOT(handleItemMoved(QGraphicsPixmapItem*, bool*)));
        connect(_scene, SIGNAL(mapMouseUp()), this, SLOT(mapClickDetected()));
    }
}

void BattleFrame::updateFowImage(const QPixmap& fow)
{
    if(_fow)
        _fow->setPixmap(fow);

    _bwFoWImage = QImage();
}

void BattleFrame::setItemsInert(bool inert)
{
    qDebug() << "[Battle Frame] Setting inert value for all items. Mode: " << _stateMachine.getCurrentStateId() << ", inert = " << inert;

    bool enabled = !inert;

    for(QGraphicsPixmapItem* item : _combatantIcons.values())
    {
        if(item)
        {
            item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
            item->setFlag(QGraphicsItem::ItemIsMovable, enabled);
        }
    }

    QList<QGraphicsItem*> effects = _scene->getEffectItems();
    for(QGraphicsItem* effect : effects)
    {
        if(effect)
        {
            effect->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
            effect->setFlag(QGraphicsItem::ItemIsMovable, enabled);
        }
    }
}

void BattleFrame::removeRollover()
{
    if(!_hoverFrame)
        return;

    qDebug() << "[Battle Frame] Removing rollover";

    _hoverFrame->cancelClose();
    _hoverFrame->deleteLater();
    _hoverFrame = nullptr;
}

void BattleFrame::stateUpdated()
{
    BattleFrameState* currentState = _stateMachine.getCurrentState();

    if((currentState == nullptr) || (currentState->getType() == BattleFrameState::BattleFrameStateType_Base))
    {
        ui->graphicsView->viewport()->unsetCursor();
    }
    else
    {
        ui->graphicsView->viewport()->setCursor(currentState->getCursor());
    }
}

void BattleFrame::mapClickDetected()
{
    if(_battle)
        emit animationUpdated(_battle);
}

CombatantWidget* BattleFrame::createCombatantWidget(BattleDialogModelCombatant* combatant)
{
    CombatantWidget* newWidget = nullptr;

    if(_combatantWidgets.contains(combatant))
    {
        newWidget = _combatantWidgets.value(combatant);
        qDebug() << "[Battle Frame] found widget for combatant " << combatant->getName() << ": " << reinterpret_cast<quint64>(newWidget);
        return newWidget;
    }

    switch(combatant->getCombatantType())
    {
        case DMHelper::CombatantType_Character:
        {
            BattleDialogModelCharacter* character = dynamic_cast<BattleDialogModelCharacter*>(combatant);
            if(character)
            {
                qDebug() << "[Battle Frame] creating character widget for " << character->getName();
                newWidget = new WidgetCharacter(ui->scrollAreaWidgetContents);
                WidgetCharacterInternal* widgetInternals = new WidgetCharacterInternal(character, dynamic_cast<WidgetCharacter*>(newWidget));
                connect(widgetInternals, SIGNAL(clicked(QUuid)),this,SIGNAL(characterSelected(QUuid)));
                connect(widgetInternals, SIGNAL(contextMenu(BattleDialogModelCombatant*,QPoint)), this, SLOT(handleContextMenu(BattleDialogModelCombatant*,QPoint)));
                connect(widgetInternals, SIGNAL(hitPointsChanged(BattleDialogModelCombatant*,int)), this, SLOT(updateCombatantVisibility()));
                connect(widgetInternals, SIGNAL(hitPointsChanged(BattleDialogModelCombatant*,int)), this, SLOT(registerCombatantDamage(BattleDialogModelCombatant*, int)));
                connect(dynamic_cast<WidgetCharacter*>(newWidget), SIGNAL(isShownChanged(bool)), character, SLOT(setShown(bool)));
                connect(dynamic_cast<WidgetCharacter*>(newWidget), SIGNAL(isKnownChanged(bool)), character, SLOT(setKnown(bool)));
                connect(newWidget, SIGNAL(imageChanged(BattleDialogModelCombatant*)), this, SLOT(updateCombatantIcon(BattleDialogModelCombatant*)));
            }
            break;
        }
        case DMHelper::CombatantType_Monster:
        {
            BattleDialogModelMonsterBase* monster = dynamic_cast<BattleDialogModelMonsterBase*>(combatant);
            if(monster)
            {
                qDebug() << "[Battle Frame] creating monster widget for " << monster->getName();
                newWidget = new WidgetMonster(ui->scrollAreaWidgetContents);
                WidgetMonsterInternal* widgetInternals = new WidgetMonsterInternal(monster, dynamic_cast<WidgetMonster*>(newWidget));
                connect(widgetInternals, SIGNAL(clicked(const QString&)),this,SIGNAL(monsterSelected(const QString&)));
                connect(widgetInternals, SIGNAL(contextMenu(BattleDialogModelCombatant*,QPoint)), this, SLOT(handleContextMenu(BattleDialogModelCombatant*,QPoint)));
                connect(widgetInternals, SIGNAL(hitPointsChanged(BattleDialogModelCombatant*,int)), this, SLOT(updateCombatantVisibility()));
                connect(widgetInternals, SIGNAL(hitPointsChanged(BattleDialogModelCombatant*,int)), this, SLOT(registerCombatantDamage(BattleDialogModelCombatant*, int)));
                connect(dynamic_cast<WidgetMonster*>(newWidget), SIGNAL(isShownChanged(bool)), monster, SLOT(setShown(bool)));
                connect(dynamic_cast<WidgetMonster*>(newWidget), SIGNAL(isKnownChanged(bool)), monster, SLOT(setKnown(bool)));
                connect(newWidget, SIGNAL(imageChanged(BattleDialogModelCombatant*)), this, SLOT(updateCombatantIcon(BattleDialogModelCombatant*)));
            }
            break;
        }
        default:
            qDebug() << "[Battle Frame] Unknown combatant type found in battle! Type: " << combatant->getCombatantType() << " Name: " << combatant->getName();
            break;
    }

    if(newWidget)
    {
        newWidget->installEventFilter(this);
        _combatantWidgets.insert(combatant, newWidget);
        qDebug() << "[Battle Frame] new widget inserted and event filter established: " << reinterpret_cast<quint64>(newWidget);
    }

    return newWidget;
}

void BattleFrame::clearCombatantWidgets()
{
    qDebug() << "[Battle Frame] Deleting combatant widgets";

    if(!_combatantLayout)
    {
        qDebug() << "[Battle Frame]     No combatant widgets found.";
        return;
    }

    qDebug() << "[Battle Frame]     " << _combatantLayout->count() << " widgets to be deleted.";
    QLayoutItem *child;
    while ((child = _combatantLayout->takeAt(0)) != nullptr)
        delete child;
}

void BattleFrame::buildCombatantWidgets()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set combatant widgets, no battle model is set!";
        return;
    }

    qDebug() << "[Battle Frame] building combatant widgets. count: " << _model->getCombatantCount();

    if(_model->getCombatantCount() == 0)
        return;

    for(int i = 0; i < _model->getCombatantCount(); ++i)
    {
        CombatantWidget* newWidget = createCombatantWidget(_model->getCombatant(i));

        if(newWidget)
        {
            _combatantLayout->addWidget(newWidget);
            newWidget->setActive(false);
        }
    }

    setCombatantVisibility(_model->getShowAlive(), _model->getShowDead(), true);
    if(_model->getActiveCombatant())
        setActiveCombatant(_model->getActiveCombatant());
    else
        setActiveCombatant(getFirstLivingCombatant());
}

void BattleFrame::reorderCombatantWidgets()
{
    qDebug() << "[Battle Frame] resetting combatant widget order";

    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to reset combatant widgets, no battle model is set!";
        return;
    }

    clearCombatantWidgets();
    for(int i = 0; i < _model->getCombatantCount(); ++i)
    {
        CombatantWidget* widget = _combatantWidgets.value(_model->getCombatant(i));
        if(widget)
        {
            _combatantLayout->addWidget(widget);
        }
    }
}

void BattleFrame::setActiveCombatant(BattleDialogModelCombatant* active)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set active combatant, no battle model is set!";
        return;
    }

    CombatantWidget* combatantWidget = getWidgetFromCombatant(_model->getActiveCombatant());
    if(combatantWidget)
    {
        qDebug() << "[Battle Frame] removing active flag from widget " << reinterpret_cast<quint64>(combatantWidget);
        combatantWidget->setActive(false);
    }

    combatantWidget = getWidgetFromCombatant(active);
    if(combatantWidget)
    {
        qDebug() << "[Battle Frame] adding active flag to widget " << reinterpret_cast<quint64>(combatantWidget);
        combatantWidget->setActive(true);
        ui->scrollArea->ensureWidgetVisible(combatantWidget);
    }

    if(active)
    {
        _activeScale = active->getSizeFactor();
        ui->frameCombatant->setCombatant(active);
    }

    if(_activePixmap)
    {
        QGraphicsPixmapItem* item = _combatantIcons.value(active, nullptr);
        if(item)
        {
            _activePixmap->setScale(_model->getGridScale() * _activeScale / ACTIVE_PIXMAP_SIZE);

            moveRectToPixmap(_activePixmap, item);
            _activePixmap->setVisible(item->isVisible());

            _countdownTimer->start(static_cast<int>(COUNTDOWN_TIMER * 1000));
            _countdown = static_cast<qreal>(_countdownFrame.height() - 10);
            updateCountdownText();
        }
        else
        {
            _activePixmap->hide();
        }
    }

    _model->setActiveCombatant(active);
}

void BattleFrame::createCombatantIcon(BattleDialogModelCombatant* combatant)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to create combatant icon, no battle model is set!";
        return;
    }

    if((combatant) && (_scene) && (_background))
    {
        QPixmap pix = combatant->getIconPixmap(DMHelper::PixmapSize_Battle);
        if(combatant->hasCondition(Combatant::Condition_Unconscious))
        {
            QImage originalImage = pix.toImage();
            QImage grayscaleImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
            pix = QPixmap::fromImage(grayscaleImage);
        }
        Combatant::drawConditions(&pix, combatant->getConditions());

        QGraphicsPixmapItem* pixmapItem = new UnselectedPixmap(pix, combatant);
        _scene->addItem(pixmapItem);
        pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true);
        pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
        pixmapItem->setZValue(DMHelper::BattleDialog_Z_Combatant);
        pixmapItem->setPos(combatant->getPosition());
        pixmapItem->setOffset(-static_cast<qreal>(pix.width())/2.0, -static_cast<qreal>(pix.height())/2.0);
        qreal sizeFactor = combatant->getSizeFactor();
        qreal scaleFactor = (static_cast<qreal>(_model->getGridScale()-2)) * sizeFactor / static_cast<qreal>(qMax(pix.width(),pix.height()));
        pixmapItem->setScale(scaleFactor);

        QString itemTooltip = QString("<b>") + combatant->getName() + QString("</b>");
        QStringList conditionString = Combatant::getConditionString(combatant->getConditions());
        if(conditionString.count() > 0)
            itemTooltip += QString("<p>") + conditionString.join(QString("<br/>"));
        pixmapItem->setToolTip(itemTooltip);

        qDebug() << "[Battle Frame] combatant icon added " << combatant->getName() << ", scale " << scaleFactor;

        qreal gridSize = (static_cast<qreal>(_model->getGridScale())) / scaleFactor;
        qreal gridOffset = gridSize * static_cast<qreal>(sizeFactor) / 2.0;
        QGraphicsRectItem* rect = new QGraphicsRectItem(0, 0, gridSize * sizeFactor, gridSize * static_cast<qreal>(sizeFactor));
        rect->setPos(-gridOffset, -gridOffset);
        rect->setData(BattleDialogItemChild_Index, BattleDialogItemChild_Area);
        rect->setParentItem(pixmapItem);
        rect->setVisible(false);
        qDebug() << "[Battle Frame] created " << pixmapItem << " with area child " << rect;

        applyPersonalEffectToItem(pixmapItem);

        _combatantIcons.insert(combatant, pixmapItem);

        connect(combatant, SIGNAL(combatantMoved(BattleDialogModelCombatant*)), this, SLOT(handleCombatantMoved(BattleDialogModelCombatant*)), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
        connect(combatant, SIGNAL(combatantMoved(BattleDialogModelCombatant*)), this, SLOT(updateHighlights()), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
        connect(combatant, SIGNAL(combatantSelected(BattleDialogModelCombatant*)), this, SLOT(handleCombatantSelected(BattleDialogModelCombatant*)));
    }
}

void BattleFrame::relocateCombatantIcon(QGraphicsPixmapItem* icon)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to relocate the combatant icon, no battle model is set!";
        return;
    }

    if(!icon)
        return;

    QPoint mapPos = icon->pos().toPoint() + _model->getPreviousMapRect().topLeft();
    if(_model->getMapRect().contains(mapPos))
    {
        icon->setPos(mapPos - _model->getMapRect().topLeft());
    }
    else
    {
        icon->setPos(10, 10);
    }
}

QWidget* BattleFrame::findCombatantWidgetFromPosition(const QPoint& position) const
{
    qDebug() << "[Battle Frame] searching for widget from position " << position.x() << "x" << position.y() << "...";
    QWidget* widget = ui->scrollAreaWidgetContents->childAt(position);

    if(widget)
    {
        while((widget->parentWidget() != ui->scrollAreaWidgetContents) && (widget->parentWidget() != nullptr))
        {
            widget = widget->parentWidget();
        }

        if(widget->parentWidget() == nullptr)
        {
            qDebug() << "[Battle Frame] ...widget not found";
            return nullptr;
        }
    }

    qDebug() << "[Battle Frame] ...widget found: " << reinterpret_cast<quint64>(widget);
    return widget;
}

CombatantWidget* BattleFrame::getWidgetFromCombatant(BattleDialogModelCombatant* combatant) const
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to get widget from combatant, no battle model is set!";
        return nullptr;
    }

    if(!combatant)
        return nullptr;

    int pos = _model->getCombatantList().indexOf(combatant);
    qDebug() << "[Battle Frame] finding widget for combatant " << combatant << " at " << pos;
    if((pos >= 0) && (pos < _combatantLayout->count()))
    {
        return dynamic_cast<CombatantWidget*>(_combatantLayout->itemAt(pos)->widget());
    }
    else
    {
        return nullptr;
    }
}

void BattleFrame::moveRectToPixmap(QGraphicsItem* rectItem, QGraphicsPixmapItem* pixmapItem)
{
    if((!rectItem) || (!pixmapItem))
        return;

    QRect itemRect = rectItem->boundingRect().toRect();
    rectItem->setPos(pixmapItem->x() + (((pixmapItem->pixmap().width() / 2) + pixmapItem->offset().x()) * pixmapItem->scale()) - (itemRect.width() * rectItem->scale() / 2),
                     pixmapItem->y() + (((pixmapItem->pixmap().height() / 2) + pixmapItem->offset().y()) * pixmapItem->scale()) - (itemRect.height() * rectItem->scale() / 2));
}

BattleDialogModelCombatant* BattleFrame::getNextCombatant(BattleDialogModelCombatant* combatant)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to get the next combatant, no battle model is set!";
        return nullptr;
    }

    if(!combatant)
        return nullptr;

    int nextHighlight = _model->getCombatantList().indexOf(combatant);

    if(_combatantLayout->count() <= 1)
        return nullptr;

    do
    {
        if(++nextHighlight >= _model->getCombatantCount())
        {
            nextHighlight = 0;
        }
    } while( ( (_model->getCombatant(nextHighlight)->getHitPoints() <= 0) ||
               (!_model->getCombatant(nextHighlight)->getKnown()) ) &&
             (_model->getCombatant(nextHighlight) != _model->getActiveCombatant()));

    return _model->getCombatant(nextHighlight);
}

void BattleFrame::getImageForPublishing(QImage& imageForPublishing)
{
#ifdef BATTLE_DIALOG_LOG_VIDEO
    qDebug() << "[Battle Frame] Getting image for publishing" << imageForPublishing;
#endif

#ifdef BATTLE_DIALOG_PROFILE_RENDER
    tProfile.start();
    #ifdef BATTLE_DIALOG_PROFILE_RENDER_TEXT
        qDebug() << "[Battle Frame][PROFILE] Starting Render";
    #endif
#endif

    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to get image for publishing, no battle model is set!";
        return;
    }

    QSize backgroundImageSize = (_videoPlayer && _videoPlayer->getImage()) ? _sourceRect.size() : _prescaledBackground.size();
    QSize unrotatedBackgroundImageSize = backgroundImageSize;
    if((!_videoPlayer || !_videoPlayer->getImage()) && ((_rotation == 90) || (_rotation == 270)))
        unrotatedBackgroundImageSize.transpose();

    //TODO: premultiplied format and draw directly to the return value image???
    QSize tempRotFrameSize;
    if(_videoPlayer && _videoPlayer->getImage())
    {
        tempRotFrameSize = sizeBackgroundToFrame(backgroundImageSize);
        if((_rotation == 90) || (_rotation == 270))
            tempRotFrameSize.transpose();
    }
    else
    {
        tempRotFrameSize = getRotatedTargetFrameSize(backgroundImageSize);
    }

    QImage drawingImageForPublishing = QImage(tempRotFrameSize, QImage::Format_ARGB32_Premultiplied);
    drawingImageForPublishing.fill(_model->getBackgroundColor());

    QPainter painter;
    painter.begin(&drawingImageForPublishing);

#ifdef BATTLE_DIALOG_PROFILE_RENDER
    tBasicPrep = tProfile.restart();
    #ifdef BATTLE_DIALOG_PROFILE_RENDER_TEXT
        qDebug() << "[Battle Frame][PROFILE] " << tProfile.restart() << "; basic preparation complete";
    #endif
#endif

    // For a static image, the pre-rendered background image is pre-rotated, so we should render it now before setting a rotation in the painter
    if(!_videoPlayer)
        renderPrescaledBackground(painter, tempRotFrameSize);

    if(_rotation != 0)
    {
        int rotatePoint = qMax(tempRotFrameSize.width(),tempRotFrameSize.height()) / 2;

        painter.translate(rotatePoint, rotatePoint);
        painter.rotate(_rotation);
        painter.translate(-rotatePoint, -rotatePoint);

        if(_rotation == 90)
            painter.translate(0,tempRotFrameSize.height() - tempRotFrameSize.width());
        if(_rotation == 180)
            painter.translate(0,tempRotFrameSize.width() - tempRotFrameSize.height());
    }

    // For a video image, the image is not pre-rotated, so we should render it now after setting the rotation in the painter
    if(_videoPlayer)
        renderVideoBackground(painter);

    // Draw the contents of the battle frame in publish mode
    QRect viewportRect = _publishRectValue.isValid() ? ui->graphicsView->mapFromScene(getCameraRect()).boundingRect() : ui->graphicsView->viewport()->rect();
    QRect sceneViewportRect = ui->graphicsView->mapFromScene(ui->graphicsView->sceneRect()).boundingRect();
    QRect sourceRect = viewportRect.intersected(sceneViewportRect);
    setPublishVisibility(true);
    ui->graphicsView->render(&painter,
                             QRectF(QPointF(0,0),unrotatedBackgroundImageSize),
                             sourceRect);
    setPublishVisibility(false);

#ifdef BATTLE_DIALOG_PROFILE_RENDER
    tContent = tProfile.restart();
    #ifdef BATTLE_DIALOG_PROFILE_RENDER_TEXT
        qDebug() << "[Battle Frame][PROFILE] " << tProfile.restart() << "; contents drawn";
    #endif
#endif

    // Draw the active combatant image on top
    if(_model->getActiveCombatant())
    {
        BattleDialogModelCombatant* nextCombatant = nullptr;

        if(_showOnDeck)
        {
            QPixmap pmp;
            if(_model->getActiveCombatant()->getShown())
                pmp = _model->getActiveCombatant()->getIconPixmap(DMHelper::PixmapSize_Animate);
            else
                pmp = ScaledPixmap::defaultPixmap()->getPixmap(DMHelper::PixmapSize_Animate);

            painter.drawImage(unrotatedBackgroundImageSize.width(),
                              0,
                              _combatantFrame);
            int dx = qMax(5, (_combatantFrame.width()-pmp.width())/2);
            int dy = qMax(5, (_combatantFrame.height()-pmp.height())/2);
            painter.drawPixmap(unrotatedBackgroundImageSize.width() + dx,
                               dy,
                               pmp);
            int conditions = _model->getActiveCombatant()->getConditions();
            if(conditions > 0)
            {
                int cx = 5;
                int cy = 5;
                for(int i = 0; i < Combatant::getConditionCount(); ++i)
                {
                    int condition = Combatant::getConditionByIndex(i);
                    if((conditions & condition) && (cy <= _combatantFrame.height() - 30))
                    {
                        QPixmap conditionPixmap(QString(":/img/data/img/") + Combatant::getConditionIcon(condition) + QString(".png"));
                        painter.drawPixmap(unrotatedBackgroundImageSize.width() + cx,
                                           cy,
                                           conditionPixmap.scaled(30,30));
                        cx += 30;
                        if(cx > _combatantFrame.width() - 30)
                        {
                            cx = 5;
                            cy += 30;
                        }
                    }
                }
            }

            nextCombatant = getNextCombatant(_model->getActiveCombatant());
        }

        if(_showCountdown)
        {
            int xPos = unrotatedBackgroundImageSize.width();
            if(_showOnDeck)
                xPos += _combatantFrame.width();
            if(_countdown > 0.0)
            {
                int countdownInt = static_cast<int>(_countdown);
                painter.setBrush(QBrush(_countdownColor));
                painter.drawRect(xPos + 5,
                                 _countdownFrame.height() - countdownInt - 5,
                                 10,
                                 countdownInt);
            }
            painter.drawImage(xPos,
                              0,
                              _countdownFrame);
        }

        if(_showOnDeck)
        {
            int yPos = DMHelper::PixmapSizes[DMHelper::PixmapSize_Animate][1] + 10;
            while((nextCombatant) &&
                  (nextCombatant != _model->getActiveCombatant()) &&
                  (yPos + _combatantFrame.height() < tempRotFrameSize.height()))
            {
                QPixmap nextPmp;
                if(nextCombatant->getShown())
                    nextPmp = nextCombatant->getIconPixmap(DMHelper::PixmapSize_Animate);
                else
                    nextPmp = ScaledPixmap::defaultPixmap()->getPixmap(DMHelper::PixmapSize_Animate);
                painter.drawImage(unrotatedBackgroundImageSize.width(),
                                  yPos,
                                  _combatantFrame);
                int dx = qMax(5, (_combatantFrame.width()-nextPmp.width())/2);
                int dy = qMax(5, (_combatantFrame.height()-nextPmp.height())/2);
                painter.drawPixmap(unrotatedBackgroundImageSize.width() + dx,
                                   yPos + dy,
                                   nextPmp);
                int conditions = nextCombatant->getConditions();
                if(conditions > 0)
                {
                    int cx = 5;
                    int cy = 5;
                    for(int i = 0; i < Combatant::getConditionCount(); ++i)
                    {
                        int condition = Combatant::getConditionByIndex(i);
                        if((conditions & condition) && (cy <= _combatantFrame.height() - 30))
                        {
                            QPixmap conditionPixmap(QString(":/img/data/img/") + Combatant::getConditionIcon(condition) + QString(".png"));
                            painter.drawPixmap(unrotatedBackgroundImageSize.width() + cx,
                                               yPos + cy,
                                               conditionPixmap.scaled(30,30));
                            cx += 30;
                            if(cx > _combatantFrame.width() - 30)
                            {
                                cx = 5;
                                cy += 30;
                            }
                        }
                    }
                }

                yPos += DMHelper::PixmapSizes[DMHelper::PixmapSize_Animate][1] + 10;
                nextCombatant = getNextCombatant(nextCombatant);
            }
        }
    }

    painter.end();

    imageForPublishing = drawingImageForPublishing;

#ifdef BATTLE_DIALOG_PROFILE_RENDER
    tAdditionalContent = tProfile.restart();
    #ifdef BATTLE_DIALOG_PROFILE_RENDER_TEXT
        qDebug() << "[Battle Frame][PROFILE] " << tProfile.restart() << "; additional contents drawn";
    #endif
#endif

#ifdef BATTLE_DIALOG_PROFILE_RENDER
    qDebug() << "[Battle Frame][PROFILE] " << tBasicPrep << " ; " << tVideoPrep << " ; " << tVideoRender << " ; " << tPrescaledPrep << " ; " << tPrescaledRender << " ; " << tContent << " ; " << tAdditionalContent;
#endif

#ifdef BATTLE_DIALOG_LOG_VIDEO
    qDebug() << "[Battle Frame] Image for publishing created.";
#endif
}

void BattleFrame::updatePublishEnable()
{
    emit setPublishEnabled((_model) && (_model->getMap()));
}

void BattleFrame::createVideoPlayer(bool dmPlayer)
{
#ifdef BATTLE_DIALOG_LOG_VIDEO
    qDebug() << "[Battle Frame] Creating video player. For DM: " << dmPlayer << ". Existing player: " << _videoPlayer;
#endif

    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to create video player, no battle model is set!";
        return;
    }

    if(!_model->getMap())
        return;

    if(_videoPlayer)
    {
        _videoPlayer->stopThenDelete();
        _videoPlayer = nullptr;
    }

    if(dmPlayer)
    {
        qDebug() << "[BattleDialog] Publish FoW DM animation started";
        _videoPlayer = new VideoPlayer(_model->getMap()->getFileName(), QSize(0, 0), true, false);
        if(_videoPlayer->isNewImage())
            updateVideoBackground();
        else
            connect(_videoPlayer, SIGNAL(screenShotAvailable()), this, SLOT(updateVideoBackground()));
    }
    else
    {
        qDebug() << "[BattleDialog] Publish FoW Player animation started";
        resetVideoSizes();
        _videoPlayer = new VideoPlayer(_model->getMap()->getFileName(), _videoSize, true, _model->getMap()->getPlayAudio());
    }

#ifdef BATTLE_DIALOG_LOG_VIDEO
    qDebug() << "[Battle Frame] Video player created.";
#endif

}

void BattleFrame::resetVideoSizes()
{
#ifdef BATTLE_DIALOG_LOG_VIDEO
    qDebug() << "[Battle Frame] Resetting video sizes";
#endif

    QRect viewportScene = _publishRectValue.isValid() ? getCameraRect().toRect() : ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect().toAlignedRect();
    QRect sceneRect = ui->graphicsView->sceneRect().toRect();
    QRect visibleSceneRect = viewportScene.intersected(sceneRect);

    qreal widthScale = static_cast<qreal>(sceneRect.width()) / static_cast<qreal>(visibleSceneRect.width());
    qreal heightScale = static_cast<qreal>(sceneRect.height()) / static_cast<qreal>(visibleSceneRect.height());

    QSize rotatedTargetBackground = getRotatedTargetBackgroundSize(visibleSceneRect.size());

    _videoSize = QSize(static_cast<int>(static_cast<qreal>(rotatedTargetBackground.width()) * widthScale),
                       static_cast<int>(static_cast<qreal>(rotatedTargetBackground.height()) * heightScale));

    _sourceRect = QRect(QPoint(static_cast<int>(static_cast<qreal>(visibleSceneRect.left()) * (static_cast<qreal>(_videoSize.width()) / static_cast<qreal>(sceneRect.width()))),
                               static_cast<int>(static_cast<qreal>(visibleSceneRect.top()) * (static_cast<qreal>(_videoSize.height()) / static_cast<qreal>(sceneRect.height())))),
                        rotatedTargetBackground);

    _bwFoWImage = QImage();

#ifdef BATTLE_DIALOG_LOG_VIDEO
    qDebug() << "[Battle Frame] Video sizes reset.";
#endif
}

void BattleFrame::clearBattleFrame()
{
    qDebug() << "[Battle Frame] Clearing Battle Frame.";

    if(_videoPlayer)
    {
        _videoPlayer->stopThenDelete();
        _videoPlayer = nullptr;
    }

    BattleDialogLogger* tempLogger = _logger;
    _logger = nullptr;
    delete tempLogger;

    removeRollover();

    // Clean up the list of combatant widgets
    QMapIterator<BattleDialogModelCombatant*, CombatantWidget*> i(_combatantWidgets);
    while(i.hasNext())
    {
        i.next();
        i.value()->deleteLater();
    }
    _combatantWidgets.clear();

    _contextMenuCombatant = nullptr;
    _mouseDown = false;
    _mouseDownPos = QPoint();

    _activeScale = 1.0;
    _selectedScale = 1.0;

    if(_countdownTimer)
        _countdownTimer->stop();

    _countdownColor = QColor(0, 0, 0);

    setMapCursor();
}

void BattleFrame::cleanupBattleMap()
{
    _mapDrawer->setMap(nullptr, nullptr);

    // Clean up the old map
    _scene->clearBattleContents();
    delete _background; _background = nullptr; _fow = nullptr;
    delete _activePixmap; _activePixmap = nullptr;
    delete _compassPixmap; _compassPixmap = nullptr;
    delete _publishRect; _publishRect = nullptr;
    delete _movementPixmap; _movementPixmap = nullptr;

    // Clean up any existing icons
    qDeleteAll(_combatantIcons.values());
    _combatantIcons.clear();
}

void BattleFrame::replaceBattleMap()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to replace battle map, no battle model is set!";
        return;
    }

    qDebug() << "[Battle Frame] New map detected";

    cleanupBattleMap();

    updatePublishEnable();

    if(!_model->getMap())
        return;

    _background = new UnselectedPixmap();
    _fow = new UnselectedPixmap();

    updateMap();

    _scene->addItem(_background);
    _fow->setParentItem(_background);
    _background->setEnabled(false);
    _background->setZValue(DMHelper::BattleDialog_Z_Background);

    if(!_videoPlayer)
        createSceneContents();

    qDebug() << "[Battle Frame] map set to new image (" << _model->getMap()->getFileName() << ")";
}

bool BattleFrame::doSceneContentsExist()
{
    return((_activePixmap != nullptr) ||
           (_compassPixmap != nullptr) ||
           (_movementPixmap != nullptr) ||
           (_combatantIcons.count() > 0) ||
           ((_scene) && (!_scene->isSceneEmpty())));
}

void BattleFrame::createSceneContents()
{
    qDebug() << "[Battle Frame] Creating Battle Scene contents.";

    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to create scene contents, no battle model is set!";
        return;
    }

    _scene->setSceneRect(_scene->itemsBoundingRect());
    ui->graphicsView->fitInView(_model->getMapRect(), Qt::KeepAspectRatio);

    _scene->createBattleContents(_background->boundingRect().toRect());

    createActiveIcon();
    createCombatantFrame();
    createCountdownFrame();

    QPixmap compassPmp;
    compassPmp.load(":/img/data/compass.png");
    _compassPixmap = _scene->addPixmap(compassPmp);
    _compassPixmap->setTransformationMode(Qt::SmoothTransformation);
    qreal scaleW = static_cast<qreal>(_background->pixmap().width()) / static_cast<qreal>(compassPmp.width());
    qreal scaleH = static_cast<qreal>(_background->pixmap().height()) / static_cast<qreal>(compassPmp.height());
    _compassPixmap->setScale(COMPASS_SCALE * qMin(scaleW, scaleH));
    _compassPixmap->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    _compassPixmap->setVisible(_model->getShowCompass());

    QPen movementPen(QColor(23,23,23,200), 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
    _movementPixmap = _scene->addEllipse(0, 0, 100, 100, movementPen, QBrush(QColor(255,255,255,25)));
    _movementPixmap->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    _movementPixmap->setVisible(false);

    setCameraRect(true);

    // Add icons for existing combatants
    for(int i = 0; i < _model->getCombatantCount(); ++i)
    {
        createCombatantIcon(_model->getCombatant(i));
    }

    updateHighlights();
}

void BattleFrame::resizeBattleMap()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to resize battle map, no battle model is set!";
        return;
    }

    if(!_model->getMap())
        return;

    qDebug() << "[Battle Frame] Update of same map detected";

    updatePublishEnable();

    updateMap();

    _scene->resizeBattleContents(_background->boundingRect().toRect());
    _scene->setSceneRect(_scene->itemsBoundingRect());
    ui->graphicsView->fitInView(_model->getMapRect(), Qt::KeepAspectRatio);

    // try to move the icons with the map
    QList<QGraphicsPixmapItem*> iconList = _combatantIcons.values();
    for(int i = 0; i < iconList.count(); ++i)
    {
        relocateCombatantIcon(iconList.at(i));
    }
}

// Returns the maximum background image width fitting into the given frame width
int BattleFrame::widthFrameToBackground(int frameWidth)
{
    return frameWidth - getFrameWidth();
}

// Returns the width of a frame fitting to the given background width
int BattleFrame::widthBackgroundToFrame(int backgroundWidth)
{
    return backgroundWidth + getFrameWidth();
}

// Returns the maximum size of background fitting into the given frame size
QSize BattleFrame::sizeFrameToBackground(const QSize& frameSize)
{
    return QSize(frameSize.width() - getFrameWidth(),
                 frameSize.height());
}

// Returns the size of frame required to fit the given background size
QSize BattleFrame::sizeBackgroundToFrame(const QSize& backgroundSize)
{
    return QSize(backgroundSize.width() + getFrameWidth(),
                 qMax(backgroundSize.height(), getFrameHeight()));
}

// Returns the required frame width based on the current settings
int BattleFrame::getFrameWidth()
{
    return (_showOnDeck ? _combatantFrame.width() : 0) +
           (_showCountdown ? _countdownFrame.width() : 0);
}

// Returns the required frame height based on the current settings
int BattleFrame::getFrameHeight()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to get frame height, no battle model is set!";
        return 0;
    }

    if(_model->getActiveCombatant())
    {
        if(_showOnDeck)
            return 2 * _combatantFrame.height();
        else if(_showCountdown)
            return _countdownFrame.height();
    }

    return 0;
}

// Returns the maximum size for the given background size to fit in the given frame, retaining its aspect ratio
QSize BattleFrame::getTargetBackgroundSize(const QSize& originalBackgroundSize, const QSize& targetSize)
{
    QSize maximumResult = sizeFrameToBackground(targetSize);
    return originalBackgroundSize.scaled(maximumResult, Qt::KeepAspectRatio);
}

// Returns the maximum size for the given background size to fit in the current target size, retaining its aspect ratio and considering the current rotation
QSize BattleFrame::getRotatedTargetBackgroundSize(const QSize& originalBackgroundSize)
{
    return getTargetBackgroundSize(originalBackgroundSize, (_rotation % 180 == 0) ? _targetSize : _targetSize.transposed());
}

QSize BattleFrame::getRotatedTargetFrameSize(const QSize& originalBackgroundSize)
{
    if((_rotation == 0) || (_rotation == 180))
    {
        return sizeBackgroundToFrame(originalBackgroundSize);
    }
    else
    {
        return sizeBackgroundToFrame(originalBackgroundSize.transposed()).transposed();
    }
}

QPoint BattleFrame::getPrescaledRenderPos(QSize targetSize)
{
    if(_rotation == 0)
        return QPoint(0,0);
    else if(_rotation == 90)
        return QPoint(targetSize.width() - _prescaledBackground.width(),0);
    else if(_rotation == 180)
        return QPoint(targetSize.width() - _prescaledBackground.width(), targetSize.height() - _prescaledBackground.height());
    else if(_rotation == 270)
        return QPoint(0,targetSize.height() - _prescaledBackground.height());

    qDebug() << "[Battle Frame] ERROR: unexpected rotation level found: " << _rotation;
    return QPoint(0,0);
}

bool BattleFrame::convertPublishToScene(const QPointF& publishPosition, QPointF& scenePosition)
{
    qreal publishWidth = 0.0;
    qreal publishX = 0.0;
    qreal publishY = 0.0;

    if(_rotation == 0)
    {
        publishWidth = _targetLabelSize.width();
        publishX = publishPosition.x();
        publishY = publishPosition.y();
    }
    else if(_rotation == 90)
    {
        publishWidth = _targetLabelSize.height();
        publishX = publishPosition.y();
        publishY = 1.0 - publishPosition.x();
    }
    else if(_rotation == 180)
    {
        publishWidth = _targetLabelSize.width();
        publishX = 1.0 - publishPosition.x();
        publishY = 1.0 - publishPosition.y();
    }
    else if(_rotation == 270)
    {
        publishWidth = _targetLabelSize.height();
        publishX = 1.0 - publishPosition.y();
        publishY = publishPosition.x();
    }

    if((publishWidth <= 0) || (publishWidth <= getFrameWidth()))
        return false;

    qreal maxPercent = static_cast<qreal>(publishWidth - getFrameWidth()) / static_cast<qreal>(publishWidth);
    if((maxPercent <= 0.0) || (publishX < 0.0) || (publishX > maxPercent) || (publishY < 0.0) || (publishY > 100.0))
        return false;

    scenePosition = QPointF((publishX * _publishRectValue.width() / maxPercent) + _publishRectValue.x(),
                            (publishY * _publishRectValue.height()) + _publishRectValue.y());

    return true;
}

void BattleFrame::setCameraRect(bool cameraOn)
{
    Q_UNUSED(cameraOn);

    if(!_scene)
        return;

    if(!_publishRect)
    {
        if((!_model) || (_model->getCameraRect().isValid()))
        {
            _publishRect = new CameraRect(_model->getCameraRect().width(), _model->getCameraRect().height(), *_scene, ui->graphicsView->viewport());
            _publishRect->setPos(_model->getCameraRect().x(),_model->getCameraRect().y());
        }
        else
        {
            _publishRect = new CameraRect(_scene->width(), _scene->height(), *_scene, ui->graphicsView->viewport());
            _publishRect->setPos(0,0);
        }
    }

    updateCameraRect();
}

void BattleFrame::updateCameraRect()
{
    if(_publishRect)
    {
        _publishRectValue = _publishRect->rect();
        _publishRectValue.moveTo(_publishRect->pos());
    }
    else
    {
        _publishRectValue = QRectF();
    }

    if(_model)
        _model->setCameraRect(_publishRectValue);
}

QRectF BattleFrame::getCameraRect()
{
    return _publishRectValue;
}

void BattleFrame::setCameraToView()
{
    if(!_publishRect)
        return;

    QRectF viewRect = ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect();
    viewRect.adjust(1.0, 1.0, -1.0, -1.0);
    _publishRect->setCameraRect(viewRect);
}

void BattleFrame::renderPrescaledBackground(QPainter& painter, QSize targetSize)
{
    if((_prescaledBackground.isNull()) || (!painter.device()))
        return;

    QPoint renderImagePosition = getPrescaledRenderPos(targetSize);

#ifdef BATTLE_DIALOG_PROFILE_RENDER
    tPrescaledPrep = tProfile.restart();
    #ifdef BATTLE_DIALOG_PROFILE_RENDER_TEXT
        qDebug() << "[Battle Frame][PROFILE] " << tProfile.restart() << "; rendering prepared (image background)";
    #endif
#endif

    // Draw the background image
    painter.drawPixmap(renderImagePosition, _prescaledBackground);
#ifdef BATTLE_DIALOG_PROFILE_RENDER
    tPrescaledRender = tProfile.restart();
    #ifdef BATTLE_DIALOG_PROFILE_RENDER_TEXT
        qDebug() << "[Battle Frame][PROFILE] " << tProfile.restart() << "; background drawn (image background)";
    #endif
#endif
}

void BattleFrame::renderVideoBackground(QPainter& painter)
{
#ifdef BATTLE_DIALOG_LOG_VIDEO
    qDebug() << "[Battle Frame] Rendering video background";
#endif

    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to render video background, no battle model is set!";
        return;
    }

    if((!_videoPlayer) || (!_videoPlayer->getImage()) || (_videoPlayer->getImage()->isNull()) || (_videoPlayer->isError()) || (!_videoPlayer->getMutex()) || (!_model->getMap()) || (!painter.device()))
        return;

    QMutexLocker locker(_videoPlayer->getMutex());

    // One-shot prepare the FoW image
    if((_bwFoWImage.isNull()) && (!_model->getMap()->isCleared()))
    {
        QSize originalSize = _videoPlayer->getOriginalSize();
        QSize imageSize = _videoPlayer->getImage()->size();
        if((!originalSize.isEmpty()) && (!imageSize.isEmpty()))
        {
            QImage bwImg = _model->getMap()->getBWFoWImage(originalSize).scaled(imageSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            _bwFoWImage = bwImg.copy(_sourceRect);
            _bwFoWImage.convertTo(QImage::Format_ARGB32_Premultiplied);

            if(_model->getGridOn())
            {
                QRect viewportRect = ui->graphicsView->viewport()->rect();
                QRect sceneViewportRect = ui->graphicsView->mapFromScene(ui->graphicsView->sceneRect()).boundingRect();
                QRect sourceRect = viewportRect.intersected(sceneViewportRect);

                setGridOnlyVisibility(true);
                QPainter painter;
                painter.begin(&_bwFoWImage);
                ui->graphicsView->render(&painter,
                                         QRectF(),
                                         sourceRect);
                painter.end();
                setGridOnlyVisibility(false);
            }
        }
    }

#ifdef BATTLE_DIALOG_PROFILE_RENDER
    tVideoPrep = tProfile.restart();
    #ifdef BATTLE_DIALOG_PROFILE_RENDER_TEXT
        qDebug() << "[Battle Frame][PROFILE] " << tProfile.restart() << "; rendering prepared (video background)";
    #endif
#endif

    painter.drawImage(QPoint(0,0), *_videoPlayer->getImage(), _sourceRect);
    if(!_bwFoWImage.isNull())
        painter.drawImage(QPoint(0,0), _bwFoWImage);

#ifdef BATTLE_DIALOG_PROFILE_RENDER
    tVideoRender = tProfile.restart();
    #ifdef BATTLE_DIALOG_PROFILE_RENDER_TEXT
        qDebug() << "[Battle Frame][PROFILE] " << tProfile.restart() << "; background drawn (video background)";
    #endif
#endif

#ifdef BATTLE_DIALOG_LOG_VIDEO
    qDebug() << "[Battle Frame] Video background rendered.";
#endif
}

bool BattleFrame::isItemInEffect(QGraphicsPixmapItem* item, QGraphicsItem* effect)
{
    if((!item) || (!effect))
        return false;

    QGraphicsItem* collisionEffect = effect;
    for(QGraphicsItem* childEffect : effect->childItems())
    {
        if(childEffect->data(BATTLE_DIALOG_MODEL_EFFECT_ROLE).toInt() == BattleDialogModelEffect::BattleDialogModelEffectRole_Area)
            collisionEffect = childEffect;
    }

    if(item->childItems().count() > 0)
    {
        for(QGraphicsItem* childItem : item->childItems())
        {
            if((childItem) && (childItem->data(BattleDialogItemChild_Index).toInt() == BattleDialogItemChild_Area))
                return childItem->collidesWithItem(collisionEffect);
        }
    }

    return item->collidesWithItem(collisionEffect);
}

void BattleFrame::removeEffectsFromItem(QGraphicsPixmapItem* item)
{
    if(!item)
        return;

#ifdef BATTLE_DIALOG_LOG_MOVEMENT
    qDebug() << "[Battle Frame] Removing effects from item " << item;
#endif

    // Remove any existing effects on this combatant
    for(QGraphicsItem* childItem : item->childItems())
    {
        if((childItem) && (childItem->data(BattleDialogItemChild_Index).toInt() == BattleDialogItemChild_AreaEffect))
        {
#ifdef BATTLE_DIALOG_LOG_MOVEMENT
            qDebug() << "[Battle Frame] Deleting child item: " << childItem;
#endif
            childItem->setParentItem(nullptr);
            delete childItem;
        }
    }
}

void BattleFrame::applyEffectToItem(QGraphicsPixmapItem* item, BattleDialogModelEffect* effect)
{
    if((!item) || (!effect))
        return;

    QColor ellipseColor = effect->getColor();
    if(!ellipseColor.isValid())
        return;

    QRect itemRect = item->boundingRect().toRect();
    int maxSize = qMax(itemRect.width(), itemRect.height());

    QGraphicsEllipseItem* effectItem = new QGraphicsEllipseItem(-maxSize/2, -maxSize/2, maxSize, maxSize);
    effectItem->setPen(QPen(ellipseColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    ellipseColor.setAlpha(128);
    effectItem->setBrush(QBrush(ellipseColor));
    effectItem->setFlag(QGraphicsItem::ItemNegativeZStacksBehindParent);
    effectItem->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    effectItem->setData(BattleDialogItemChild_Index, BattleDialogItemChild_AreaEffect);
    effectItem->setParentItem(item);

#ifdef BATTLE_DIALOG_LOG_MOVEMENT
    qDebug() << "[Battle Frame] applying effects to item. Item: " << item << ", effect: " << effect << " with effect item " << effectItem;
#endif
}

void BattleFrame::applyPersonalEffectToItem(QGraphicsPixmapItem* item)
{
    if(!item)
        return;

    // TODO: Add personal effects
}

void BattleFrame::startMovement(QGraphicsPixmapItem* item, int speed)
{
    if((!item) || (!_model))
        return;

    if((_movementPixmap) && (_model->getShowMovement()))
    {
        int speedSquares = 2 * (speed / 5) + 1;
        _moveRadius = _model->getGridScale() * speedSquares;
        _moveStart = item->pos();
        _movementPixmap->setPos(_moveStart);
        _movementPixmap->setRect(-_moveRadius/2.0, -_moveRadius/2.0, _moveRadius, _moveRadius);
        _movementPixmap->setVisible(true);
    }

    if((_mapDrawer) && (_model->getFowMovement()))
    {
        BattleDialogModelCombatant* combatant = _combatantIcons.key(item, nullptr);
        if((combatant) && (combatant->getCombatantType() == DMHelper::CombatantType_Character))
        {
            qreal gridSize = static_cast<qreal>(_model->getGridScale());
            // TODO: make the radius variable instead of 30'
            _mapDrawer->handleMouseDown(item->pos(), 6 * gridSize, DMHelper::BrushType_Circle, true, true);
        }
    }
}

void BattleFrame::updateMovement(QGraphicsPixmapItem* item)
{
    if((!item) || (!_model))
        return;

    if((_movementPixmap) && (_model->getShowMovement()))
    {
        QPointF combatantPos = item->pos();

        if(_moveRadius > _model->getGridScale())
        {
            QPointF diff = _moveStart - combatantPos;
            qreal delta = qSqrt((diff.x() * diff.x()) + (diff.y() * diff.y()));
            _moveRadius -= 2 * delta;
        }

        if(_moveRadius <= _model->getGridScale())
        {
            _moveRadius = _model->getGridScale();
            _movementPixmap->setRotation(0.0);
            _movementPixmap->setVisible(false);
        }
        else
        {
            _moveStart = combatantPos;
        }

        _movementPixmap->setPos(combatantPos);
        _movementPixmap->setRect(-_moveRadius/2.0, -_moveRadius/2.0, _moveRadius, _moveRadius);
    }

    if((_mapDrawer) && (_model->getFowMovement()))
        _mapDrawer->handleMouseMoved(item->pos());
}

void BattleFrame::endMovement()
{
    if(_movementPixmap)
    {
        _movementPixmap->setRotation(0.0);
        _movementPixmap->setVisible(false);
        //emit animationUpdated(_battle);
    }

    if((_mapDrawer) && (_model) && (_model->getFowMovement()))
        _mapDrawer->handleMouseUp(QPointF());
}

QPixmap BattleFrame::getPointerPixmap()
{
    if(!_pointerFile.isEmpty())
    {
        QPixmap result;
        if(result.load(_pointerFile))
            return result;
    }

    return QPixmap(":/img/data/arrow.png");
}

void BattleFrame::prepareStateMachine()
{
    /* BUG:
     * so this is the actual 3.
1 - select a player view area
2- turn edit mode on.
3 - click the FoW area clear button
4 - realize you still have edit mode on
5 - turn edit mode off.
    => in the top ribbon the only icon lighted up now is FoW Area
6 - try to clear fog of war inside player view
instead move the player view
*/

    _stateMachine.addState(new BattleFrameState(DMHelper::BattleFrameState_CombatantEdit, BattleFrameState::BattleFrameStateType_Base));

    BattleFrameState* zoomSelectState = new BattleFrameState(DMHelper::BattleFrameState_ZoomSelect, BattleFrameState::BattleFrameStateType_Transient, QPixmap(":/img/data/icon_zoomselectcursor.png"), 32, 32);
    connect(zoomSelectState, &BattleFrameState::stateChanged, this, &BattleFrame::zoomSelectToggled);
    connect(zoomSelectState, &BattleFrameState::stateChanged, this, &BattleFrame::setItemsInert);
    _stateMachine.addState(zoomSelectState);

    BattleFrameState* cameraSelectState = new BattleFrameState(DMHelper::BattleFrameState_CameraSelect, BattleFrameState::BattleFrameStateType_Transient, QPixmap(":/img/data/icon_cameraselectcursor.png"), 32, 32);
    connect(cameraSelectState, &BattleFrameState::stateChanged, this, &BattleFrame::cameraSelectToggled);
    connect(cameraSelectState, &BattleFrameState::stateChanged, this, &BattleFrame::setItemsInert);
    _stateMachine.addState(cameraSelectState);

    BattleFrameState* cameraEditState = new BattleFrameState(DMHelper::BattleFrameState_CameraEdit, BattleFrameState::BattleFrameStateType_Persistent);
    connect(cameraEditState, &BattleFrameState::stateChanged, this, &BattleFrame::cameraEditToggled);
    connect(cameraEditState, &BattleFrameState::stateChanged, this, &BattleFrame::setCameraSelectable);
    _stateMachine.addState(cameraEditState);

    BattleFrameState* distanceState = new BattleFrameState(DMHelper::BattleFrameState_Distance, BattleFrameState::BattleFrameStateType_Persistent, QPixmap(":/img/data/icon_distancecursor.png"), 32, 32);
    connect(distanceState, &BattleFrameState::stateChanged, this, &BattleFrame::distanceToggled);
    _stateMachine.addState(distanceState);

    BattleFrameState* pointerState = new BattleFrameState(DMHelper::BattleFrameState_Pointer, BattleFrameState::BattleFrameStateType_Persistent, getPointerPixmap(), 0, 0);
    connect(pointerState, &BattleFrameState::stateChanged, this, &BattleFrame::pointerToggled);
    connect(this, SIGNAL(pointerChanged(const QCursor&)), pointerState, SLOT(setCursor(const QCursor&)));
    connect(pointerState, SIGNAL(cursorChanged(const QCursor&)), this, SLOT(stateUpdated()));
    _stateMachine.addState(pointerState);

    BattleFrameState* fowSelectState = new BattleFrameState(DMHelper::BattleFrameState_FoWSelect, BattleFrameState::BattleFrameStateType_Transient, QPixmap(":/img/data/icon_selectcursor.png"), 32, 32);
    connect(fowSelectState, &BattleFrameState::stateChanged, this, &BattleFrame::foWSelectToggled);
    connect(fowSelectState, &BattleFrameState::stateChanged, this, &BattleFrame::setItemsInert);
    _stateMachine.addState(fowSelectState);

    BattleFrameState* fowEditState = new BattleFrameState(DMHelper::BattleFrameState_FoWEdit, BattleFrameState::BattleFrameStateType_Persistent);
    connect(fowEditState, &BattleFrameState::stateChanged, this, &BattleFrame::foWEditToggled);
    connect(fowEditState, SIGNAL(stateChanged(bool)), this, SLOT(setEditMode()));
    connect(_mapDrawer, SIGNAL(cursorChanged(const QCursor&)), fowEditState, SLOT(setCursor(const QCursor&)));
    connect(fowEditState, SIGNAL(cursorChanged(const QCursor&)), this, SLOT(stateUpdated()));
    _stateMachine.addState(fowEditState);

    _stateMachine.reset();
}
