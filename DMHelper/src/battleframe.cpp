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
#include "spellbook.h"
#include "spell.h"
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
#include "battledialogmodeleffectfactory.h"
#include "battledialogmodeleffectobject.h"
#include "battledialogeffectsettings.h"
#include "battledialoggraphicsscene.h"
#include "battlecombatantframe.h"
#include "dicerolldialogcombatants.h"
#include "itemselectdialog.h"
#include "videoplayer.h"
#include "videoplayerglscreenshot.h"
#include "camerarect.h"
#include "battleframemapdrawer.h"
#include "battleframestate.h"
#include "combatantrolloverframe.h"
#include "publishglbattleimagerenderer.h"
#include "publishglbattlevideorenderer.h"
#include "layerseditdialog.h"
#include "layertokens.h"
#include "layergrid.h"
#include "layerreference.h"
#include "selectitemdialog.h"
#include "selectcombatantdialog.h"
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
#include <QFileDialog>
#include <QtMath>
#include <QUuid>
#include <QPixmap>
#include <QScreen>
#include <QImageReader>

//#define BATTLE_DIALOG_PROFILE_RENDER
//#define BATTLE_DIALOG_PROFILE_RENDER_TEXT
//#define BATTLE_DIALOG_PROFILE_PRESCALED_BACKGROUND
//#define BATTLE_DIALOG_LOG_MOVEMENT

//#define BATTLE_DIALOG_LOG_VIDEO

/*
 *
 * setCameraRect
 * emit cameraRectChanged ==> renderer
 * changing the size/shape of the CameraRect causes handleItemChanged which calls updateCameraRect
 * updateCameraRect sets CameraRect
 *
 */

const qreal ACTIVE_PIXMAP_SIZE = 800.0;
const qreal COUNTDOWN_TIMER = 0.05;

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
    _selectedCombatant(nullptr),
    _contextMenuCombatant(nullptr),
    _mouseDown(false),
    _mouseDownPos(),
    _hoverFrame(nullptr),
    _publishMouseDown(false),
    _publishMouseDownPos(),
    _publishEffectItem(nullptr),
    _scene(nullptr),
    _activePixmap(nullptr),
    _activeScale(1.0),
    _selectedScale(1.0),
    _movementPixmap(nullptr),
    _cameraRect(nullptr),
    _publishRectValue(),
    _includeHeight(false),
    _pitchHeight(0.0),
    _countdownTimer(nullptr),
    _countdown(0.0),
    _isPublishing(false),
    _isVideo(false),
    _fowImage(),
    _bwFoWImage(),
    _combatantFrame(),
    _countdownFrame(),
    _targetSize(),
    _targetLabelSize(),
    _isGridLocked(false),
    _gridLockScale(0.0),
    _mapDrawer(nullptr),
    _renderer(nullptr),
    _initiativeType(DMHelper::InitiativeType_ImageName),
    _showCountdown(true),
    _countdownDuration(15),
    _countdownColor(0, 0, 0),
    _pointerFile(),
    _activeFile(),
    _rubberBandRect(),
    _scale(1.0),
    _rotation(0),
    _copyList(),
    _moveRadius(0.0),
    _moveStart()
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(0, 5);
    ui->splitter->setStretchFactor(1, 0);

    _scene = new BattleDialogGraphicsScene(this);
    ui->graphicsView->setScene(_scene);

    _combatantLayout = new QVBoxLayout;
    _combatantLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ui->scrollAreaWidgetContents->setLayout(_combatantLayout);

    ui->scrollArea->setAcceptDrops(true);
    ui->scrollArea->installEventFilter(this);

    ui->graphicsView->installEventFilter(this);

    _countdownTimer = new QTimer(this);
    _countdownTimer->setSingleShot(false);
    connect(_countdownTimer, SIGNAL(timeout()), this, SLOT(countdownTimerExpired()));

    _mapDrawer = new BattleFrameMapDrawer(this);
    connect(_mapDrawer, &BattleFrameMapDrawer::fowEdited, this, &BattleFrame::updateFowImage);
    connect(_mapDrawer, &BattleFrameMapDrawer::fowEdited, this, &BattleFrame::updateFowImage);

    connect(ui->graphicsView, SIGNAL(rubberBandChanged(QRect, QPointF, QPointF)), this, SLOT(handleRubberBandChanged(QRect, QPointF, QPointF)));

    connect(ui->btnSort, SIGNAL(clicked()), this, SLOT(sort()));
    connect(ui->btnTop, SIGNAL(clicked()), this, SLOT(top()));
    connect(ui->btnNext, SIGNAL(clicked()), this, SLOT(next()));

    connect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
    connect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));

    connect(_scene, SIGNAL(mapZoom(int)), this, SLOT(zoomDelta(int)));
    connect(_scene, &BattleDialogGraphicsScene::addEffectRadius, this, &BattleFrame::addEffectRadius);
    connect(_scene, &BattleDialogGraphicsScene::addEffectCone, this, &BattleFrame::addEffectCone);
    connect(_scene, &BattleDialogGraphicsScene::addEffectCube, this, &BattleFrame::addEffectCube);
    connect(_scene, &BattleDialogGraphicsScene::addEffectLine, this, &BattleFrame::addEffectLine);
    connect(_scene, &BattleDialogGraphicsScene::addEffectObject, this, &BattleFrame::addEffectObject);
    connect(_scene, &BattleDialogGraphicsScene::castSpell, this, &BattleFrame::castSpell);
    connect(_scene, SIGNAL(effectChanged(QGraphicsItem*)), this, SLOT(handleEffectChanged(QGraphicsItem*)));
    connect(_scene, SIGNAL(effectRemoved(QGraphicsItem*)), this, SLOT(handleEffectRemoved(QGraphicsItem*)));
    connect(_scene, SIGNAL(applyEffect(QGraphicsItem*)), this, SLOT(handleApplyEffect(QGraphicsItem*)));
    connect(_scene, SIGNAL(distanceChanged(const QString&)), this, SIGNAL(distanceChanged(const QString&)));
    connect(_scene, SIGNAL(combatantHover(BattleDialogModelCombatant*, bool)), this, SLOT(handleCombatantHover(BattleDialogModelCombatant*, bool)));
    connect(_scene, SIGNAL(combatantActivate(BattleDialogModelCombatant*)), this, SLOT(handleCombatantActivate(BattleDialogModelCombatant*)));
    connect(_scene, SIGNAL(combatantRemove(BattleDialogModelCombatant*)), this, SLOT(handleCombatantRemove(BattleDialogModelCombatant*)));
    connect(_scene, SIGNAL(combatantChangeLayer(BattleDialogModelCombatant*)), this, SLOT(handleCombatantChangeLayer(BattleDialogModelCombatant*)));
    connect(_scene, SIGNAL(combatantDamage(BattleDialogModelCombatant*)), this, SLOT(handleCombatantDamage(BattleDialogModelCombatant*)));
    connect(_scene, SIGNAL(combatantHeal(BattleDialogModelCombatant*)), this, SLOT(handleCombatantHeal(BattleDialogModelCombatant*)));
    connect(_scene, SIGNAL(itemLink(BattleDialogModelObject*)), this, SLOT(handleItemLink(BattleDialogModelObject*)));
    connect(_scene, SIGNAL(itemUnlink(BattleDialogModelObject*)), this, SLOT(handleItemUnlink(BattleDialogModelObject*)));
    connect(_scene, SIGNAL(itemChanged(QGraphicsItem*)), this, SLOT(handleItemChanged(QGraphicsItem*)));
    connect(_scene, &BattleDialogGraphicsScene::mapMousePress, this, &BattleFrame::handleMapMousePress);
    connect(_scene, &BattleDialogGraphicsScene::mapMouseMove, this, &BattleFrame::handleMapMouseMove);
    connect(_scene, &BattleDialogGraphicsScene::mapMouseRelease, this, &BattleFrame::handleMapMouseRelease);
    connect(_scene, &BattleDialogGraphicsScene::changed, this, &BattleFrame::handleSceneChanged);

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

void BattleFrame::activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer)
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

    rendererActivated(dynamic_cast<PublishGLBattleRenderer*>(currentRenderer));

    _isPublishing = (currentRenderer) && (_battle) && (currentRenderer->getObject() == _battle->getBattleDialogModel());
    if(_cameraRect)
        _cameraRect->setPublishing(_isPublishing);

    emit checkableChanged(true);
}

void BattleFrame::deactivateObject()
{
    if(!_battle)
    {
        qDebug() << "[BattleFrame] WARNING: Invalid (nullptr) battle object deactivated!";
        return;
    }

    rendererDeactivated();
    cancelSelect();

    ui->frameCombatant->setCombatant(nullptr);
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

    // TODO: Layers - do we need to resize a map now?
//    if(_model->isMapChanged())
//    {
        disconnect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
        disconnect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
        replaceBattleMap();
        connect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
        connect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
/*    }
    else
    {
        resizeBattleMap();
    }
    *
    */
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

/*
void BattleFrame::recenterCombatants()
{
    if(!_model)
        return;

    QPointF mapCenter = _model->getLayerScene().boundingRect().center();

    // try to center the icons on the map
    QList<QGraphicsPixmapItem*> iconList = _combatantIcons.values();
    for(int i = 0; i < iconList.count(); ++i)
    {
        QGraphicsPixmapItem* icon = iconList.at(i);
        if(icon)
            icon->setPos(mapCenter);
    }
}
*/

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

void BattleFrame::top()
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to go to the first combatant, no battle model is set!";
        return;
    }

    BattleDialogModelCombatant* nextCombatant = getFirstLivingCombatant();
    setActiveCombatant(nextCombatant);
    qDebug() << "[Battle Frame] Activated first combatant: " << nextCombatant;
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
    if((!_battle) || (!_isPublishing))
        return;

    QPointF newPosition;
    if(!convertPublishToScene(position, newPosition))
        return;

    QList<QGraphicsItem *> itemList = _scene->items(newPosition);
    for(QGraphicsItem* graphicsItem : itemList)
    {
        if((graphicsItem->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable)
        {
            QGraphicsPixmapItem* pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(graphicsItem);
            if(pixmapItem)
            {
                BattleDialogModelCombatant* selectedCombatant = _combatantIcons.key(pixmapItem, nullptr);
                if(selectedCombatant)
                {
                    setUniqueSelection(selectedCombatant);
                    _selectedCombatant = selectedCombatant;
                    _publishMouseDown = true;
                    _publishMouseDownPos = newPosition;
                    startMovement(selectedCombatant, pixmapItem, selectedCombatant->getSpeed());
                    return;
                }
            }

            if(!BattleDialogModelEffect::getEffectIdFromItem(graphicsItem).isNull())
            {
                cancelSelect();
                _publishEffectItem = graphicsItem;
                _publishMouseDown = true;
                _publishMouseDownPos = newPosition;
                handleEffectChanged(graphicsItem);
            }
        }
    }
}

void BattleFrame::publishWindowMouseMove(const QPointF& position)
{
    if((!_battle) || (!_publishMouseDown))
        return;

    if(!_isPublishing)
    {
        publishWindowMouseRelease(position);
        return;
    }

    QPointF newPosition;
    if(!convertPublishToScene(position, newPosition))
        return;

    if(newPosition == _publishMouseDownPos)
        return;

    if(_selectedCombatant)
    {
        QGraphicsPixmapItem* pixmapItem = _combatantIcons.value(_selectedCombatant);
        pixmapItem->setPos(newPosition);
        updateMovement(_selectedCombatant, pixmapItem);
    }
    else if(_publishEffectItem)
    {
        BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(_publishEffectItem);
        if(effect)
        {
            _publishEffectItem->setPos(newPosition);
            effect->setPosition(_publishEffectItem->pos());
            handleEffectChanged(_publishEffectItem);
        }
    }
}

void BattleFrame::publishWindowMouseRelease(const QPointF& position)
{
    Q_UNUSED(position);

    if(!_battle)
        return;

    endMovement();
    _selectedCombatant = nullptr;
    _publishEffectItem = nullptr;
    _publishMouseDown = false;
}

void BattleFrame::setGridScale(int gridScale)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set the grid scale, no battle model is set!";
        return;
    }

    _scene->setDistanceScale(gridScale);
    // TODO: Layers
    //_model->setGridScale(gridScale);
    _model->getLayerScene().setScale(gridScale);

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
            scaleFactor = (static_cast<qreal>(gridScale-2)) * combatantScaleFactor / static_cast<qreal>(qMax(i.value()->pixmap().width(), i.value()->pixmap().height()));
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
    //updateRendererGrid();
}

void BattleFrame::selectGridCount()
{
    if(!_model)
        return;

    bool ok = false;
    int gridCount = QInputDialog::getInt(this, QString("Get Grid Count"), QString("How many grid squares should the map have horizontally?"), DMHelper::DEFAULT_GRID_COUNT, 1, 100000, 1, &ok);
    if((ok) && (gridCount > 0))
    {
        int newGridScale = _model->getLayerScene().sceneSize().width() / gridCount;
        if(newGridScale > 0)
            setGridScale(newGridScale);
    }
}

void BattleFrame::setGridAngle(int gridAngle)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set the grid angle, no battle model is set!";
        return;
    }

    // TODO: Layers
    //_model->setGridAngle(gridAngle);
    _scene->updateBattleContents();
    ui->graphicsView->update();
    //updateRendererGrid();
}

void BattleFrame::setGridType(int gridType)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set the grid type, no battle model is set!";
        return;
    }

    // TODO: Layers
    //_model->setGridType(gridType);
    _scene->updateBattleContents();
    ui->graphicsView->update();
    //updateRendererGrid();
}

void BattleFrame::setXOffset(int xOffset)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set the X offset, no battle model is set!";
        return;
    }

    // TODO: Layers
    //_model->setGridOffsetX(xOffset);
    _scene->updateBattleContents();
    ui->graphicsView->update();
    //updateRendererGrid();
}

void BattleFrame::setYOffset(int yOffset)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set the Y offset, no battle model is set!";
        return;
    }

    // TODO: Layers
    //_model->setGridOffsetY(yOffset);
    _scene->updateBattleContents();
    ui->graphicsView->update();
    //updateRendererGrid();
}

void BattleFrame::setGridWidth(int gridWidth)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set the grid width, no battle model is set!";
        return;
    }

    // TODO: Layers
    //_model->setGridWidth(gridWidth);
    _scene->updateBattleContents();
    ui->graphicsView->update();
    //updateRendererGrid();
}

void BattleFrame::setGridColor(const QColor& gridColor)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set the grid color, no battle model is set!";
        return;
    }

    // TODO: Layers
    //_model->setGridColor(gridColor);
    _scene->updateBattleContents();
    ui->graphicsView->update();
    //updateRendererGrid();
}

void BattleFrame::setGridVisible(bool gridVisible)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to turn the grid on or off, no battle model is set!";
        return;
    }

    // TODO: Layers
    /*
    if((_scene) && (gridVisible != _model->getGridOn()))
    {
        // TODO: Layers
        //_model->setGridOn(gridVisible);
        _scene->setGridVisibility(gridVisible);
        ui->graphicsView->invalidateScene();
        updateRendererGrid();
    }
    */
}

void BattleFrame::setGridLocked(bool gridLocked)
{
    _isGridLocked = gridLocked;
    if(_cameraRect)
        _cameraRect->setRatioLocked(_isGridLocked);
}

void BattleFrame::setGridLockScale(qreal gridLockScale)
{
    _gridLockScale = gridLockScale;
}

void BattleFrame::setInitiativeType(int initiativeType)
{
    _initiativeType = initiativeType;
    if(_renderer)
        _renderer->setInitiativeType(_initiativeType);
}

void BattleFrame::setShowCountdown(bool showCountdown)
{
    _showCountdown = showCountdown;
    if(_renderer)
        _renderer->setShowCountdown(_showCountdown);
}

void BattleFrame::setCountdownDuration(int countdownDuration)
{
    _countdownDuration = countdownDuration;
}

void BattleFrame::setPointerFile(const QString& filename)
{
    if(_pointerFile != filename)
    {
        _pointerFile = filename;
        emit pointerFileNameChanged(_pointerFile);

        QPixmap pointerPixmap = getPointerPixmap();
        _scene->setPointerPixmap(pointerPixmap);
        emit pointerChanged(QCursor(pointerPixmap.scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), 0, 0));
    }
}

void BattleFrame::setSelectedIcon(const QString& selectedIcon)
{
    _scene->setSelectedIcon(selectedIcon);

    if(_renderer)
        _renderer->setSelectionToken(selectedIcon);
}

void BattleFrame::setActiveIcon(const QString& activeIcon)
{
    if(_activeFile != activeIcon)
    {
        _activeFile = activeIcon;
        createActiveIcon();

        if(_renderer)
            _renderer->setActiveToken(activeIcon);
    }
}

void BattleFrame::createActiveIcon()
{
    if(!_model)
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
        if(_renderer)
            _renderer->setCombatantFrame(_combatantFile);
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
        if(_renderer)
            _renderer->setCountdownFrame(_countdownFile);
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
    setScale(_scale * 1.1);
}

void BattleFrame::zoomOut()
{
    setScale(_scale * 0.9);
}

void BattleFrame::zoomFit()
{
    if((!ui->graphicsView->viewport()) || (!_scene))
        return;

    qreal widthFactor = static_cast<qreal>(ui->graphicsView->viewport()->width()) / _scene->width();
    qreal heightFactor = static_cast<qreal>(ui->graphicsView->viewport()->height()) / _scene->height();
    setScale(qMin(widthFactor, heightFactor));
}

void BattleFrame::zoomSelect(bool enabled)
{
    Q_UNUSED(enabled);

    _stateMachine.toggleState(DMHelper::BattleFrameState_ZoomSelect);
}

void BattleFrame::zoomDelta(int delta)
{
    if(delta > 0)
        zoomIn();
    else if(delta < 0)
        zoomOut();
}

void BattleFrame::cancelSelect()
{
    qDebug() << "[BattleFrame] cancelSelect";
    _stateMachine.deactivateState();
}

bool BattleFrame::createNewBattle()
{
    if((!_battle) || (!_battle->getBattleDialogModel()))
        return false;

    Campaign* campaign = dynamic_cast<Campaign*>(_battle->getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return false;

    LayerGrid* gridLayer = nullptr;
    LayerTokens* monsterTokens = nullptr;
    LayerTokens* pcTokens = nullptr;

    qDebug() << "[Battle Frame] Selecting a new map...";

    Map* battleMap = selectRelatedMap();
    if(battleMap)
    {
        battleMap->initialize();

        // Create a grid after the first image layer, a monster token layer before the FoW
        for(int i = 0; i < battleMap->getLayerScene().layerCount(); ++i)
        {
            Layer* layer = battleMap->getLayerScene().layerAt(i);
            if(layer)
            {
                if((!monsterTokens) && (layer->getFinalType() == DMHelper::LayerType_Fow))
                {
                    monsterTokens = new LayerTokens(_battle->getBattleDialogModel(), QString("Monster tokens"));
                    _battle->getBattleDialogModel()->getLayerScene().appendLayer(monsterTokens);
                }

                _battle->getBattleDialogModel()->getLayerScene().appendLayer(new LayerReference(battleMap, layer, layer->getOrder()));

                if((!gridLayer) && ((layer->getFinalType() == DMHelper::LayerType_Image) || (layer->getFinalType() == DMHelper::LayerType_Video)))
                {
                     gridLayer = new LayerGrid(QString("Grid"));
                     _battle->getBattleDialogModel()->getLayerScene().appendLayer(gridLayer);
                }
            }
        }
    }

    if(!gridLayer)
        _battle->getBattleDialogModel()->getLayerScene().appendLayer(new LayerGrid(QString("Grid")));

    if(!monsterTokens)
    {
        monsterTokens = new LayerTokens(_battle->getBattleDialogModel(), QString("Monster tokens"));
        _battle->getBattleDialogModel()->getLayerScene().appendLayer(monsterTokens);
    }

    pcTokens = new LayerTokens(_battle->getBattleDialogModel(), QString("PC tokens"));
    _battle->getBattleDialogModel()->getLayerScene().appendLayer(pcTokens);

    // Add the active characters
    _battle->getBattleDialogModel()->getLayerScene().setSelectedLayer(pcTokens);
    QList<Character*> activeCharacters = campaign->getActiveCharacters();
    for(int i = 0; i < activeCharacters.count(); ++i)
    {
        BattleDialogModelCharacter* newCharacter = new BattleDialogModelCharacter(activeCharacters.at(i));
        newCharacter->setPosition(QPointF(0.0, 0.0));
        _battle->getBattleDialogModel()->appendCombatant(newCharacter);
    }

    // Select the monster layer as a default to add monsters
    _battle->getBattleDialogModel()->getLayerScene().setSelectedLayer(monsterTokens);

    return battleMap != nullptr;

    // Todo: Layers - maybe need more about the zoom rect
    /*
    QImage fowImage = battleMap->getGrayImage();

    SelectZoom zoomDlg(fowImage, this);
    zoomDlg.exec();
    if(zoomDlg.result() == QDialog::Accepted)
    {
        qDebug() << "[Battle Frame] Setting new map to: " << battleMap->getID() << " " << battleMap->getName();
        _battle->getBattleDialogModel()->setMap(battleMap, zoomDlg.getZoomRect());
        setBattleMap();
        return true;
    }

    return false;
    */
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
        int localInitiative = combatantDlg.getInitiative().toInt();

        qreal sizeFactor = 0.0;
        bool conversionResult = false;
        sizeFactor = combatantDlg.getSizeFactor().toDouble(&conversionResult);
        if(!conversionResult)
            sizeFactor = 0.0;

        qDebug() << "[Battle Dialog Manager] ... adding " << monsterCount << " monsters of name " << baseName;

        for(int i = 0; i < monsterCount; ++i)
        {
            BattleDialogModelMonsterClass* monster = new BattleDialogModelMonsterClass(monsterClass);
            monster->setMonsterName((monsterCount == 1) ? baseName : (baseName + QString("#") + QString::number(i+1)));
            monster->setHitPoints(combatantDlg.getCombatantHitPoints());
            monster->setInitiative(combatantDlg.isRandomInitiative() ? Dice::d20() + Combatant::getAbilityMod(monsterClass->getDexterity()) : localInitiative);
            monster->setKnown(combatantDlg.isKnown());
            monster->setShown(combatantDlg.isShown());
            monster->setSizeFactor(sizeFactor);
            monster->setPosition(combatantPos);
            monster->setIconIndex(combatantDlg.getIconIndex());
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

void BattleFrame::addEffectObject()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, QString("Select object image file.."));
    if((filename.isEmpty()) || (!QImageReader(filename).canRead()))
        return;

    registerEffect(createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Object, 20, 20, QColor(), filename));
}

void BattleFrame::castSpell()
{
    if(!_model)
        return;

    bool ok = false;
    QString selectedSpell = QInputDialog::getItem(nullptr,
                                                  QString("Select Spell"),
                                                  QString("Select the spell to cast:"),
                                                  Spellbook::Instance()->getSpellList(),
                                                  0,
                                                  false,
                                                  &ok);
    if(!ok)
    {
        qDebug() << "[BattleFrame] Spell cast aborted: spell selection dialog cancelled.";
        return;
    }

    qDebug() << "[BattleFrame] Casting spell: " << selectedSpell;

    Spell* spell = Spellbook::Instance()->getSpell(selectedSpell);
    if(!spell)
    {
        qDebug() << "[BattleFrame] Spell cast aborted: not able to find selected spell in the Spellbook.";
        return;
    }

    BattleDialogModelEffect* effect = createEffect(spell->getEffectType(),
                                                   spell->getEffectSize().height(),
                                                   spell->getEffectSize().width(),
                                                   spell->getEffectColor(),
                                                   spell->getEffectTokenPath());

    if(!effect)
    {
        qDebug() << "[BattleFrame] Spell cast aborted: unable to create the effect object.";
        return;
    }

    if((spell->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Object) || (spell->getEffectToken().isEmpty()))
    {
        // Either an Object or a basic shape without a token
        effect->setName(spell->getName());
        effect->setTip(spell->getName());
        // TODO: Layers - move to Frame
        //addEffect(effect);
    }
    else
    {
        // A basic shape with a token image as well
        int tokenHeight = spell->getEffectSize().height();
        int tokenWidth = spell->getEffectSize().height();
        if(spell->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Radius)
        {
            tokenHeight *= 2;
            tokenWidth *= 2;
        }
        else if(spell->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Line)
        {
            tokenWidth = spell->getEffectSize().width();
        }

        BattleDialogModelEffectObject* tokenEffect =  dynamic_cast<BattleDialogModelEffectObject*>(createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Object,
                                                                                                                tokenHeight,
                                                                                                                tokenWidth,
                                                                                                                Qt::black, //spell->getEffectColor(),
                                                                                                                spell->getEffectTokenPath()));

        if(!tokenEffect)
        {
            qDebug() << "[BattleFrame] Spell cast aborted: unable to create the effect's token object!";
            delete effect;
            return;
        }

        tokenEffect->setName(spell->getName());
        tokenEffect->setTip(spell->getName());
        tokenEffect->setEffectActive(true);
        tokenEffect->setImageRotation(spell->getEffectTokenRotation());

        effect->addObject(tokenEffect);
        _model->appendEffect(effect);
        // TODO: Layers - move to Frame
        //addSpellEffect(*effect);
    }
}

void BattleFrame::addEffectRadius()
{
    registerEffect(createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Radius, 20, 0, QColor(115, 18, 0, 64), QString()));
}

void BattleFrame::addEffectCone()
{
    registerEffect(createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Cone, 20, 0, QColor(115, 18, 0, 64), QString()));
}

void BattleFrame::addEffectCube()
{
    registerEffect(createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Cube, 20, 0, QColor(115, 18, 0, 64), QString()));
}

void BattleFrame::addEffectLine()
{
    registerEffect(createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Line, 20, 5, QColor(115, 18, 0, 64), QString()));
}

void BattleFrame::registerEffect(BattleDialogModelEffect* effect)
{
    if(!effect)
        return;

    if(!_model)
    {
        delete effect;
        return;
    }

    BattleDialogEffectSettings* settings = effect->getEffectEditor();
    if(!settings)
    {
        delete effect;
        return;
    }

    _model->appendEffect(effect);

    settings->exec();
    if(settings->result() == QDialog::Accepted)
        settings->copyValues(*effect);

    settings->deleteLater();
}

void BattleFrame::setCameraCouple()
{
    setCameraToView();
}

void BattleFrame::setCameraMap()
{
    if(!_cameraRect)
        return;

    QRectF newCameraRect;
    if(_isGridLocked)
    {
        //QRectF currentRect(QPointF(0.0, 0.0), QSizeF(_targetSize).scaled(_scene->sceneRect().size(), Qt::KeepAspectRatioByExpanding));
        QRectF currentRect = _cameraRect->getCameraRect();
        QPointF newCenter = _scene->sceneRect().center();
        currentRect.moveTo(newCenter.x() - (currentRect.width() / 2.0), newCenter.y() - (currentRect.height() / 2.0));
        newCameraRect = currentRect;
    }
    else
    {
        newCameraRect = _scene->sceneRect();
    }

    _cameraRect->setCameraRect(newCameraRect);
    emit cameraRectChanged(newCameraRect);
}

void BattleFrame::setCameraVisible()
{
    if((!_cameraRect) || (!_model) || (!_model->getMap()))
        return;

    QRectF newCameraRect = _model->getMap()->getShrunkPublishRect();
    if(_isGridLocked)
    {
        //QRectF currentRect(QPointF(0.0, 0.0), QSizeF(_targetSize).scaled(newCameraRect.size(), Qt::KeepAspectRatioByExpanding));
        QRectF currentRect = _cameraRect->getCameraRect();
        QPointF newCenter = newCameraRect.center();
        currentRect.moveTo(newCenter.x() - (currentRect.width() / 2.0), newCenter.y() - (currentRect.height() / 2.0));
        newCameraRect = currentRect;
    }
    _cameraRect->setCameraRect(newCameraRect);
    emit cameraRectChanged(newCameraRect);
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

void BattleFrame::setFreeDistance(bool enabled)
{
    Q_UNUSED(enabled);
    _stateMachine.toggleState(DMHelper::BattleFrameState_FreeDistance);
}

void BattleFrame::setDistanceHeight(bool heightEnabled, qreal height)
{
    _scene->setDistanceHeight(heightEnabled ? height : 0.0);
}

void BattleFrame::setDistanceScale(int scale)
{
    _scene->setDistanceScale(scale);
}

void BattleFrame::setDistanceLineColor(const QColor& color)
{
    _scene->setDistanceLineColor(color);
}

void BattleFrame::setDistanceLineType(int lineType)
{
    _scene->setDistanceLineType(lineType);
}

void BattleFrame::setDistanceLineWidth(int lineWidth)
{
    _scene->setDistanceLineWidth(lineWidth);
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
    if(e->key() == Qt::Key_Space)
    {
        next();
        return;
    }
    else if(e->key() == Qt::Key_Escape)
    {
        cancelSelect();
        return;
    }
    else if((e->key() == Qt::Key_C) && (e->modifiers() == Qt::ControlModifier))
    {
        copyMonsters();
    }
    else if((e->key() == Qt::Key_V) && (e->modifiers() == Qt::ControlModifier))
    {
        pasteMonsters();
    }

    QFrame::keyPressEvent(e);
}

bool BattleFrame::eventFilter(QObject *obj, QEvent *event)
{
    if(!event)
        return false;

    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if((keyEvent) && (keyEvent->modifiers() == Qt::AltModifier))
        {
            if(keyEvent->key() == Qt::Key_Left)
                emit navigateBackwards();
            else if(keyEvent->key() == Qt::Key_Right)
                emit navigateForwards();
            return true;
        }
    }

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
                qDebug() << "[Battle Frame] combatant widget mouse down " << _mouseDownPos.x() << ", " << _mouseDownPos.y();
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
                                qDebug() << "[Battle Frame] starting combatant widget drag: index " << index << ": " << combatant->getName() << ", (" << reinterpret_cast<quint64>(widget) << ") " << mouseEvent->pos().x() << ", " << mouseEvent->pos().y();
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
                if((!_mouseDown) && (_combatantLayout) && (widget->getCombatant()) &&
                   ((widget->getCombatant()->getCombatantType() == DMHelper::CombatantType_Monster) || (widget->getCombatant()->getCombatantType() == DMHelper::CombatantType_Character)))
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
                    qDebug() << "[Battle Frame] combatant widget drag dropped (" << dropEvent->pos().x() << ", " << dropEvent->pos().y() << ")";

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
                            BattleDialogModelCombatant* combatant = _model->getCombatant(index);
                            _model->moveCombatant(index, currentIndex);
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

    return CampaignObjectFrame::eventFilter(obj, event);
}

void BattleFrame::resizeEvent(QResizeEvent *event)
{
    qDebug() << "[Battle Frame] resized: " << event->size().width() << "x" << event->size().height();
    if(_model)
        ui->graphicsView->fitInView(_model->getMapRect(), Qt::KeepAspectRatio);

    QFrame::resizeEvent(event);
}

void BattleFrame::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    qDebug() << "[Battle Frame]shown (" << isVisible() << ")";
    if(_model)
        ui->graphicsView->fitInView(_model->getMapRect(), Qt::KeepAspectRatio);

    QScreen* primary = QGuiApplication::primaryScreen();
    if(!primary)
        return;

    if(_targetSize.isEmpty())
        _targetSize = primary->availableSize() * primary->devicePixelRatio();

    if(_targetLabelSize.isEmpty())
        _targetLabelSize = primary->availableSize() * primary->devicePixelRatio();

    int ribbonHeight = primary->availableSize().height() / 15;
    QFontMetrics metrics = ui->lblNext->fontMetrics();

    int labelHeight = metrics.height() + (ribbonHeight / 10);
    int iconDim = ribbonHeight - labelHeight;
    int newWidth = qMax(metrics.horizontalAdvance(ui->lblCountdown->text()), iconDim);

    ui->lblNext->setMinimumWidth(newWidth);
    ui->lblNext->setMaximumWidth(newWidth);
    ui->lblNext->setMinimumHeight(labelHeight);
    ui->lblNext->setMaximumHeight(labelHeight);

    ui->lblTop->setMinimumWidth(newWidth);
    ui->lblTop->setMaximumWidth(newWidth);
    ui->lblTop->setMinimumHeight(labelHeight);
    ui->lblTop->setMaximumHeight(labelHeight);

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

    ui->btnTop->setMinimumWidth(newWidth);
    ui->btnTop->setMaximumWidth(newWidth);
    ui->btnTop->setMinimumHeight(iconDim);
    ui->btnTop->setMaximumHeight(iconDim);

    ui->btnSort->setMinimumWidth(newWidth);
    ui->btnSort->setMaximumWidth(newWidth);
    ui->btnSort->setMinimumHeight(iconDim);
    ui->btnSort->setMaximumHeight(iconDim);

    int iconSize = qMin(newWidth, iconDim) * 4 / 5;
    ui->btnNext->setIconSize(QSize(iconSize, iconSize));
    ui->btnTop->setIconSize(QSize(iconSize, iconSize));
    ui->btnSort->setIconSize(QSize(iconSize, iconSize));
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

    // TODO: Layers - is this right?
    /*
    if((!_model->getMap()) || (!_mapDrawer))
    {
        qDebug() << "[Battle Frame] No map found to be updated for the current battle model";
        return;
    }

    qDebug() << "[Battle Frame] Updating map " << _model->getMap()->getFileName() << " rect=" << _model->getMapRect().left() << ", " << _model->getMapRect().top() << ", " << _model->getMapRect().width() << "x" << _model->getMapRect().height();
    */
    _isVideo = false;
    //if(_model->getMap()->initialize())
    {
        // TODO: Layers - is this right?
        _model->getLayerScene().initializeLayers();
        //if(!_model->getMap()->isInitialized())
        //    return;

        qDebug() << "[Battle Frame] Initializing battle map image";
        _model->getLayerScene().dmInitialize(_scene);
        // TODO: Layers
        //if(_model->getBackgroundImage().isNull())
        //    _model->setBackgroundImage(_model->getMap()->getBackgroundImage());
        //_background->setPixmap((QPixmap::fromImage(_model->getBackgroundImage())));
        //_fowImage = QPixmap::fromImage(_model->getMap()->getFoWImage());
        // TODO: Layers
        //_bwFoWImage = _model->getMap()->getBWFoWImage();
        //_mapDrawer->setMap(_model->getMap(), &_fowImage, &_bwFoWImage);
    }
    // TODO: Layers - video support
    /*
    else if(_model->getMap()->isValid())
    {
        qDebug() << "[Battle Frame] Initializing battle map video";
        extractDMScreenshot();
        _isVideo = true;
    }
    */
}

void BattleFrame::updateRounds()
{
    if(_logger)
        ui->edtRounds->setText(QString::number(_logger->getRounds()));
}

void BattleFrame::handleContextMenu(BattleDialogModelCombatant* combatant, const QPoint& position)
{
    if(!combatant)
        return;

    qDebug() << "[Battle Frame] context menu opened for " << combatant->getName() << " at " << position.x() << "x" << position.y();

    _contextMenuCombatant = combatant;

    QMenu* contextMenu = new QMenu(ui->scrollArea);

    QAction* activateItem = new QAction(QString("Activate"), contextMenu);
    connect(activateItem, SIGNAL(triggered()), this, SLOT(activateCombatant()));
    contextMenu->addAction(activateItem);

    QAction* removeItem = new QAction(QString("Remove"), contextMenu);
    connect(removeItem, SIGNAL(triggered()), this, SLOT(removeCombatant()));
    contextMenu->addAction(removeItem);

    if(_contextMenuCombatant->getLinkedObject() == nullptr)
    {
        QAction* linkItem = new QAction(QString("Link Item..."), contextMenu);
        connect(linkItem, SIGNAL(triggered()), this, SLOT(itemLink()));
        contextMenu->addAction(linkItem);
    }
    else
    {
        QAction* unlinkItem = new QAction(QString("Unlink Item"), contextMenu);
        connect(unlinkItem, SIGNAL(triggered()), this, SLOT(itemUnlink()));
        contextMenu->addAction(unlinkItem);
    }

    if((_model) && (_model->getLayerScene().layerCount(DMHelper::LayerType_Tokens) > 1))
    {
        QAction* shiftItem = new QAction(QString("Change Layer..."), contextMenu);
        connect(shiftItem, SIGNAL(triggered()), this, SLOT(changeCombatantLayer()));
        contextMenu->addAction(shiftItem);
    }

    contextMenu->addSeparator();

    QAction* damageItem = new QAction(QString("Damage..."), contextMenu);
    connect(damageItem, SIGNAL(triggered()), this, SLOT(damageCombatant()));
    contextMenu->addAction(damageItem);

    QAction* healItem = new QAction(QString("Heal..."), contextMenu);
    connect(healItem, SIGNAL(triggered()), this, SLOT(healCombatant()));
    contextMenu->addAction(healItem);

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

void BattleFrame::layerSelected(int selected)
{
    if(_model)
        _model->getLayerScene().setSelectedLayerIndex(selected);
}

void BattleFrame::publishClicked(bool checked)
{
    if((!_model) || ((_isPublishing == checked) && (_renderer) && (_renderer->getObject() == _model)))
        return;

    _isPublishing = checked;
    if(_cameraRect)
        _cameraRect->setPublishing(_isPublishing);

    if(_isPublishing)
    {
        if(_renderer)
            emit registerRenderer(nullptr);

        PublishGLBattleRenderer* newRenderer;
        if(_isVideo)
            newRenderer = new PublishGLBattleVideoRenderer(_model);
        else
            newRenderer = new PublishGLBattleImageRenderer(_model);

        rendererActivated(newRenderer);
        emit registerRenderer(newRenderer);
        emit showPublishWindow();
    }
    else
    {
        emit registerRenderer(nullptr);
    }
}

void BattleFrame::setRotation(int rotation)
{
    if(_rotation == rotation)
        return;

    _rotation = rotation;

    if(_renderer)
        _renderer->setRotation(_rotation);
}

void BattleFrame::setBackgroundColor(const QColor& color)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to set the background color, no battle model is set!";
        return;
    }

    _model->setBackgroundColor(color);
}

/*
void BattleFrame::reloadObject()
{
    recreateCombatantWidgets();
    reloadMap();
}
*/
void BattleFrame::editLayers()
{
    if(!_model)
        return;

    LayersEditDialog dlg(_model->getLayerScene(), _model);
    dlg.resize(width() * 9 / 10, height() * 9 / 10);
    dlg.exec();

    emit setLayers(_model->getLayerScene().getLayers(), _model->getLayerScene().getSelectedLayerIndex());
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
                applyEffectToItem(item, effect);
        }
    }
}

void BattleFrame::handleEffectRemoved(QGraphicsItem* effectItem)
{
    Q_UNUSED(effectItem);

    for(QGraphicsPixmapItem* item : _combatantIcons.values())
    {
        // OPTIMIZE: Optimize to only remove effects if not still relevant
        if(item)
            removeEffectsFromItem(item);
    }
}

void BattleFrame::handleCombatantMoved(BattleDialogModelObject* object)
{
    BattleDialogModelCombatant* combatant = dynamic_cast<BattleDialogModelCombatant*>(object);

    if(!combatant)
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

void BattleFrame::handleCombatantActivate(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    qDebug() << "[Battle Frame] activating combatant " << combatant->getName();
    setActiveCombatant(combatant);
}

void BattleFrame::handleCombatantRemove(BattleDialogModelCombatant* combatant)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to remove combatant, no battle model is set!";
        return;
    }

    if(!combatant)
        return;

    qDebug() << "[Battle Frame] removing combatant " << combatant->getName();
    removeRollover();

    // Check the active combatant highlight
    if(combatant == _model->getActiveCombatant())
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
    else if(combatant == ui->frameCombatant->getCombatant())
    {
        ui->frameCombatant->setCombatant(nullptr);
    }

    // Find the index of the removed item
    int index = _model->getCombatantList().indexOf(combatant);

    // Delete the widget for the combatant
    _combatantWidgets.remove(combatant);
    QLayoutItem *child = _combatantLayout->takeAt(index);
    if(child != nullptr)
    {
        qDebug() << "[Battle Frame] deleting combatant widget: " << reinterpret_cast<quint64>(child->widget());
        child->widget()->deleteLater();
        delete child;
    }

    _model->removeCombatant(combatant);

    /*
    // Remove the combatant
    BattleDialogModelCombatant* removedCombatant = _model->removeCombatant(i);
    if(removedCombatant)
    {
        qDebug() << "[Battle Frame] removing combatant from list " << removedCombatant->getName();

        // Remove the widget from the list of widgets
        // TODO: should this be above with deleting the widget for the combatant?

        // Remove the icon from the list of icons and delete it from the scene
        QGraphicsPixmapItem* item = _combatantIcons.take(removedCombatant);
        if(item)
            delete item;

        delete removedCombatant;
    }
    */
}

void BattleFrame::handleCombatantChangeLayer(BattleDialogModelCombatant* combatant)
{
    if((!combatant) || (!_model))
        return;

    QList<Layer*> tokenLayers = _model->getLayerScene().getLayers(DMHelper::LayerType_Tokens);
    if(tokenLayers.count() <= 1)
        return;

    int currentLayerIndex = 0;
    LayerTokens* currentLayer = nullptr;
    QStringList tokenLayerNames;
    for(int i = 0; i < tokenLayers.count(); ++i)
    {
        LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(tokenLayers.at(i));
        tokenLayerNames << (tokenLayer ? tokenLayer->getName() : QString(""));
        if((tokenLayer) && (tokenLayer->containsCombatant(combatant)))
        {
            currentLayerIndex = i;
            currentLayer = tokenLayer;
        }
    }

    if(!currentLayer)
        return;

    SelectItemDialog dlg(tokenLayerNames);
    dlg.setSelectedItem(currentLayerIndex);
    if(dlg.exec() == QDialog::Accepted)
    {
        if(dlg.getSelectedItem() == currentLayerIndex)
            return;

        LayerTokens* newLayer= dynamic_cast<LayerTokens*>(tokenLayers.at(dlg.getSelectedItem()));
        if(!newLayer)
            return;

        currentLayer->removeCombatant(combatant);
        newLayer->addCombatant(combatant);
    }
}

void BattleFrame::handleCombatantDamage(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    int damage = QInputDialog::getInt(this, QString("Damage Combatant: ") + combatant->getName(), QString("Please enter the amount of damage to be done: "));
    applyCombatantHPChange(combatant, -damage);
}

void BattleFrame::handleCombatantHeal(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    int heal = QInputDialog::getInt(this, QString("Heal Combatant: ") + combatant->getName(), QString("Please enter the amount of healing to be applied: "));
    applyCombatantHPChange(combatant, heal);
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

    DiceRollDialogCombatants* dlg = new DiceRollDialogCombatants(Dice(1, 20, 0), combatantList, 15, this);
    connect(dlg, SIGNAL(selectCombatant(BattleDialogModelCombatant*)), this, SLOT(setSelectedCombatant(BattleDialogModelCombatant*)));
    connect(dlg, SIGNAL(combatantChanged(BattleDialogModelCombatant*)), this, SLOT(updateCombatantWidget(BattleDialogModelCombatant*)));
    connect(dlg, SIGNAL(hitPointsChanged(BattleDialogModelCombatant*, int)), this, SLOT(updateCombatantVisibility()));
    connect(dlg, SIGNAL(hitPointsChanged(BattleDialogModelCombatant*, int)), this, SLOT(registerCombatantDamage(BattleDialogModelCombatant*, int)));
    dlg->resize(800, 600);

    dlg->fireAndForget();
}

void BattleFrame::handleItemLink(BattleDialogModelObject* item)
{
    if(!_model)
        return;

    SelectCombatantDialog dlg(*_model, item);
    dlg.resize(width() / 2, height() / 2);

    int result = dlg.exec();
    if(result != QDialog::Accepted)
        return;

    BattleDialogModelObject* selectedObject = dlg.getSelectedObject();
    if(!selectedObject)
        return;

    if(dlg.isCentered())
    {
        QGraphicsItem* linkedItem = _model->getObjectItem(item);
        QGraphicsItem* selectedItem = _model->getObjectItem(selectedObject);

        if((linkedItem) && (selectedItem))
            linkedItem->setPos(selectedItem->scenePos());
    }
    item->setLinkedObject(selectedObject);
}

void BattleFrame::handleItemUnlink(BattleDialogModelObject* item)
{
    if(!item)
        return;

    item->setLinkedObject(nullptr);
}

void BattleFrame::handleItemMouseDown(QGraphicsPixmapItem* item)
{
    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to handle item mouse down, no battle model is set!";
        return;
    }

    QList<Layer*> tokenLayers = _model->getLayerScene().getLayers(DMHelper::LayerType_Tokens);
    foreach(Layer* layer, tokenLayers)
    {
        if(layer)
        {
            LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(layer);
            if(tokenLayer)
            {
                BattleDialogModelCombatant* combatant = tokenLayer->getCombatantFromItem(item);
                if(combatant)
                {
                    startMovement(combatant, item, combatant->getSpeed());
                    _selectedCombatant = combatant;
                    ui->frameCombatant->setCombatant(combatant);

                    CombatantWidget* widget = _combatantWidgets.value(combatant, nullptr);
                    if(widget)
                        ui->scrollArea->ensureWidgetVisible(widget);
                }
            }
        }
    }

    /*
    BattleDialogModelCombatant* combatant = _combatantIcons.key(item, nullptr);
    if(combatant)
    {
        startMovement(combatant, _combatantIcons.value(combatant), combatant->getSpeed());
        _selectedCombatant = combatant;
        ui->frameCombatant->setCombatant(combatant);

        CombatantWidget* widget = _combatantWidgets.value(combatant, nullptr);
        if(widget)
            ui->scrollArea->ensureWidgetVisible(widget);
    }
    */
}

void BattleFrame::handleItemMoved(QGraphicsPixmapItem* item, bool* result)
{
    Q_UNUSED(result);

    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible handle the item movement, no battle model is set!";
        return;
    }

    updateMovement(_selectedCombatant, item);
}

void BattleFrame::handleItemMouseUp(QGraphicsPixmapItem* item)
{
    Q_UNUSED(item);

    endMovement();
    _selectedCombatant = nullptr;
}

void BattleFrame::handleItemChanged(QGraphicsItem* item)
{
    if((_cameraRect) && (_cameraRect == item))
        updateCameraRect();
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

void BattleFrame::handleMapMousePress(const QPointF& pos)
{
    _mouseDown = true;
    _mouseDownPos = ui->graphicsView->mapFromScene(pos);
}

void BattleFrame::handleMapMouseMove(const QPointF& pos)
{
    if(!_mouseDown)
        return;

    QPoint viewPos = ui->graphicsView->mapFromScene(pos);
    QPoint delta = _mouseDownPos - viewPos;

    _mouseDown = false;
    if(ui->graphicsView->horizontalScrollBar())
        ui->graphicsView->horizontalScrollBar()->setValue(ui->graphicsView->horizontalScrollBar()->value() + delta.x());
    if(ui->graphicsView->verticalScrollBar())
        ui->graphicsView->verticalScrollBar()->setValue(ui->graphicsView->verticalScrollBar()->value() + delta.y());
    _mouseDown = true;

    _mouseDownPos = viewPos;
}

void BattleFrame::handleMapMouseRelease(const QPointF& pos)
{
    Q_UNUSED(pos);
    _mouseDown = false;
}

void BattleFrame::handleSceneChanged(const QList<QRectF> &region)
{
    Q_UNUSED(region);

    if((_isPublishing) && (_renderer))
        _renderer->updateRender();
}

void BattleFrame::itemLink()
{
    handleItemLink(_contextMenuCombatant);
}

void BattleFrame::itemUnlink()
{
    handleItemUnlink(_contextMenuCombatant);
}

void BattleFrame::removeCombatant()
{
    handleCombatantRemove(_contextMenuCombatant);
}

void BattleFrame::activateCombatant()
{
    handleCombatantActivate(_contextMenuCombatant);
}

void BattleFrame::changeCombatantLayer()
{
    handleCombatantChangeLayer(_contextMenuCombatant);
}

void BattleFrame::damageCombatant()
{
    handleCombatantDamage(_contextMenuCombatant);
}

void BattleFrame::healCombatant()
{
    handleCombatantHeal(_contextMenuCombatant);
}

void BattleFrame::applyCombatantHPChange(BattleDialogModelCombatant* combatant, int hpChange)
{
    if(!combatant)
        return;

    combatant->setHitPoints(combatant->getHitPoints() + hpChange);
    updateCombatantWidget(combatant);
    updateCombatantVisibility();

    registerCombatantDamage(combatant, hpChange);
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
            combatantWidget->setSelected(!isSelected);

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

void BattleFrame::copyMonsters()
{
    _copyList.clear();
    QList<QGraphicsItem*> selected = _scene->selectedItems();

    for(int i = 0; i < selected.count(); ++i)
    {
        QGraphicsPixmapItem* item = dynamic_cast<QGraphicsPixmapItem*>(selected.at(i));
        if(item)
        {
            BattleDialogModelCombatant* combatant = _combatantIcons.key(item);
            if((combatant) && (combatant->getCombatantType() == DMHelper::CombatantType_Monster))
                _copyList.append(combatant);
        }
    }
}

void BattleFrame::clearCopy()
{
    _copyList.clear();
}

void BattleFrame::pasteMonsters()
{
    if(_copyList.count() == 0)
        return;

    // Need this because changing the combatant list clears the copy list to avoid issues
    QList<BattleDialogModelCombatant*> copyListBackup = _copyList;

    // Go through the copied monsters and find their rectangle
    QRectF monsterRect(_copyList.at(0)->getPosition(), QSizeF(0, 0));
    for(int i = 1; i < _copyList.count(); ++i)
    {
        monsterRect.setTop(qMin(monsterRect.top(), _copyList.at(i)->getPosition().y()));
        monsterRect.setLeft(qMin(monsterRect.left(), _copyList.at(i)->getPosition().x()));
        monsterRect.setBottom(qMax(monsterRect.bottom(), _copyList.at(i)->getPosition().y()));
        monsterRect.setRight(qMax(monsterRect.right(), _copyList.at(i)->getPosition().x()));
    }
    QPointF combatantPos = viewportCenter();

    QList<BattleDialogModelCombatant*> newCombatants;
    for(int i = 0; i < _copyList.count(); ++i)
    {
        BattleDialogModelCombatant* newCombatant = _copyList.at(i)->clone();
        newCombatant->setPosition(combatantPos + _copyList.at(i)->getPosition() - monsterRect.center());
        newCombatants.append(newCombatant);
    }
    addCombatants(newCombatants);
    recreateCombatantWidgets();

    _copyList = copyListBackup;
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

    if((_renderer) && (_countdownFrame.height() > 10))
        _renderer->setCountdownValues(_countdown / static_cast<qreal>(_countdownFrame.height() - 10), _countdownColor);
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
            if(_cameraRect)
            {
                QRectF cameraRect = ui->graphicsView->mapToScene(_rubberBandRect).boundingRect();
                /*
                if((_isGridLocked) && (!_targetSize.isEmpty()))
                {
                    cameraRect.setHeight(cameraRect.width() * static_cast<qreal>(_targetSize.height()) / static_cast<qreal>(_targetSize.width()));
                    setGridScale(_gridLockScale * cameraRect.width() / static_cast<qreal>(_targetSize.width()));
                }
                */
                if((_isGridLocked) && (!_targetSize.isEmpty()))
                    cameraRect.setHeight(cameraRect.width() * static_cast<qreal>(_targetSize.height()) / static_cast<qreal>(_targetSize.width()));

                _cameraRect->setCameraRect(cameraRect);
                //emit cameraRectChanged(_cameraRect->getCameraRect());
            }
        }
        else if(_stateMachine.getCurrentStateId() == DMHelper::BattleFrameState_FoWSelect)
        {
            if(_mapDrawer)
                _mapDrawer->drawRect(ui->graphicsView->mapToScene(_rubberBandRect).boundingRect().toRect());
        }

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
    _scene->setEffectVisibility(visibility);
    ui->graphicsView->invalidateScene();
}

void BattleFrame::setMapCursor()
{
}

void BattleFrame::setCameraSelectable(bool selectable)
{
    if(_cameraRect)
        _cameraRect->setCameraSelectable(selectable);
}

void BattleFrame::setScale(qreal s)
{
    _scale = s;
    ui->graphicsView->setTransform(QTransform::fromScale(_scale, _scale));
    setMapCursor();
    storeViewRect();

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
}

void BattleFrame::setModel(BattleDialogModel* model)
{
    qDebug() << "[Battle Frame] Setting battle model to: " << model;

    if(_model)
    {
        // TODO: Layers
        //disconnect(_model, &BattleDialogModel::gridScaleChanged, this, &BattleFrame::gridScaleChanged);
        disconnect(_model, SIGNAL(showAliveChanged(bool)), this, SLOT(updateCombatantVisibility()));
        disconnect(_model, SIGNAL(showDeadChanged(bool)), this, SLOT(updateCombatantVisibility()));
        disconnect(_model, SIGNAL(showEffectsChanged(bool)), this, SLOT(updateEffectLayerVisibility()));
        disconnect(_model, &BattleDialogModel::combatantListChanged, this, &BattleFrame::clearCopy);
        disconnect(_mapDrawer, &BattleFrameMapDrawer::fowEdited, _model, &BattleDialogModel::dirty);

        clearBattleFrame();
        cleanupBattleMap();
        clearCombatantWidgets();

        emit setLayers(QList<Layer*>(), 0);
    }

    _model = model;

    _scene->setModel(model);

    ui->btnSort->setEnabled(_model != nullptr);
    ui->btnTop->setEnabled(_model != nullptr);
    ui->btnNext->setEnabled(_model != nullptr);
    ui->edtRounds->setEnabled(_model != nullptr);
    ui->edtCountdown->setEnabled(_model != nullptr);
    updatePublishEnable();
    ui->graphicsView->setEnabled(_model != nullptr);

    if(_model)
    {
        emit backgroundColorChanged(_model->getBackgroundColor());

        // TODO: Layers
        //connect(_model, &BattleDialogModel::gridScaleChanged, this, &BattleFrame::gridScaleChanged);
        connect(_model, SIGNAL(showAliveChanged(bool)), this, SLOT(updateCombatantVisibility()));
        connect(_model, SIGNAL(showDeadChanged(bool)), this, SLOT(updateCombatantVisibility()));
        connect(_model, SIGNAL(showEffectsChanged(bool)), this, SLOT(updateEffectLayerVisibility()));
        connect(_model, &BattleDialogModel::combatantListChanged, this, &BattleFrame::clearCopy);
        connect(_mapDrawer, &BattleFrameMapDrawer::fowEdited, _model, &BattleDialogModel::dirty);

        setBattleMap();
        recreateCombatantWidgets();

        emit setLayers(_model->getLayerScene().getLayers(), _model->getLayerScene().getSelectedLayerIndex());

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

    MapSelectDialog mapSelectDlg(*campaign, _battle->getID());
    connect(&mapSelectDlg, &MapSelectDialog::mapCreated, this, &BattleFrame::mapCreated);
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

        connect(_scene, SIGNAL(battleMousePress(const QPointF&)), _mapDrawer, SLOT(handleMouseDown(const QPointF&)));
        connect(_scene, SIGNAL(battleMouseMove(const QPointF&)), _mapDrawer, SLOT(handleMouseMoved(const QPointF&)));
        connect(_scene, SIGNAL(battleMouseRelease(const QPointF&)), _mapDrawer, SLOT(handleMouseUp(const QPointF&)));

        if(_model)
        {
            QRect viewSize = ui->graphicsView->mapFromScene(QRect(0, 0, _model->getGridScale(), _model->getGridScale())).boundingRect();
            _mapDrawer->setScale(_model->getGridScale(), viewSize.width());
            // TODO: Layers
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
    }
}

void BattleFrame::updateFowImage(const QPixmap& fow)
{
    // TODO: Layers
    // This probably can disappear? Make sure connections between drawer and scene are done
    //if(_fow)
    //    _fow->setPixmap(fow);
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

    _hoverFrame->cancelClose();
    _hoverFrame->deleteLater();
    _hoverFrame = nullptr;
}

void BattleFrame::handleScreenshotReady(const QImage& image)
{
    if((image.isNull()) || (!_mapDrawer) || (!_model) || (!_model->getMap()))
        return;

    if(_model->getBackgroundImage().isNull())
        _model->setBackgroundImage(image);
    // TODO: Layers
    //_background->setPixmap((QPixmap::fromImage(image)));
    if(_model->getMap()->getFoWImage().isNull())
    {
        QImage fowImage = QImage(image.size(), QImage::Format_ARGB32);
        fowImage.fill(QColor(0, 0, 0, 0));
        _model->getMap()->setExternalFoWImage(fowImage);
    }
    _fowImage = QPixmap::fromImage(_model->getMap()->getFoWImage());
    // TODO: Layers
    //_bwFoWImage = _model->getMap()->getBWFoWImage(image.size());
    _mapDrawer->setMap(_model->getMap(), &_fowImage, &_bwFoWImage);

    if(!_model->getCameraRect().isValid())
        _model->setCameraRect(_model->getLayerScene().boundingRect().toRect());

    if(_cameraRect)
        _cameraRect->setCameraRect(_model->getCameraRect());
    else
        _cameraRect = new CameraRect(_model->getCameraRect(), *_scene, ui->graphicsView->viewport(), _isGridLocked);
    emit cameraRectChanged(_model->getCameraRect());
}

void BattleFrame::rendererActivated(PublishGLBattleRenderer* renderer)
{
    if((!renderer) || (!_battle) || (renderer->getObject() != _battle->getBattleDialogModel()))
        return;

    // TODO: Layers
//    connect(_mapDrawer, &BattleFrameMapDrawer::fowChanged, renderer, &PublishGLBattleRenderer::fowChanged);
//    connect(_scene, &BattleDialogGraphicsScene::pointerMove, renderer, &PublishGLRenderer::setPointerPosition);
    connect(_scene, &BattleDialogGraphicsScene::distanceChanged, renderer, &PublishGLBattleRenderer::distanceChanged);
    connect(_scene, &BattleDialogGraphicsScene::distanceItemChanged, renderer, &PublishGLBattleRenderer::distanceItemChanged);
    connect(this, &BattleFrame::cameraRectChanged, renderer, &PublishGLBattleRenderer::setCameraRect);
    connect(this, &BattleFrame::pointerToggled, renderer, &PublishGLRenderer::pointerToggled);
    connect(this, &BattleFrame::pointerFileNameChanged, renderer, &PublishGLRenderer::setPointerFileName);
    connect(this, &BattleFrame::movementChanged, renderer, &PublishGLBattleRenderer::movementChanged);
    connect(renderer, &PublishGLRenderer::deactivated, this, &BattleFrame::rendererDeactivated);
//    connect(renderer, &PublishGLRenderer::initializationComplete, this, &BattleFrame::updateRendererGrid);

    renderer->setPointerFileName(_pointerFile);
    renderer->setActiveToken(_activeFile);
    renderer->setSelectionToken(_scene->getSelectedIconFile());
    renderer->setRotation(_rotation);
    renderer->setInitiativeType(_initiativeType);
    // TODO: Layers
    //_fowImage = QPixmap::fromImage(_model->getMap()->getFoWImage());
    //_bwFoWImage = _model->getMap()->getBWFoWImage(_fowImage.size());
    //renderer->fowChanged(_bwFoWImage);
    renderer->setCombatantFrame(_combatantFile);
    renderer->setCountdownFrame(_countdownFile);
    renderer->setShowCountdown(_showCountdown);
    if(_countdownFrame.height() > 10)
        renderer->setCountdownValues(_countdown / static_cast<qreal>(_countdownFrame.height() - 10), _countdownColor);
    renderer->setCountdownValues(_countdown, _countdownColor);

    if(_cameraRect)
        renderer->setCameraRect(_cameraRect->getCameraRect());

    _renderer = renderer;
//    updateRendererGrid();
}

void BattleFrame::rendererDeactivated()
{
    if(!_renderer)
        return;

    // TODO: Layers
//    disconnect(_mapDrawer, &BattleFrameMapDrawer::fowChanged, _renderer, &PublishGLBattleRenderer::fowChanged);
    disconnect(_scene, &BattleDialogGraphicsScene::pointerMove, _renderer, &PublishGLRenderer::setPointerPosition);
    disconnect(_scene, &BattleDialogGraphicsScene::distanceChanged, _renderer, &PublishGLBattleRenderer::distanceChanged);
    disconnect(_scene, &BattleDialogGraphicsScene::distanceItemChanged, _renderer, &PublishGLBattleRenderer::distanceItemChanged);
    disconnect(this, &BattleFrame::cameraRectChanged, _renderer, &PublishGLBattleRenderer::setCameraRect);
    disconnect(this, &BattleFrame::pointerToggled, _renderer, &PublishGLRenderer::pointerToggled);
    disconnect(this, &BattleFrame::pointerFileNameChanged, _renderer, &PublishGLRenderer::setPointerFileName);
    disconnect(this, &BattleFrame::movementChanged, _renderer, &PublishGLBattleRenderer::movementChanged);
    disconnect(_renderer, &PublishGLRenderer::deactivated, this, &BattleFrame::rendererDeactivated);
//    disconnect(_renderer, &PublishGLRenderer::initializationComplete, this, &BattleFrame::updateRendererGrid);

    _renderer = nullptr;
}

/*
void BattleFrame::updateRendererGrid()
{
    if(!_renderer)
        return;

    QImage gridImage(_renderer->getBackgroundSize().toSize(), QImage::Format_RGBA8888);
    if(gridImage.isNull())
        return;

    gridImage.fill(Qt::transparent);

    QPainter gridPainter;
    gridPainter.begin(&gridImage);
        gridPainter.setRenderHint(QPainter::Antialiasing);
        _scene->paintGrid(&gridPainter);
    gridPainter.end();

    _renderer->setGrid(gridImage);
}
*/

void BattleFrame::stateUpdated()
{
    BattleFrameState* currentState = _stateMachine.getCurrentState();

    if((currentState == nullptr) || (currentState->getType() == BattleFrameState::BattleFrameStateType_Base))
        ui->graphicsView->viewport()->unsetCursor();
    else
        ui->graphicsView->viewport()->setCursor(currentState->getCursor());
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
                connect(widgetInternals, SIGNAL(clicked(QUuid)), this, SIGNAL(characterSelected(QUuid)));
                connect(widgetInternals, SIGNAL(contextMenu(BattleDialogModelCombatant*, QPoint)), this, SLOT(handleContextMenu(BattleDialogModelCombatant*, QPoint)));
                connect(widgetInternals, SIGNAL(hitPointsChanged(BattleDialogModelCombatant*, int)), this, SLOT(updateCombatantVisibility()));
                connect(widgetInternals, SIGNAL(hitPointsChanged(BattleDialogModelCombatant*, int)), this, SLOT(registerCombatantDamage(BattleDialogModelCombatant*, int)));
                connect(dynamic_cast<WidgetCharacter*>(newWidget), SIGNAL(isShownChanged(bool)), character, SLOT(setShown(bool)));
                connect(dynamic_cast<WidgetCharacter*>(newWidget), SIGNAL(isKnownChanged(bool)), character, SLOT(setKnown(bool)));
                connect(newWidget, SIGNAL(imageChanged(BattleDialogModelCombatant*)), this, SLOT(updateCombatantIcon(BattleDialogModelCombatant*)));
                connect(character, SIGNAL(moveUpdated()), newWidget, SLOT(updateMove()));
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
                connect(widgetInternals, SIGNAL(clicked(const QString&)), this, SIGNAL(monsterSelected(const QString&)));
                connect(widgetInternals, SIGNAL(contextMenu(BattleDialogModelCombatant*, QPoint)), this, SLOT(handleContextMenu(BattleDialogModelCombatant*, QPoint)));
                connect(widgetInternals, SIGNAL(hitPointsChanged(BattleDialogModelCombatant*, int)), this, SLOT(updateCombatantVisibility()));
                connect(widgetInternals, SIGNAL(hitPointsChanged(BattleDialogModelCombatant*, int)), this, SLOT(registerCombatantDamage(BattleDialogModelCombatant*, int)));
                connect(dynamic_cast<WidgetMonster*>(newWidget), SIGNAL(isShownChanged(bool)), monster, SLOT(setShown(bool)));
                connect(dynamic_cast<WidgetMonster*>(newWidget), SIGNAL(isKnownChanged(bool)), monster, SLOT(setKnown(bool)));
                connect(newWidget, SIGNAL(imageChanged(BattleDialogModelCombatant*)), this, SLOT(updateCombatantIcon(BattleDialogModelCombatant*)));
                connect(monster, SIGNAL(moveUpdated()), newWidget, SLOT(updateMove()));
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
            _combatantLayout->addWidget(widget);
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
        active->resetMoved();
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
    return;

    if(!_model)
    {
        qDebug() << "[Battle Frame] ERROR: Not possible to create combatant icon, no battle model is set!";
        return;
    }

    if(combatant)
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
        qreal scaleFactor = (static_cast<qreal>(_model->getGridScale()-2)) * sizeFactor / static_cast<qreal>(qMax(pix.width(), pix.height()));
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

        connect(combatant, SIGNAL(objectMoved(BattleDialogModelObject*)), this, SLOT(handleCombatantMoved(BattleDialogModelObject*)), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
        connect(combatant, SIGNAL(objectMoved(BattleDialogModelObject*)), this, SLOT(updateHighlights()), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
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
        icon->setPos(mapPos - _model->getMapRect().topLeft());
    else
        icon->setPos(_model->getMapRect().center());
}

QWidget* BattleFrame::findCombatantWidgetFromPosition(const QPoint& position) const
{
    qDebug() << "[Battle Frame] searching for widget from position " << position.x() << "x" << position.y() << "...";
    QWidget* widget = ui->scrollAreaWidgetContents->childAt(position);

    if(widget)
    {
        while((widget->parentWidget() != ui->scrollAreaWidgetContents) && (widget->parentWidget() != nullptr))
            widget = widget->parentWidget();

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
        return dynamic_cast<CombatantWidget*>(_combatantLayout->itemAt(pos)->widget());
    else
        return nullptr;
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
            nextHighlight = 0;
    } while(((_model->getCombatant(nextHighlight)->getHitPoints() <= 0) ||
             (!_model->getCombatant(nextHighlight)->getKnown())) &&
            (_model->getCombatant(nextHighlight) != _model->getActiveCombatant()));

    return _model->getCombatant(nextHighlight);
}

void BattleFrame::updatePublishEnable()
{
    //emit setPublishEnabled((_model) && (_model->getMap()), true);
    emit setPublishEnabled(_model != nullptr, true);
}

void BattleFrame::clearBattleFrame()
{
    qDebug() << "[Battle Frame] Clearing Battle Frame";

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
    // Unload the map
    if(_mapDrawer)
    {
#if defined(Q_OS_WIN32) && !defined(Q_OS_WIN64)
        if(_mapDrawer->getMap())
            _mapDrawer->getMap()->uninitialize();
#endif

        _mapDrawer->setMap(nullptr, nullptr, nullptr);
    }

    // Clean up the old map
    _scene->clearBattleContents();
    delete _activePixmap; _activePixmap = nullptr;
    delete _cameraRect; _cameraRect = nullptr;
    delete _movementPixmap; _movementPixmap = nullptr;

    if(_model)
        _model->getLayerScene().dmUninitialize();

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

    // TODO: Layers
    //if(!_model->getMap())
    //    return;

    //_background = new UnselectedPixmap();
    //_fow = new UnselectedPixmap();
    _model->setBackgroundImage(QImage());

    updateMap();

    // TODO: Layers
    //_scene->addItem(_background);
    //_fow->setParentItem(_background);
    //_background->setEnabled(false);
    //_background->setZValue(DMHelper::BattleDialog_Z_Background);

    createSceneContents();

    //qDebug() << "[Battle Frame] map set to new image (" << _model->getMap()->getFileName() << ")";
}

bool BattleFrame::doSceneContentsExist()
{
    return((_activePixmap != nullptr) ||
           (_movementPixmap != nullptr) ||
           (_combatantIcons.count() > 0) ||
           (!_scene->isSceneEmpty()));
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

    _scene->createBattleContents();

    createActiveIcon();
    createCombatantFrame();
    createCountdownFrame();

    QPen movementPen(QColor(23, 23, 23, 200), 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
    _movementPixmap = _scene->addEllipse(0, 0, 100, 100, movementPen, QBrush(QColor(255, 255, 255, 25)));
    _movementPixmap->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    _movementPixmap->setVisible(false);

    if(_model->getCameraRect().isValid())
    {
        _cameraRect = new CameraRect(_model->getCameraRect().width(), _model->getCameraRect().height(), *_scene, ui->graphicsView->viewport(), _isGridLocked);
        _cameraRect->setPos(_model->getCameraRect().x(), _model->getCameraRect().y());
    }
    else
    {
        _cameraRect = new CameraRect(_scene->width(), _scene->height(), *_scene, ui->graphicsView->viewport(), _isGridLocked);
        _cameraRect->setPos(0, 0);
    }
    _cameraRect->setRatioLocked(_isGridLocked);
    updateCameraRect();

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

    _scene->resizeBattleContents();
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
    return ((_initiativeType != DMHelper::InitiativeType_None) ? _combatantFrame.width() : 0) +
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
        if(_initiativeType != DMHelper::InitiativeType_None)
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
        return sizeBackgroundToFrame(originalBackgroundSize);
    else
        return sizeBackgroundToFrame(originalBackgroundSize.transposed()).transposed();
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

    if(publishWidth <= 0)
        return false;

    scenePosition = QPointF((publishX * _publishRectValue.width()) + _publishRectValue.x(),
                            (publishY * _publishRectValue.height()) + _publishRectValue.y());

    return true;
}

void BattleFrame::updateCameraRect()
{
    if(_cameraRect)
    {
        if(_isGridLocked)
            setGridScale(_gridLockScale * _cameraRect->getCameraRect().width() / static_cast<qreal>(_targetSize.width()));

        _publishRectValue = _cameraRect->rect();
        _publishRectValue.moveTo(_cameraRect->pos());
        emit cameraRectChanged(_cameraRect->getCameraRect());
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
    if(!_cameraRect)
        return;

    QRectF viewRect = ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect();

    if(_isGridLocked)
    {
        QRectF currentRect = _cameraRect->getCameraRect();
        QPointF newCenter = viewRect.center();
        currentRect.moveTo(newCenter.x() - (currentRect.width() / 2.0), newCenter.y() - (currentRect.height() / 2.0));
        viewRect = currentRect;
    }
    else
    {
        viewRect.adjust(1.0, 1.0, -1.0, -1.0);
    }

    _cameraRect->setCameraRect(viewRect);
    emit cameraRectChanged(viewRect);
}

void BattleFrame::extractDMScreenshot()
{
    if((!_model) || (!_model->getMap()))
        return;

    VideoPlayerGLScreenshot* screenshot = new VideoPlayerGLScreenshot(_model->getMap()->getFileName());
    connect(screenshot, &VideoPlayerGLScreenshot::screenshotReady, this, &BattleFrame::handleScreenshotReady);
    screenshot->retrieveScreenshot();
}

BattleDialogModelEffect* BattleFrame::createEffect(int type, int size, int width, const QColor& color, const QString& filename)
{
    if(!_scene)
        return nullptr;

    BattleDialogModelEffect* result = nullptr;
    qreal scaledHalfSize;
    QPointF effectPosition = _scene->getCommandPosition() != DMHelper::INVALID_POINT ? _scene->getCommandPosition() : _scene->getViewportCenter();

    switch(type)
    {
        case BattleDialogModelEffect::BattleDialogModelEffect_Radius:
            result = BattleDialogModelEffectFactory::createEffectRadius(effectPosition, size, color);
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Cone:
            result = BattleDialogModelEffectFactory::createEffectCone(effectPosition, size, color);
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Cube:
            scaledHalfSize = static_cast<qreal>(size) * _model->getGridScale() / (5.0 * 2.0);
            effectPosition -= QPointF(scaledHalfSize, scaledHalfSize);
            result = BattleDialogModelEffectFactory::createEffectCube(effectPosition, size, color);
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Line:
            result = BattleDialogModelEffectFactory::createEffectLine(effectPosition, size, width, color);
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Object:
            result = BattleDialogModelEffectFactory::createEffectObject(effectPosition, QSize(width, size), color, filename);
            break;
        default:
            break;
    }

    return result;
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
    //effectItem->setFlag(QGraphicsItem::ItemNegativeZStacksBehindParent);
    //effectItem->setFlag(QGraphicsItem::ItemStacksBehindParent);
    //effectItem->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
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

void BattleFrame::startMovement(BattleDialogModelCombatant* combatant, QGraphicsPixmapItem* item, int speed)
{
    if((!combatant) || (!item) || (!_model))
        return;

    if((_movementPixmap) && (_model->getShowMovement()))
    {
        int speedSquares = 2 * (speed / 5) + 1;
        _moveRadius = _model->getGridScale() * speedSquares;
        _moveStart = item->scenePos();
        _movementPixmap->setPos(_moveStart);
        _movementPixmap->setRect(-_moveRadius/2.0, -_moveRadius/2.0, _moveRadius, _moveRadius);
        _movementPixmap->setVisible(true);

        emit movementChanged(true, combatant, _moveRadius);
    }
}

void BattleFrame::updateMovement(BattleDialogModelCombatant* combatant, QGraphicsPixmapItem* item)
{
    if((!combatant) || (!item) || (!_model))
        return;

    QPointF combatantPos = item->scenePos();
    QPointF diff = _moveStart - combatantPos;
    _moveStart = combatantPos;
    qreal delta = qSqrt((diff.x() * diff.x()) + (diff.y() * diff.y()));

    if(_model->getGridScale() > 0)
        combatant->incrementMoved(5.0 * delta / static_cast<qreal>(_model->getGridScale()));

    if(!_movementPixmap)
        return;

    if(_model->getShowMovement())
    {
        if(_moveRadius > _model->getGridScale())
            _moveRadius -= 2 * delta;

        if(_moveRadius <= _model->getGridScale())
        {
            _moveRadius = _model->getGridScale();
            _movementPixmap->setRotation(0.0);
            _movementPixmap->setVisible(false);
        }
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

    emit movementChanged(_movementPixmap->isVisible(), combatant, _moveRadius);
}

void BattleFrame::endMovement()
{
    if(!_movementPixmap)
        return;

    _movementPixmap->setRotation(0.0);
    _movementPixmap->setVisible(false);
    emit movementChanged(false, nullptr, 0.0);
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

    BattleFrameState* freeDistanceState = new BattleFrameState(DMHelper::BattleFrameState_FreeDistance, BattleFrameState::BattleFrameStateType_Persistent, QPixmap(":/img/data/icon_distancecursor.png"), 32, 32);
    connect(freeDistanceState, &BattleFrameState::stateChanged, this, &BattleFrame::freeDistanceToggled);
    _stateMachine.addState(freeDistanceState);

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
