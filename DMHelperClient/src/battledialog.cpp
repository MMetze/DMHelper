#include "battledialog.h"
#include "ui_battledialog.h"
#include "dmhsrc/dmconstants.h"
#include "dmhsrc/unselectedpixmap.h"
#include "dmhsrc/battledialogmodel.h"
#include "dmhsrc/battledialogmodelcharacter.h"
#include "dmhsrc/battledialogmodelmonsterbase.h"
#include "dmhsrc/battledialoggraphicsscene.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QMenu>
#include <QAction>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QtGlobal>
#include <QPen>
#include <QBrush>
#include <QTimer>
#include <QTime>
#include <QScrollBar>
#include <QInputDialog>

const qreal SELECTED_PIXMAP_SIZE = 800.0;
const qreal ACTIVE_PIXMAP_SIZE = 800.0;
const qreal COUNTDOWN_DURATION = 15;
const qreal COUNTDOWN_TIMER = 0.05;
const qreal COMPASS_SCALE = 0.25;

BattleDialog::BattleDialog(BattleDialogModel& model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BattleDialog),
    _model(model),
    _combatantIcons(),
    _selectedCombatant(nullptr),
    _contextMenuCombatant(nullptr),
    _mouseDown(false),
    _mouseDownPos(),
    _scene(nullptr),
    _background(nullptr),
    _activePixmap(nullptr),
    _activeScale(1),
    _selectedPixmap(nullptr),
    _selectedScale(1),
    _compassPixmap(nullptr),
    _countdownTimer(nullptr),
    _countdown(0),
    _prescaledBackground(),
    _combatantFrame(),
    _countdownFrame(),
    _targetSize(),
    _showCountdown(true),
    _countdownDuration(15),
    _countdownColor(0,0,0),
    _rubberBandRect(),
    _scale(1.0)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);

    _scene = new BattleDialogGraphicsScene(_model, this);
    ui->graphicsView->setScene(_scene);

    _countdownTimer = new QTimer(this);
    _countdownTimer->setSingleShot(false);
    connect(_countdownTimer, SIGNAL(timeout()),this,SLOT(countdownTimerExpired()));

    connect(ui->btnZoomIn,SIGNAL(clicked()),this,SLOT(zoomIn()));
    connect(ui->btnZoomIn,SIGNAL(clicked()),this,SLOT(cancelSelect()));
    connect(ui->btnZoomOut,SIGNAL(clicked()),this,SLOT(zoomOut()));
    connect(ui->btnZoomOut,SIGNAL(clicked()),this,SLOT(cancelSelect()));
    connect(ui->btnZoomFit,SIGNAL(clicked()),this,SLOT(zoomFit()));
    connect(ui->btnZoomFit,SIGNAL(clicked()),this,SLOT(cancelSelect()));
    connect(ui->btnZoomSelect,SIGNAL(clicked()),this,SLOT(zoomSelect()));
    connect(ui->graphicsView,SIGNAL(rubberBandChanged(QRect,QPointF,QPointF)),this,SLOT(handleRubberBandChanged(QRect,QPointF,QPointF)));

    connect(ui->btnDistance, SIGNAL(clicked(bool)), _scene, SLOT(setShowDistance(bool)));
    connect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(createPrescaledBackground()));
    connect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(createPrescaledBackground()));
    connect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
    connect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));

    connect(_scene, SIGNAL(selectionChanged()),this,SLOT(handleSelectionChanged()));
    connect(_scene, SIGNAL(effectChanged(QAbstractGraphicsShapeItem*)), this, SLOT(handleEffectChanged(QAbstractGraphicsShapeItem*)));
    //connect(_scene, SIGNAL(applyEffect(QAbstractGraphicsShapeItem*)), this, SLOT(handleApplyEffect(QAbstractGraphicsShapeItem*)));
    connect(_scene, SIGNAL(distanceChanged(const QString&)), ui->edtDistance, SLOT(setText(const QString&)));

    _combatantFrame.load(":/img/data/combatant_frame.png");
    _countdownFrame.load(":/img/data/countdown_frame.png");

    setMapCursor();

    qDebug() << "[Battle Dialog] created";
}

BattleDialog::~BattleDialog()
{
    delete ui;
    qDebug() << "[Battle Dialog] destroyed.";
}

BattleDialogModel& BattleDialog::getModel()
{
    return _model;
}

const BattleDialogModel& BattleDialog::getModel() const
{
    return _model;
}

/*
void BattleDialog::setBattleMap()
{
    if(_model.isMapChanged())
    {
        replaceBattleMap();
    }
    else
    {
        resizeBattleMap();
    }
}
*/

void BattleDialog::addCombatant(BattleDialogModelCombatant* combatant)
{
    qDebug() << "[Battle Dialog] combatant added, type " << combatant->getType() << ", init " << combatant->getInitiative() << ", pos " << combatant->getPosition();

    _model.appendCombatant(combatant);
    createCombatantIcon(combatant);
}

void BattleDialog::addCombatants(QList<BattleDialogModelCombatant*> combatants)
{
    for(int i = 0; i < combatants.count(); ++i)
    {
        addCombatant(combatants.at(i));
    }
}

QList<BattleDialogModelCombatant*> BattleDialog::getCombatants() const
{
    return _model.getCombatantList();
}

/*
QList<BattleDialogModelCombatant*> BattleDialog::getLivingCombatants() const
{
    QList<BattleDialogModelCombatant*> result;
    for(int i = 0; i < _model.getCombatantCount(); ++i)
    {
        if((_model.getCombatant(i)) && (_model.getCombatant(i)->getHitPoints() > 0))
        {
            result.append(_model.getCombatant(i));
        }
    }

    qDebug() << "[Battle Dialog] " << result.count() << " living combatants found.";

    return result;
}

BattleDialogModelCombatant* BattleDialog::getFirstLivingCombatant() const
{
    for(int i = 0; i < _model.getCombatantCount(); ++i)
    {
        if((_model.getCombatant(i)) && (_model.getCombatant(i)->getHitPoints() > 0))
        {
            qDebug() << "[Battle Dialog] first living combatants found: " << _model.getCombatant(i)->getName();
            return _model.getCombatant(i);
        }
    }

    qDebug() << "[Battle Dialog] No first living combatants found.";

    return nullptr;
}

QList<BattleDialogModelCombatant*> BattleDialog::getMonsters() const
{
    QList<BattleDialogModelCombatant*> result;
    for(int i = 0; i < _model.getCombatantCount(); ++i)
    {
        if((_model.getCombatant(i)) && (_model.getCombatant(i)->getType() == DMHelper::CombatantType_Monster))
        {
            result.append(_model.getCombatant(i));
        }
    }

    qDebug() << "[Battle Dialog] " << result.count() << " monster combatants found.";

    return result;
}

QList<BattleDialogModelCombatant*> BattleDialog::getLivingMonsters() const
{
    QList<BattleDialogModelCombatant*> result;
    for(int i = 0; i < _model.getCombatantCount(); ++i)
    {
        if((_model.getCombatant(i)) && (_model.getCombatant(i)->getType() == DMHelper::CombatantType_Monster) && (_model.getCombatant(i)->getHitPoints() > 0))
        {
            result.append(_model.getCombatant(i));
        }
    }

    qDebug() << "[Battle Dialog] " << result.count() << " living monster combatants found.";

    return result;
}
*/

QRect BattleDialog::viewportRect()
{
    return ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect().toAlignedRect();
}

QPoint BattleDialog::viewportCenter()
{
    QPoint combatantPos = viewportRect().topLeft();
    combatantPos += QPoint(viewportRect().width() / 2, viewportRect().height() / 2);
    return combatantPos;
}

/*
void BattleDialog::sort()
{
    // OPTIMIZE: can this be optimized?
    qDebug() << "[Battle Dialog] sorting combatant widgets";
    clearCombatantWidgets();
    _model.sortCombatants();
    buildCombatantWidgets();
    setActiveCombatant(_model.getActiveCombatant());
    ui->scrollArea->setFocus();
    qDebug() << "[Battle Dialog] combatant widgets sorted";
}

void BattleDialog::next()
{
    BattleDialogModelCombatant* activeCombatant = _model.getActiveCombatant();
    if(!activeCombatant)
        return;

    qDebug() << "[Battle Dialog] Looking for next combatant (current combatant " << activeCombatant << ")...";
    BattleDialogModelCombatant* nextCombatant = getNextCombatant(activeCombatant);

    if(!nextCombatant)
        return;

    if(ui->chkLair->isChecked() && nextCombatant)
    {
        int activeInitiative = activeCombatant->getInitiative();
        int nextInitiative = nextCombatant->getInitiative();
        if((activeInitiative >= 20) && (nextInitiative < 20))
        {
            qDebug() << "[Battle Dialog] Triggering Lair Action request.";
            QMessageBox::information(this, QString("Lair Action"), QString("The legendary creature(s) can now use one of their lair action options. It cannot do so while incapacitated, surprised or otherwise unable to take actions."));
        }
    }

    setActiveCombatant(nextCombatant);
    qDebug() << "[Battle Dialog] ... next combatant found: " << nextCombatant;
}
*/

void BattleDialog::updateMap()
{
    // TODO: load background
    _background = new UnselectedPixmap();

    // TODO: GET MAP IMAGE
    /*
    if((!_background) || (!_model.getMap()))
        return;

    qDebug() << "[Battle Dialog] Updating map " << _model.getMap()->getFileName() << " rect=" << _model.getMapRect().left() << "," << _model.getMapRect().top() << ", " << _model.getMapRect().width() << "x" << _model.getMapRect().height();
    _model.getMap()->initialize();
    QImage battleMap = _model.getMap()->getPublishImage();
    _background->setPixmap((QPixmap::fromImage(battleMap)));
    */

    // NEW ui->graphicsView->fitInView(_model.getMapRect(), Qt::KeepAspectRatio);
    createPrescaledBackground();

    _scene->addItem(_background);
    _background->setEnabled(false);
    _background->setZValue(DMHelper::BattleDialog_Z_Background);

    _scene->setSceneRect(_scene->itemsBoundingRect());
    ui->graphicsView->fitInView(_model.getMapRect(), Qt::KeepAspectRatio);

    _scene->createBattleContents(_background->boundingRect().toRect());

    QPixmap activePmp;
    activePmp.load(":/img/data/active.png");
    _activePixmap = _scene->addPixmap(activePmp);
    _activePixmap->setScale(_model.getGridScale() * _activeScale / ACTIVE_PIXMAP_SIZE);
    _activePixmap->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    _activePixmap->hide();

    QPixmap selectPmp;
    selectPmp.load(":/img/data/selected.png");
    _selectedPixmap = _scene->addPixmap(selectPmp);
    _selectedPixmap->setScale(_model.getGridScale() * _selectedScale / ACTIVE_PIXMAP_SIZE);
    _selectedPixmap->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    _selectedPixmap->hide();

    QPixmap compassPmp;
    compassPmp.load(":/img/data/compass.png");
    _compassPixmap = _scene->addPixmap(compassPmp);
    _compassPixmap->setTransformationMode(Qt::SmoothTransformation);
    qreal scaleW = static_cast<qreal>(_background->pixmap().width()) / static_cast<qreal>(compassPmp.width());
    qreal scaleH = static_cast<qreal>(_background->pixmap().height()) / static_cast<qreal>(compassPmp.height());
    _compassPixmap->setScale(COMPASS_SCALE * qMin(scaleW, scaleH));
    _compassPixmap->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    _compassPixmap->setVisible(_model.getShowCompass());

    // Add icons for existing combatants
    for(int i = 0; i < _model.getCombatantCount(); ++i)
    {
        createCombatantIcon(_model.getCombatant(i));
    }

    updateHighlights();
}

/*
void BattleDialog::setTargetSize(const QSize& targetSize)
{
    int newWidth = widthWindowToBackground(targetSize.width());

    if((newWidth == _targetSize.width()) && (targetSize.height() == _targetSize.height()))
        return;

    _targetSize.setHeight(targetSize.height());
    _targetSize.setWidth(newWidth);

    createPrescaledBackground();
}
*/

void BattleDialog::setGridScale(int gridScale)
{
    if(_scene)
    {
        _model.setGridScale(gridScale);

        qreal scaleFactor;
        qreal oldScaleFactor;

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
                oldScaleFactor = i.value()->scale();
                i.value()->setScale(scaleFactor);
            }
        }

        if(_selectedPixmap)
        {
            scaleFactor = static_cast<qreal>((gridScale)/SELECTED_PIXMAP_SIZE);
            oldScaleFactor = _selectedPixmap->scale();
            _selectedPixmap->setScale(scaleFactor);
            _selectedPixmap->setPos(_selectedPixmap->pos() * scaleFactor/oldScaleFactor);
        }

        if(_activePixmap)
        {
            scaleFactor = static_cast<qreal>((gridScale * _activeScale)/ACTIVE_PIXMAP_SIZE);
            oldScaleFactor = _activePixmap->scale();
            _activePixmap->setScale(scaleFactor);
            _activePixmap->setPos(_activePixmap->pos() * scaleFactor/oldScaleFactor);
        }

        _scene->updateBattleContents();

        ui->graphicsView->update();
    }
}

void BattleDialog::setXOffset(int xOffset)
{
    if(_scene)
    {
        _model.setGridOffsetX(xOffset);
        _scene->updateBattleContents();
        ui->graphicsView->update();
    }
}

void BattleDialog::setYOffset(int yOffset)
{
    if(_scene)
    {
        _model.setGridOffsetY(yOffset);
        _scene->updateBattleContents();
        ui->graphicsView->update();
    }
}

void BattleDialog::setGridVisible(bool gridVisible)
{
    if((_scene) && (gridVisible != _model.getGridOn()))
    {
        _model.setGridOn(gridVisible);
        _scene->setGridVisibility(gridVisible);
        ui->graphicsView->invalidateScene();
    }
}

/*
void BattleDialog::setShowOnDeck(bool showOnDeck)
{
    _showOnDeck = showOnDeck;
    createPrescaledBackground();
}

void BattleDialog::setShowCountdown(bool showCountdown)
{
    _showCountdown = showCountdown;
    createPrescaledBackground();
}
*/

void BattleDialog::setCountdownDuration(int countdownDuration)
{
    _countdownDuration = countdownDuration;
}

void BattleDialog::zoomIn()
{
    setScale(1.1);
}

void BattleDialog::zoomOut()
{
    setScale(0.9);
}

void BattleDialog::zoomFit()
{
    // NEW qreal widthFactor = ((qreal)ui->graphicsView->viewport()->width()) / _scene->width();
    // NEW qreal heightFactor = ((qreal)ui->graphicsView->viewport()->height()) / _scene->height();
    ui->graphicsView->fitInView(_background, Qt::KeepAspectRatio);
    setScale(1.0);
}

void BattleDialog::zoomSelect()
{
    ui->btnZoomSelect->setChecked(true);
    setMapCursor();
}

void BattleDialog::cancelSelect()
{
    ui->btnZoomSelect->setChecked(false);
    setMapCursor();
}

void BattleDialog::keyPressEvent(QKeyEvent * e)
{
    switch(e->key())
    {
        case Qt::Key_Escape:
            cancelSelect();
            return;
        default:
            break;
    }

    QDialog::keyPressEvent(e);
}

void BattleDialog::resizeEvent(QResizeEvent *event)
{
    qDebug() << "[Battle Dialog] resized: " << event->size().width() << "x" << event->size().height();
    if(_background)
    {
        _scene->setSceneRect(_scene->itemsBoundingRect());
        // NEW ui->graphicsView->fitInView(_background, Qt::KeepAspectRatio);
        ui->graphicsView->fitInView(_model.getMapRect(), Qt::KeepAspectRatio);
    }
    QDialog::resizeEvent(event);
}

void BattleDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    qDebug() << "[Battle Dialog] shown (" << isVisible() << ")";
    if(_background)
    {
        _scene->setSceneRect(_scene->itemsBoundingRect());
        // NEW ui->graphicsView->fitInView(_background, Qt::KeepAspectRatio);
        ui->graphicsView->fitInView(_model.getMapRect(), Qt::KeepAspectRatio);
    }
}

void BattleDialog::setCompassVisibility(bool visible)
{
    _model.setShowCompass(visible);
    qDebug() << "[Battle Dialog] show compass checked changed: Visibility=" << visible;
    if(_compassPixmap)
        _compassPixmap->setVisible(visible);
}

void BattleDialog::updateCombatantVisibility()
{
    //_model.setShowAlive(ui->chkShowLiving->isChecked());
    //_model.setShowDead(ui->chkShowDead->isChecked());
    qDebug() << "[Battle Dialog] show alive/dead checked updated: Alive=" << _model.getShowAlive() << ", Dead=" << _model.getShowDead();
    setCombatantVisibility();//_model.getShowAlive(), _model.getShowDead());
}

void BattleDialog::updateEffectLayerVisibility()
{
    //_model.setShowEffects(ui->chkShowEffects->isChecked());
    qDebug() << "[Battle Dialog] show effects checked changed: " << _model.getShowEffects();
    setEffectLayerVisibility(_model.getShowEffects());
}

/*
void BattleDialog::handleContextMenu(BattleDialogModelCombatant* combatant, const QPoint& position)
{
    if(!combatant)
        return;

    qDebug() << "[Battle Dialog] context menu opened for " << combatant->getName() << " at " << position.x() << "x" << position.y();

    _contextMenuCombatant = combatant;

    QMenu* contextMenu = new QMenu(ui->scrollArea);

    QAction* activateItem = new QAction(QString("Activate"), contextMenu);
    connect(activateItem,SIGNAL(triggered()),this,SLOT(activateCombatant()));
    contextMenu->addAction(activateItem);

    QAction* removeItem = new QAction(QString("Remove"), contextMenu);
    connect(removeItem,SIGNAL(triggered()),this,SLOT(removeCombatant()));
    contextMenu->addAction(removeItem);

    contextMenu->exec(position);
    delete contextMenu;

    _contextMenuCombatant = nullptr;
}
*/

/*
void BattleDialog::handleBattleComplete()
{
    QMessageBox::StandardButton result = QMessageBox::critical(this, QString("Confirm Battle Complete"), QString("Are you sure you wish to complete this battle? All changes will be discarded."), QMessageBox::Yes | QMessageBox::No);

    if(result == QMessageBox::Yes)
    {
        qDebug() << "[Battle Dialog] battle completed";
        emit battleComplete();
    }
    else
    {
        qDebug() << "[Battle Dialog] battle completed request denied";
    }
}
*/

void BattleDialog::handleSelectionChanged()
{
    if(!_scene)
        return;

    QList<QGraphicsItem *> selectedList = _scene->selectedItems();
    if(selectedList.isEmpty())
    {
        setSelectedCombatant(nullptr);
        return;
    }

    QGraphicsPixmapItem* pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(selectedList.first());
    if(pixmapItem)
    {
        BattleDialogModelCombatant* combatant = _combatantIcons.key(pixmapItem, nullptr);
        setSelectedCombatant(combatant);
    }
    else
    {
        QAbstractGraphicsShapeItem* shapeItem = dynamic_cast<QAbstractGraphicsShapeItem*>(selectedList.first());
        if((shapeItem)&&(shapeItem->data(0).toInt() >= 1))
        {
            handleEffectChanged(shapeItem);
        }
    }
}

void BattleDialog::handleEffectChanged(QAbstractGraphicsShapeItem* effect)
{
    qDebug() << "[Battle Dialog] Handle effect changed for " << effect;

    for(QGraphicsPixmapItem* item : _combatantIcons.values())
    {
        if(item)
        {
            // OPTIMIZE: Optimize to only remove effects if not still relevant
            removeEffectsFromItem(item);

            if(isItemInEffect(item, effect))
            {
                applyEffectToItem(item, effect);
            }
        }
    }
}

void BattleDialog::handleCombatantMoved(BattleDialogModelCombatant* combatant)
{
    if((!_scene) || (!combatant))
        return;

    qDebug() << "[Battle Dialog] Handle Combatant Moved for " << combatant;

    QGraphicsPixmapItem* item = _combatantIcons.value(combatant, nullptr);
    if(!item)
        return;

    removeEffectsFromItem(item);

    QList<QGraphicsItem*> effects = _scene->getEffectItems();

    for(QGraphicsItem* effect : effects)
    {
        if(effect)
        {
            QAbstractGraphicsShapeItem* abstractEffect = dynamic_cast<QAbstractGraphicsShapeItem*>(effect);
            if(isItemInEffect(item, abstractEffect))
            {
                applyEffectToItem(item, abstractEffect);
                return;
            }
        }
    }
}

/*
void BattleDialog::handleApplyEffect(QAbstractGraphicsShapeItem* effect)
{
    QList<BattleDialogModelCombatant*> combatantList;

    for(QGraphicsPixmapItem* item : _combatantIcons.values())
    {
        if(item)
        {
            if((effect) && (effect->contains(effect->mapFromScene(item->pos()))))
            {
                BattleDialogModelCombatant* combatant = _combatantIcons.key(item, nullptr);
                if(combatant)
                    combatantList.append(combatant);
            }
        }
    }

    if(combatantList.isEmpty())
    {
        QMessageBox::information(this, QString("Apply Effect"), QString("No target combatants were found for the selected effect."));
        return;
    }

    DiceRollDialogCombatants* dlg = new DiceRollDialogCombatants(Dice(1,20,0), combatantList, 15, this);
    connect(dlg,SIGNAL(selectCombatant(BattleDialogModelCombatant*)),this,SLOT(setSelectedCombatant(BattleDialogModelCombatant*)));
    connect(dlg,SIGNAL(combatantChanged(BattleDialogModelCombatant*)),this,SLOT(updateCombatantWidget(BattleDialogModelCombatant*)));
    dlg->fireAndForget();
}
*/

/*
void BattleDialog::removeCombatant()
{
    if(!_contextMenuCombatant)
        return;

    qDebug() << "[Battle Dialog] removing combatant " << _contextMenuCombatant->getName();

    // Check the active combatant highlight
    if(_contextMenuCombatant == _model.getActiveCombatant())
    {
        if(_model.getCombatantCount() <= 1)
        {
            _model.setActiveCombatant(nullptr);
            qDebug() << "[Battle Dialog] removed combatant has highlight, highlight removed";
        }
        else
        {
            next();
        }
    }

    // Check the selection highlight
    if(_contextMenuCombatant == _selectedCombatant)
    {
        setSelectedCombatant(nullptr);
    }

    // Find the index of the removed item
    int i = _model.getCombatantList().indexOf(_contextMenuCombatant);

    // Delete the widget for the combatant
    QLayoutItem *child = _combatantLayout->takeAt(i);
    if(child != nullptr)
    {
        qDebug() << "[Battle Dialog] deleting combatant widget: " << reinterpret_cast<std::uintptr_t>(child->widget());
        child->widget()->deleteLater();
        delete child;
    }

    // Remove the combatant
    BattleDialogModelCombatant* combatant = _model.removeCombatant(i);
    qDebug() << "[Battle Dialog] removing combatant from list " << combatant->getName();
    if(combatant)
    {
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
*/

/*
void BattleDialog::activateCombatant()
{
    if(!_contextMenuCombatant)
        return;

    qDebug() << "[Battle Dialog] activating combatant " << _contextMenuCombatant->getName();
    setActiveCombatant(_contextMenuCombatant);
}
*/

void BattleDialog::setSelectedCombatant(BattleDialogModelCombatant* selected)
{
    if(selected == _selectedCombatant)
        return;

//    CombatantWidget* combatantWidget = getWidgetFromCombatant(_selectedCombatant);
//    if(combatantWidget)
//    {
//        qDebug() << "[Battle Dialog] removing selected flag from widget " << reinterpret_cast<std::uintptr_t>(combatantWidget);
//        combatantWidget->setSelected(false);
        if(_combatantIcons.value(_selectedCombatant, nullptr))
            _combatantIcons.value(_selectedCombatant, nullptr)->setZValue(DMHelper::BattleDialog_Z_Combatant);
//    }

    QGraphicsPixmapItem* selectedItem = nullptr;
    if(selected)
    {
//        combatantWidget = getWidgetFromCombatant(selected);
        selectedItem = _combatantIcons.value(selected, nullptr);
//        if(combatantWidget)
//        {
//            qDebug() << "[Battle Dialog] adding selected flag to widget " << reinterpret_cast<std::uintptr_t>(combatantWidget);
//            combatantWidget->setSelected(true);
//            ui->scrollArea->ensureWidgetVisible(combatantWidget);
            if(selectedItem)
                selectedItem->setZValue(DMHelper::BattleDialog_Z_SelectedCombatant);
//        }
    }

    if(_selectedPixmap)
    {
        if(selectedItem)
        {
            if(selected)
            {
                _selectedScale = selected->getSizeFactor();
                _selectedPixmap->setScale(_model.getGridScale() * _selectedScale / SELECTED_PIXMAP_SIZE);
            }
            moveRectToPixmap(_selectedPixmap, selectedItem);
            _selectedPixmap->show();
        }
        else
        {
            _selectedPixmap->hide();
        }
    }

    _selectedCombatant = selected;
}

/*
void BattleDialog::updateCombatantWidget(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    CombatantWidget* widget = _combatantWidgets.value(combatant);
    if(!widget)
        return;

    widget->updateData();
}
*/

/*
void BattleDialog::togglePublishing(bool publishing)
{
    qDebug() << "[Battle Dialog] publishing toggled (" << publishing << ")";
    _publishing = publishing;
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

void BattleDialog::publishImage()
{
    if(_publishing)
    {
        if(!_publishTimer->isActive())
        {
            executePublishImage();
            // OPTIMIZE: optimize this to be faster, doing only changes?
            _publishTimer->start(25);
            qDebug() << "[Battle Dialog] publish timer activated";
        }
    }
}

void BattleDialog::executePublishImage()
{
    qDebug() << "[Battle Dialog] publishing image";

    QImage pub;
    getImageForPublishing(pub);
    emit publishImage(pub);
}

void BattleDialog::executeAnimateImage()
{
    QImage pub;
    getImageForPublishing(pub);
    emit animateImage(pub);
}
*/

void BattleDialog::updateHighlights()
{
    QGraphicsPixmapItem* item;

    if(_activePixmap)
    {
        item = _combatantIcons.value(_model.getActiveCombatant(), nullptr);
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

    if(_selectedPixmap)
    {
        item = _combatantIcons.value(_selectedCombatant, nullptr);
        if(item)
        {
            moveRectToPixmap(_selectedPixmap, item);
        }
    }
}

void BattleDialog::countdownTimerExpired()
{
    if(_countdown > 0)
    {
        _countdown -= (static_cast<qreal>(_countdownFrame.height() - 10) / static_cast<qreal>(_countdownDuration)) * COUNTDOWN_TIMER;
        if(_countdown <= 0)
        {
            _countdown = 0;
            _countdownTimer->stop();
        }
    }

    updateCountdownText();
}

void BattleDialog::updateCountdownText()
{
    ui->edtCountdown->setText(QString::number(_countdown));

    int halfMaxVal = (_countdownFrame.height() - 10) / 2;
    if(halfMaxVal <= 0)
        return;

    if(_countdown > halfMaxVal)
    {
        _countdownColor.setRed(196 - (196 * (_countdown - halfMaxVal) / halfMaxVal));
        _countdownColor.setGreen(196);
    }
    else
    {
        _countdownColor.setRed(196);
        _countdownColor.setGreen(196 * _countdown / halfMaxVal);
    }

    QString style = "color: " + _countdownColor.name() + ";";
    ui->edtCountdown->setStyleSheet(style);
}

void BattleDialog::createPrescaledBackground()
{
/*
     if((!_model.getMap()) || (!ui->btnPublish->isEnabled()) || (!ui->btnPublish->isChecked()))
        return;
*/

    QRect sourceRect;
    QRect viewportScene = ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect().toAlignedRect();
    QRect sceneRect = ui->graphicsView->sceneRect().toRect();
    sourceRect = viewportScene.intersected(sceneRect);

    // TODO - Add image
    /*
    QImage battleMap = _model.getMap()->getPublishImage().copy(sourceRect);
    _prescaledBackground = battleMap.scaled(_targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    */
}

void BattleDialog::handleRubberBandChanged(QRect rubberBandRect, QPointF fromScenePoint, QPointF toScenePoint)
{
    Q_UNUSED(fromScenePoint);
    Q_UNUSED(toScenePoint);

    if(!ui->btnZoomSelect->isChecked())
        return;

    if(rubberBandRect.isNull())
    {
        ui->graphicsView->fitInView(ui->graphicsView->mapToScene(_rubberBandRect).boundingRect(), Qt::KeepAspectRatio);
        _scale = ui->graphicsView->transform().m11();
    }
    else
    {
        _rubberBandRect = rubberBandRect;
    }
}

void BattleDialog::setCombatantVisibility()
{
    for(int i = 0; i < _model.getCombatantCount(); ++i)
    {
        // TODO: Handle visibility
        bool vis = true;//(_model.getCombatant(i)->getHitPoints() > 0) ? aliveVisible : deadVisible;

        /*
        if(widgetsIncluded)
        {
            QWidget* widget = _combatantLayout->itemAt(i)->widget();
            if(widget)
            {
                widget->setVisible(vis);
            }
        }
        */

        QGraphicsPixmapItem* item = _combatantIcons.value(_model.getCombatant(i));
        if(item)
            item->setVisible(vis);

        if((!vis) && (_model.getCombatant(i) == _selectedCombatant))
            setSelectedCombatant(nullptr);

        // Set the visibility of the active rect
        if((_activePixmap) && (_model.getCombatant(i) == _model.getActiveCombatant()))
            _activePixmap->setVisible(vis);
    }
}

void BattleDialog::setEffectLayerVisibility(bool visibility)
{
    if(_scene)
    {
        _scene->setEffectVisibility(visibility);
        ui->graphicsView->invalidateScene();
    }
}

/*
void BattleDialog::setPublishVisibility(bool publish)
{
    QList<BattleDialogModelCombatant*> combatantList = _model.getCombatantList();

    for(BattleDialogModelCombatant* combatant : combatantList)
    {
        if(combatant)
        {
            if((!combatant->getShown())||(!combatant->getKnown()))
            {
                QGraphicsPixmapItem* item = _combatantIcons.value(combatant);
                if(item)
                {
                    item->setVisible(!publish);

                    if((_selectedPixmap) && (combatant == _selectedCombatant))
                        _selectedPixmap->setVisible(!publish);

                    if((_activePixmap) && (combatant == _model.getActiveCombatant()))
                        _activePixmap->setVisible(!publish);
                }
            }
        }
    }
}
*/

void BattleDialog::setMapCursor()
{
    _rubberBandRect = QRect();

    if(ui->btnZoomSelect->isChecked())
        ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/crosshair.png").scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    else
        ui->graphicsView->viewport()->unsetCursor();
}

void BattleDialog::setScale(qreal s)
{
    _scale = s;
    ui->graphicsView->scale(s,s);
    createPrescaledBackground();
    setMapCursor();
    storeViewRect();
}

void BattleDialog::storeViewRect()
{
    _model.setMapRect(ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect().toAlignedRect());
}

/*
CombatantWidget* BattleDialog::createCombatantWidget(BattleDialogModelCombatant* combatant)
{
    CombatantWidget* newWidget = nullptr;

    if(_combatantWidgets.contains(combatant))
    {
        newWidget = _combatantWidgets.value(combatant);
        qDebug() << "[Battle Dialog] found widget for combatant " << combatant->getName() << ": " << reinterpret_cast<std::uintptr_t>(newWidget);
        return newWidget;
    }

    switch(combatant->getType())
    {
        case DMHelper::CombatantType_Character:
        {
            BattleDialogModelCharacter* character = dynamic_cast<BattleDialogModelCharacter*>(combatant);
            if(character)
            {
                qDebug() << "[Battle Dialog] creating character widget for " << character->getName();
                newWidget = new CharacterWidget(character, ui->scrollAreaWidgetContents);
                connect(dynamic_cast<CharacterWidget*>(newWidget), SIGNAL(clicked(int)),this,SIGNAL(characterSelected(int)));
                connect(newWidget, SIGNAL(contextMenu(BattleDialogModelCombatant*,QPoint)), this, SLOT(handleContextMenu(BattleDialogModelCombatant*,QPoint)));
            }
            break;
        }
        case DMHelper::CombatantType_Monster:
        {
            BattleDialogModelMonsterBase* monster = dynamic_cast<BattleDialogModelMonsterBase*>(combatant);
            if(monster)
            {
                qDebug() << "[Battle Dialog] creating monster widget for " << monster->getName();
                newWidget = new WidgetMonster(ui->scrollAreaWidgetContents);
                WidgetMonsterInternal* widgetInternals = new WidgetMonsterInternal(monster, dynamic_cast<WidgetMonster*>(newWidget));
                connect(widgetInternals, SIGNAL(clicked(const QString&)),this,SIGNAL(monsterSelected(const QString&)));
                connect(widgetInternals, SIGNAL(contextMenu(BattleDialogModelCombatant*,QPoint)), this, SLOT(handleContextMenu(BattleDialogModelCombatant*,QPoint)));
                connect(widgetInternals, SIGNAL(hitPointsChanged()), this, SLOT(updateCombatantVisibility()));
                connect(dynamic_cast<WidgetMonster*>(newWidget), SIGNAL(isShownChanged(bool)), monster, SLOT(setShown(bool)));
                connect(dynamic_cast<WidgetMonster*>(newWidget), SIGNAL(isKnownChanged(bool)), monster, SLOT(setKnown(bool)));
            }
            break;
        }
        default:
            qDebug() << "[Battle Dialog] Unknown combatant type found in battle! Type: " << combatant->getType() << " Name: " << combatant->getName();
            break;
    }

    if(newWidget)
    {
        newWidget->installEventFilter(this);
        _combatantWidgets.insert(combatant, newWidget);
        qDebug() << "[Battle Dialog] new widget inserted and event filter established: " << reinterpret_cast<std::uintptr_t>(newWidget);
    }

    return newWidget;
}

void BattleDialog::clearCombatantWidgets()
{
    qDebug() << "[Battle Dialog] deleting combatant widgets";
    QLayoutItem *child;
    while ((child = _combatantLayout->takeAt(0)) != nullptr)
    {
        delete child;
    }
}

void BattleDialog::buildCombatantWidgets()
{
    qDebug() << "[Battle Dialog] building combatant widgets. count: " << _model.getCombatantCount();

    if(_model.getCombatantCount() == 0)
        return;

    for(int i = 0; i < _model.getCombatantCount(); ++i)
    {
        CombatantWidget* newWidget = createCombatantWidget(_model.getCombatant(i));

        if(newWidget)
        {
            _combatantLayout->addWidget(newWidget);
            newWidget->setActive(false);
        }
    }

    setCombatantVisibility(_model.getShowAlive(), _model.getShowDead(), true);
    if(_model.getActiveCombatant())
        setActiveCombatant(_model.getActiveCombatant());
    else
        setActiveCombatant(getFirstLivingCombatant());
}

void BattleDialog::reorderCombatantWidgets()
{
    qDebug() << "[Battle Dialog] resetting combatant widget order";
    clearCombatantWidgets();
    for(int i = 0; i < _model.getCombatantCount(); ++i)
    {
        CombatantWidget* widget = _combatantWidgets.value(_model.getCombatant(i));
        if(widget)
        {
            _combatantLayout->addWidget(widget);
        }
    }
}
*/

void BattleDialog::setActiveCombatant(BattleDialogModelCombatant* active)
{
    /*
    CombatantWidget* combatantWidget = getWidgetFromCombatant(_model.getActiveCombatant());
    if(combatantWidget)
    {
        qDebug() << "[Battle Dialog] removing active flag from widget " << reinterpret_cast<std::uintptr_t>(combatantWidget);
        combatantWidget->setActive(false);
    }

    combatantWidget = getWidgetFromCombatant(active);
    if(combatantWidget)
    {
        qDebug() << "[Battle Dialog] adding active flag to widget " << reinterpret_cast<std::uintptr_t>(combatantWidget);
        combatantWidget->setActive(true);
        ui->scrollArea->ensureWidgetVisible(combatantWidget);
    }
    */

    if(active)
        _activeScale = active->getSizeFactor();

    if(_activePixmap)
    {
        QGraphicsPixmapItem* item = _combatantIcons.value(active, nullptr);
        if(item)
        {
            _activePixmap->setScale(_model.getGridScale() * _activeScale / ACTIVE_PIXMAP_SIZE);

            moveRectToPixmap(_activePixmap, item);
            _activePixmap->setVisible(item->isVisible());

            _countdownTimer->start(static_cast<int>(COUNTDOWN_TIMER * 1000));
            _countdown = _countdownFrame.height() - 10;
            updateCountdownText();
        }
        else
        {
            _activePixmap->hide();
        }
    }

    _model.setActiveCombatant(active);
}

void BattleDialog::createCombatantIcon(BattleDialogModelCombatant* combatant)
{
    if((combatant) && (_scene) && (_background))
    {
        // TODO: get combatant icon
        /*
        QPixmap pix = combatant->getIconPixmap(DMHelper::PixmapSize_Battle);
        QGraphicsPixmapItem* pixmapItem = new UnselectedPixmap(pix, combatant);
        _scene->addItem(pixmapItem);
        pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true);
        pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
        pixmapItem->setZValue(DMHelper::BattleDialog_Z_Combatant);
        pixmapItem->setPos(combatant->getPosition());
        pixmapItem->setOffset(-static_cast<qreal>(pix.width())/2.0, -static_cast<qreal>(pix.height())/2.0);
        int sizeFactor = combatant->getSizeFactor();
        qreal scaleFactor = (static_cast<qreal>(_model.getGridScale()-2)) * static_cast<qreal>(sizeFactor) / static_cast<qreal>(qMax(pix.width(),pix.height()));
        pixmapItem->setScale(scaleFactor);

        qDebug() << "[Battle Dialog] combatant icon added " << combatant->getName() << ", scale " << scaleFactor;

        qreal gridSize = (static_cast<qreal>(_model.getGridScale())) / scaleFactor;
        qreal gridOffset = gridSize * static_cast<qreal>(sizeFactor) / 2.0;
        QGraphicsRectItem* rect = new QGraphicsRectItem(0, 0, gridSize * static_cast<qreal>(sizeFactor), gridSize * static_cast<qreal>(sizeFactor));
        rect->setPos(-gridOffset, -gridOffset);
        rect->setData(BattleDialogItemChild_Index, BattleDialogItemChild_Area);
        rect->setParentItem(pixmapItem);
        rect->setVisible(false);
        qDebug() << "[Battle Dialog] created " << pixmapItem << " with area child " << rect;

        applyPersonalEffectToItem(pixmapItem);

        _combatantIcons.insert(combatant, pixmapItem);
        */

        connect(combatant, SIGNAL(combatantMoved(BattleDialogModelCombatant*)), this, SLOT(handleCombatantMoved(BattleDialogModelCombatant*)), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
        connect(combatant, SIGNAL(combatantMoved(BattleDialogModelCombatant*)), this, SLOT(updateHighlights()), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
    }
}

void BattleDialog::relocateCombatantIcon(QGraphicsPixmapItem* icon)
{
    if(!icon)
        return;

    QPoint mapPos = icon->pos().toPoint() + _model.getPreviousMapRect().topLeft();
    if(_model.getMapRect().contains(mapPos))
    {
        icon->setPos(mapPos - _model.getMapRect().topLeft());
    }
    else
    {
        icon->setPos(10, 10);
    }
}

/*
QWidget* BattleDialog::findCombatantWidgetFromPosition(const QPoint& position) const
{
    qDebug() << "[Battle Dialog] searching for widget from position " << position.x() << "x" << position.y() << "...";
    QWidget* widget = ui->scrollAreaWidgetContents->childAt(position);

    if(widget)
    {
        while((widget->parentWidget() != ui->scrollAreaWidgetContents) && (widget->parentWidget() != nullptr))
        {
            widget = widget->parentWidget();
        }

        if(widget->parentWidget() == nullptr)
        {
            qDebug() << "[Battle Dialog] ...widget not found";
            return nullptr;
        }
    }

    qDebug() << "[Battle Dialog] ...widget found: " << reinterpret_cast<std::uintptr_t>(widget);
    return widget;
}

CombatantWidget* BattleDialog::getWidgetFromCombatant(BattleDialogModelCombatant* combatant) const
{
    if(!combatant)
        return nullptr;

    int pos = _model.getCombatantList().indexOf(combatant);
    qDebug() << "[Battle Dialog] finding widget for combatant " << combatant << " at " << pos;
    if((pos >= 0) && (pos < _combatantLayout->count()))
    {
        return dynamic_cast<CombatantWidget*>(_combatantLayout->itemAt(pos)->widget());
    }
    else
    {
        return nullptr;
    }
}
*/

void BattleDialog::moveRectToPixmap(QGraphicsItem* rectItem, QGraphicsPixmapItem* pixmapItem)
{
    if((!rectItem) || (!pixmapItem))
        return;

    QRect itemRect = rectItem->boundingRect().toRect();
    rectItem->setPos(pixmapItem->x() + (((pixmapItem->pixmap().width() / 2) + pixmapItem->offset().x()) * pixmapItem->scale()) - (itemRect.width() * rectItem->scale() / 2),
                     pixmapItem->y() + (((pixmapItem->pixmap().height() / 2) + pixmapItem->offset().y()) * pixmapItem->scale()) - (itemRect.height() * rectItem->scale() / 2));
}

/*
BattleDialogModelCombatant* BattleDialog::getNextCombatant(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return nullptr;

    int nextHighlight = _model.getCombatantList().indexOf(combatant);

    if(_combatantLayout->count() <= 1)
    {
        qDebug() << "[Battle Dialog] No next combatant possible, at most one combatant widgets found.";
        return nullptr;
    }

    do
    {
        if(++nextHighlight >= _model.getCombatantCount())
        {
            nextHighlight = 0;
        }
    } while( ( (_model.getCombatant(nextHighlight)->getHitPoints() <= 0) ||
               (!_model.getCombatant(nextHighlight)->getKnown()) ) &&
             (_model.getCombatant(nextHighlight) != _model.getActiveCombatant()));

    return _model.getCombatant(nextHighlight);
}
*/

/*
void BattleDialog::getImageForPublishing(QImage& imageForPublishing)
{
    // Set up the target image for publishing
    QSize publishSize = _targetSize;
    publishSize.rwidth() = widthBackgroundToWindow(publishSize.width());
    imageForPublishing = QImage(publishSize, QImage::Format_ARGB32);

    QPainter painter(&imageForPublishing);

    // Draw the background image
    painter.drawImage(0, 0, _prescaledBackground);

    // Draw the contents of the battle dialog in publish mode
    _background->setVisible(false);
    setPublishVisibility(true);
    QRect viewportRect = ui->graphicsView->viewport()->rect();
    QRect sceneViewportRect = ui->graphicsView->mapFromScene(ui->graphicsView->sceneRect()).boundingRect();
    QRect sourceRect = viewportRect.intersected(sceneViewportRect);
    ui->graphicsView->render(&painter, QRectF(QPointF(0,0),_targetSize), sourceRect);

    setPublishVisibility(false);
    _background->setVisible(true);

    // Draw the active combatant image on top
    if(_model.getActiveCombatant())
    {
        BattleDialogModelCombatant* nextCombatant = nullptr;

        if(_showOnDeck)
        {
            QPixmap pmp;
            if(_model.getActiveCombatant()->getShown())
                pmp = _model.getActiveCombatant()->getIconPixmap(DMHelper::PixmapSize_Animate);
            else
                pmp = ScaledPixmap::defaultPixmap()->getPixmap(DMHelper::PixmapSize_Animate);

            painter.drawImage(_prescaledBackground.width(), 0, _combatantFrame);
            int dx = qMax(5, (_combatantFrame.width()-pmp.width())/2);
            int dy = qMax(5, (_combatantFrame.height()-pmp.height())/2);
            painter.drawPixmap(_prescaledBackground.width() + dx, dy, pmp);
            nextCombatant = getNextCombatant(_model.getActiveCombatant());
        }

        if(_showCountdown)
        {
            int xPos = _prescaledBackground.width();
            if(_showOnDeck)
                xPos += _combatantFrame.width();
            if(_countdown > 0)
            {
                painter.setBrush(QBrush(_countdownColor));
                painter.drawRect(xPos + 5, _countdownFrame.height() - _countdown - 5, 10, _countdown);
            }
            painter.drawImage(xPos, 0, _countdownFrame);
        }

        if((_showOnDeck) && (nextCombatant))
        {
            QPixmap nextPmp;
            if(nextCombatant->getShown())
                nextPmp = nextCombatant->getIconPixmap(DMHelper::PixmapSize_Animate);
            else
                nextPmp = ScaledPixmap::defaultPixmap()->getPixmap(DMHelper::PixmapSize_Animate);
            painter.drawImage(_prescaledBackground.width(), DMHelper::PixmapSizes[DMHelper::PixmapSize_Animate][1] + 10, _combatantFrame);
            int dx = qMax(5, (_combatantFrame.width()-nextPmp.width())/2);
            int dy = qMax(5, (_combatantFrame.height()-nextPmp.height())/2);
            painter.drawPixmap(_prescaledBackground.width() + dx, _combatantFrame.height() + dy, nextPmp);
        }
    }
}
*/

/*
void BattleDialog::replaceBattleMap()
{
    qDebug() << "[Battle Dialog] New map detected";

    // Clean up the old map
    _scene->clearBattleContents();
    delete _background; _background = nullptr;
    delete _activePixmap; _activePixmap = nullptr;
    delete _selectedPixmap; _selectedPixmap = nullptr;
    delete _compassPixmap; _compassPixmap = nullptr;

    // Clean up any existing icons
    qDeleteAll(_combatantIcons.values());
    _combatantIcons.clear();

    //ui->btnReloadMap->setEnabled(_model.getMap() != nullptr);
    //ui->btnPublish->setEnabled(_model.getMap() != nullptr);
    if(!_model.getMap())
        return;

    // TODO: load background
    _background = new UnselectedPixmap();

    updateMap();

    _scene->addItem(_background);
    _background->setEnabled(false);
    _background->setZValue(DMHelper::BattleDialog_Z_Background);

    _scene->setSceneRect(_scene->itemsBoundingRect());
    ui->graphicsView->fitInView(_model.getMapRect(), Qt::KeepAspectRatio);

    _scene->createBattleContents(_background->boundingRect().toRect());

    QPixmap activePmp;
    activePmp.load(":/img/data/active.png");
    _activePixmap = _scene->addPixmap(activePmp);
    _activePixmap->setScale(_model.getGridScale() * _activeScale / ACTIVE_PIXMAP_SIZE);
    _activePixmap->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    _activePixmap->hide();

    QPixmap selectPmp;
    selectPmp.load(":/img/data/selected.png");
    _selectedPixmap = _scene->addPixmap(selectPmp);
    _selectedPixmap->setScale(_model.getGridScale() * _selectedScale / ACTIVE_PIXMAP_SIZE);
    _selectedPixmap->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    _selectedPixmap->hide();

    QPixmap compassPmp;
    compassPmp.load(":/img/data/compass.png");
    _compassPixmap = _scene->addPixmap(compassPmp);
    _compassPixmap->setTransformationMode(Qt::SmoothTransformation);
    qreal scaleW = static_cast<qreal>(_background->pixmap().width()) / static_cast<qreal>(compassPmp.width());
    qreal scaleH = static_cast<qreal>(_background->pixmap().height()) / static_cast<qreal>(compassPmp.height());
    _compassPixmap->setScale(COMPASS_SCALE * qMin(scaleW, scaleH));
    _compassPixmap->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    _compassPixmap->setVisible(_model.getShowCompass());

    // Add icons for existing combatants
    for(int i = 0; i < _model.getCombatantCount(); ++i)
    {
        createCombatantIcon(_model.getCombatant(i));
    }

    updateHighlights();

    //qDebug() << "[Battle Dialog] map set to new image (" << _model.getMap()->getFileName() << ")";

}

void BattleDialog::resizeBattleMap()
{
    if(!_model.getMap())
        return;

    qDebug() << "[Battle Dialog] Update of same map detected";

    //ui->btnReloadMap->setEnabled(_model.getMap() != nullptr);
    //ui->btnPublish->setEnabled(_model.getMap() != nullptr);

    updateMap();

    //_scene->updateBattleContents();
    _scene->resizeBattleContents(_background->boundingRect().toRect());
    _scene->setSceneRect(_scene->itemsBoundingRect());
    // NEW ui->graphicsView->fitInView(_background, Qt::KeepAspectRatio);
    ui->graphicsView->fitInView(_model.getMapRect(), Qt::KeepAspectRatio);

    // try to move the icons with the map
    QList<QGraphicsPixmapItem*> iconList = _combatantIcons.values();
    for(int i = 0; i < iconList.count(); ++i)
    {
        relocateCombatantIcon(iconList.at(i));
    }
}
*/

/*
int BattleDialog::widthWindowToBackground(int windowWidth)
{
    int result = windowWidth;

    if(_showOnDeck)
        result -= _combatantFrame.width();

    if(_showCountdown)
        result -= _countdownFrame.width();

    return result;
}

int BattleDialog::widthBackgroundToWindow(int backgroundWidth)
{
    int result = backgroundWidth;

    if(_showOnDeck)
        result += _combatantFrame.width();

    if(_showCountdown)
        result += _countdownFrame.width();

    return result;
}
*/

bool BattleDialog::isItemInEffect(QGraphicsPixmapItem* item, QAbstractGraphicsShapeItem* effect)
{
    if((!item) || (!effect))
        return false;

    //return item->collidesWithItem(effect);

    if(item->childItems().count() > 0)
    {
        for(QGraphicsItem* childItem : item->childItems())
        {
            if((childItem) && (childItem->data(BattleDialogItemChild_Index).toInt() == BattleDialogItemChild_Area))
            {
                return childItem->collidesWithItem(effect);
            }
        }
    }

    return item->collidesWithItem(effect);
}

void BattleDialog::removeEffectsFromItem(QGraphicsPixmapItem* item)
{
    if(!item)
        return;

    qDebug() << "[Battle Dialog] Removing effects from item " << item;

    // Remove any existing effects on this combatant
    for(QGraphicsItem* childItem : item->childItems())
    {
        if((childItem) && (childItem->data(BattleDialogItemChild_Index).toInt() == BattleDialogItemChild_AreaEffect))
        {
            qDebug() << "[Battle Dialog] Deleting child item: " << childItem;
            childItem->setParentItem(nullptr);
            delete childItem;
        }
    }
}

void BattleDialog::applyEffectToItem(QGraphicsPixmapItem* item, QAbstractGraphicsShapeItem* effect)
{
    if((!item) || (!effect))
        return;

    QRect itemRect = item->boundingRect().toRect();
    int maxSize = qMax(itemRect.width(), itemRect.height());

    QGraphicsEllipseItem* effectItem = new QGraphicsEllipseItem(-maxSize/2, -maxSize/2, maxSize, maxSize);
    QColor ellipseColor = effect->pen().color();
    effectItem->setPen(QPen(ellipseColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    ellipseColor.setAlpha(128);
    effectItem->setBrush(QBrush(ellipseColor));
    effectItem->setFlag(QGraphicsItem::ItemNegativeZStacksBehindParent);
    effectItem->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    effectItem->setData(BattleDialogItemChild_Index, BattleDialogItemChild_AreaEffect);
    effectItem->setParentItem(item);

    qDebug() << "[Battle Dialog] applying effects to item. Item: " << item << ", effect: " << effect << " with effect item " << effectItem;

}

void BattleDialog::applyPersonalEffectToItem(QGraphicsPixmapItem* item)
{
    if(!item)
        return;

    // TODO: Add personal effects
}
