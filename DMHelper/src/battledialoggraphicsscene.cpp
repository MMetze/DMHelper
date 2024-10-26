#include "battledialoggraphicsscene.h"
#include "battledialogeffectsettings.h"
#include "battledialogmodel.h"
#include "battledialogmodeleffect.h"
#include "battledialogmodeleffectfactory.h"
#include "battledialogmodelmonsterclass.h"
#include "monsterclass.h"
#include "unselectedpixmap.h"
#include "layertokens.h"
#include "layergrid.h"
#include <QMenu>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QtMath>
#include <QMimeData>
#include <QMimeDatabase>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDebug>

// TODO: adjust grid offsets to really match resized battle contents.

// Uncomment this to log non-movement mouse actions
//#define BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS

// Uncomment this to log all mouse movement actions
//#define BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE

BattleDialogGraphicsScene::BattleDialogGraphicsScene(QObject *parent) :
    CameraScene(parent),
    _contextMenuItem(nullptr),
    _model(nullptr),
    _mouseDown(false),
    _mouseDownPos(),
    _mouseDownItem(nullptr),
    _mouseHoverItem(nullptr),
    _previousRotation(0.0),
    _isRotation(false),
    _commandPosition(DMHelper::INVALID_POINT),
    _spaceDown(false),
    _inputMode(-1),
    _pointerPixmapItem(nullptr),
    _pointerVisible(false),
    _pointerPixmap(),
    _selectedIcon(),
    _selectionCount(0),
    _distanceMouseHandler(*this),
    _freeDistanceMouseHandler(*this),
    _pointerMouseHandler(*this),
    _rawMouseHandler(*this),
    _cameraMouseHandler(*this),
    _combatantMouseHandler(*this),
    _mapsMouseHandler(*this)
{
    connect(&_distanceMouseHandler, &BattleDialogGraphicsSceneMouseHandlerDistance::distanceChanged, this, &BattleDialogGraphicsScene::distanceChanged);
    connect(&_freeDistanceMouseHandler, &BattleDialogGraphicsSceneMouseHandlerFreeDistance::distanceChanged, this, &BattleDialogGraphicsScene::distanceChanged);
    connect(&_distanceMouseHandler, &BattleDialogGraphicsSceneMouseHandlerDistance::distanceItemChanged, this, &BattleDialogGraphicsScene::distanceItemChanged);
    connect(&_freeDistanceMouseHandler, &BattleDialogGraphicsSceneMouseHandlerFreeDistance::distanceItemChanged, this, &BattleDialogGraphicsScene::distanceItemChanged);

    connect(&_pointerMouseHandler, &BattleDialogGraphicsSceneMouseHandlerPointer::pointerMoved, this, &BattleDialogGraphicsScene::pointerMove);

    connect(&_rawMouseHandler, &BattleDialogGraphicsSceneMouseHandlerRaw::rawMousePress, this, &BattleDialogGraphicsScene::battleMousePress);
    connect(&_rawMouseHandler, &BattleDialogGraphicsSceneMouseHandlerRaw::rawMouseMove, this, &BattleDialogGraphicsScene::battleMouseMove);
    connect(&_rawMouseHandler, &BattleDialogGraphicsSceneMouseHandlerRaw::rawMouseRelease, this, &BattleDialogGraphicsScene::battleMouseRelease);

    connect(&_mapsMouseHandler, &BattleDialogGraphicsSceneMouseHandlerMaps::mapMousePress, this, &BattleDialogGraphicsScene::mapMousePress);
    connect(&_mapsMouseHandler, &BattleDialogGraphicsSceneMouseHandlerMaps::mapMouseMove, this, &BattleDialogGraphicsScene::mapMouseMove);
    connect(&_mapsMouseHandler, &BattleDialogGraphicsSceneMouseHandlerMaps::mapMouseRelease, this, &BattleDialogGraphicsScene::mapMouseRelease);

    connect(this, &BattleDialogGraphicsScene::selectionChanged, this, &BattleDialogGraphicsScene::handleSelectionChanged);
}

BattleDialogGraphicsScene::~BattleDialogGraphicsScene()
{
}

void BattleDialogGraphicsScene::setModel(BattleDialogModel* model)
{
    _model = model;
}

BattleDialogModel* BattleDialogGraphicsScene::getModel() const
{
    return _model;
}

void BattleDialogGraphicsScene::createBattleContents()
{
    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to create scene contents, no model exists.";
        return;
    }

    qDebug() << "[Battle Dialog Scene] Creating scene contents";

    LayerGrid* gridLayer = dynamic_cast<LayerGrid*>(_model->getLayerScene().getNearest(_model->getLayerScene().getSelectedLayer(), DMHelper::LayerType_Grid));
    setDistanceScale(gridLayer ? gridLayer->getConfig().getGridScale() : _model->getGridScale());

    QGraphicsView* view = views().constFirst();
    if(view)
    {
        if(_pointerPixmap.isNull())
            _pointerPixmap.load(":/img/data/arrow.png");
        _pointerPixmapItem = addPixmap(_pointerPixmap.scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        _pointerPixmapItem->setTransformationMode(Qt::SmoothTransformation);
        QRectF sizeInScene = view->mapToScene(0, 0, DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE).boundingRect();
        _pointerPixmapItem->setScale(sizeInScene.width() / static_cast<qreal>(DMHelper::CURSOR_SIZE));
        _pointerPixmapItem->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
        _pointerPixmapItem->setVisible(_pointerVisible);
    }
}

void BattleDialogGraphicsScene::scaleBattleContents()
{
    if(_pointerPixmapItem)
    {
        QGraphicsView* view = views().constFirst();
        if(view)
        {
            QRectF sizeInScene = view->mapToScene(0, 0, DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE).boundingRect();
            _pointerPixmapItem->setScale(sizeInScene.width() / static_cast<qreal>(DMHelper::CURSOR_SIZE));
        }
    }
}

void BattleDialogGraphicsScene::clearBattleContents()
{
    delete _pointerPixmapItem; _pointerPixmapItem = nullptr;
}

void BattleDialogGraphicsScene::setPointerVisibility(bool visible)
{
    if(_pointerVisible == visible)
        return;

    _pointerVisible = visible;
    if(_pointerPixmapItem)
        _pointerPixmapItem->setVisible(_pointerVisible);
}

void BattleDialogGraphicsScene::setPointerPos(const QPointF& pos)
{
    if(_pointerPixmapItem)
        _pointerPixmapItem->setPos(pos);
}

void BattleDialogGraphicsScene::setPointerPixmap(QPixmap pixmap)
{
    _pointerPixmap = pixmap;
    if(_pointerPixmapItem)
        _pointerPixmapItem->setPixmap(_pointerPixmap.scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QPixmap BattleDialogGraphicsScene::getSelectedIcon() const
{
    QPixmap result;

    if((_selectedIcon.isEmpty()) || (!result.load(_selectedIcon)))
        result.load(QString(":/img/data/selected.png"));

    return result;
}

QString BattleDialogGraphicsScene::getSelectedIconFile() const
{
    return _selectedIcon;
}

QPointF BattleDialogGraphicsScene::getCommandPosition() const
{
    return _commandPosition;
}

QGraphicsItem* BattleDialogGraphicsScene::getDistanceLine() const
{
    QGraphicsItem* result = _distanceMouseHandler.getDistanceLine();
    if(!result)
        result = _freeDistanceMouseHandler.getDistanceLine();

    return result;
}

QGraphicsSimpleTextItem* BattleDialogGraphicsScene::getDistanceText() const
{
    QGraphicsSimpleTextItem* result = _distanceMouseHandler.getDistanceText();
    if(!result)
        result = _freeDistanceMouseHandler.getDistanceText();

    return result;
}

QGraphicsItem* BattleDialogGraphicsScene::findTopObject(const QPointF &pos)
{
    QGraphicsView* localView = views().constFirst();
    if(!localView)
        return nullptr;

    QList<QGraphicsItem *> itemList = items(pos, Qt::IntersectsItemShape, Qt::DescendingOrder, localView->transform());
    if(itemList.count() <= 0)
        return nullptr;

    // Search for the first selectable item
    for(QGraphicsItem* item : std::as_const(itemList))
    {
        if((item)&&((item->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable))
            return dynamic_cast<QGraphicsItem*>(item);
    }

    // If we get here, nothing selectable was found. The callers assume a returned item is selectable!
    return nullptr;
}

BattleDialogModelObject* BattleDialogGraphicsScene::getFinalObjectFromItem(QGraphicsItem* item)
{
    if(!item)
        return nullptr;

    BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(item);
    if(effect)
    {
        return BattleDialogModelEffect::getFinalEffect(effect);
    }
    else
    {
        UnselectedPixmap* pixmap = dynamic_cast<UnselectedPixmap*>(item);
        return pixmap ? pixmap->getObject() : nullptr;
    }
}

bool BattleDialogGraphicsScene::handleMouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
    qDebug() << "[Battle Dialog Scene] doubleclick detected at " << mouseEvent->scenePos();
#endif

    if(mouseEvent->button() == Qt::LeftButton)
    {
        QGraphicsItem* item = findTopObject(mouseEvent->scenePos());
        if(item)
        {
            QUuid itemId = BattleDialogModelEffect::getEffectIdFromItem(item);
            if(!(itemId.isNull()))
            {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
                qDebug() << "[Battle Dialog Scene] doubleclick identified on item " << itemId;
#endif
                _contextMenuItem = item;
                editItem();
                _contextMenuItem = nullptr;
            }
            else if((item->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable)
            {
                QGraphicsPixmapItem* pixItem = dynamic_cast<QGraphicsPixmapItem*>(item);
                if(pixItem)
                {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
                    qDebug() << "[Battle Dialog Scene] doubleclick on combatant " << pixItem;
#endif
                    emit itemMouseDoubleClick(pixItem);
                }
            }
        }

        _mouseDown = false;
        _mouseDownItem = nullptr;
        mouseEvent->accept();
        return false;
    }

    return true;
}

bool BattleDialogGraphicsScene::handleMouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(!_mouseDown)
    {
        BattleDialogModelCombatant* newHoverItem = nullptr;
        UnselectedPixmap* pixmap = dynamic_cast<UnselectedPixmap*>(findTopObject(mouseEvent->scenePos()));
        if(pixmap)
            newHoverItem = dynamic_cast<BattleDialogModelCombatant*>(pixmap->getObject());

        if(_mouseHoverItem != newHoverItem)
        {
            if(_mouseHoverItem)
                emit combatantHover(_mouseHoverItem, false);

            if(newHoverItem)
                emit combatantHover(newHoverItem, true);

            _mouseHoverItem = newHoverItem;
        }
    }
    else
    {
        QGraphicsItem* abstractShape = _mouseDownItem;
        QUuid effectId = BattleDialogModelEffect::getEffectIdFromItem(_mouseDownItem);

        if(mouseEvent->buttons() & Qt::RightButton)
        {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
            qDebug() << "[Battle Dialog Scene] right button mouse move detected on " << abstractShape << " at " << mouseEvent->scenePos() << " mousedown=" << _mouseDown;
#endif

            if(_mouseDown)
            {
                // |A·B| = |A| |B| COS(θ)
                // |A×B| = |A| |B| SIN(θ)
                QPointF eventPos = mouseEvent->scenePos() - _mouseDownItem->scenePos();
                qreal cross = _mouseDownPos.x()*eventPos.y()-_mouseDownPos.y()*eventPos.x();
                qreal dot = _mouseDownPos.x()*eventPos.x()+_mouseDownPos.y()*eventPos.y();
                qreal angle = qRadiansToDegrees(qAtan2(cross, dot));

                _isRotation = true;

                if((abstractShape) && (!effectId.isNull()))
                {
                    _mouseDownItem->setRotation(_previousRotation + angle);
                    BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(_mouseDownItem);
                    if(effect)
                        effect->setRotation(_previousRotation + angle);

                    emit effectChanged(abstractShape);
                }
                else
                {
                    QGraphicsPixmapItem* pixItem = dynamic_cast<QGraphicsPixmapItem*>(_mouseDownItem);
                    if(pixItem)
                        pixItem->setRotation(_previousRotation + angle);

                    BattleDialogModelObject* object = getFinalObjectFromItem(_mouseDownItem);
                    if(object)
                        object->setRotation(_previousRotation + angle);
                }
            }

            mouseEvent->accept();
            return false;
        }
        else if(mouseEvent->buttons() & Qt::LeftButton)
        {
            if((abstractShape) && (!effectId.isNull()))
            {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
                qDebug() << "[Battle Dialog Scene] left button mouse move detected on " << abstractShape << " at " << mouseEvent->scenePos() << " mousedown=" << _mouseDown;
#endif
                BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(abstractShape);
                if(effect)
                {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
                    qDebug() << "[Battle Dialog Scene] left button setting effect position for " << effect << " to shape " << abstractShape;
#endif
                    effect->setPosition(abstractShape->pos());
                }
                emit effectChanged(abstractShape);
            }
            else
            {
                QGraphicsPixmapItem* pixItem = dynamic_cast<QGraphicsPixmapItem*>(_mouseDownItem);
                if(pixItem)
                {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
                    qDebug() << "[Battle Dialog Scene] left mouse move on combatant " << pixItem;
#endif
                    bool result = true;
                    emit itemMoved(pixItem, &result);
                    if(!result)
                        return false;
                }
            }
        }

#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
        qDebug() << "[Battle Dialog Scene] mouse move default handling triggered " << mouseEvent;
#endif
    }

    return true;
}

bool BattleDialogGraphicsScene::handleMousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsItem* item = findTopObject(mouseEvent->scenePos());
    _isRotation = false;

#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
    qDebug() << "[Battle Dialog Scene] mouse press at " << mouseEvent->scenePos() << " item " << item;
#endif

    if(item)
    {
        if(_mouseHoverItem)
        {
            emit combatantHover(_mouseHoverItem, false);
            _mouseHoverItem = nullptr;
        }

        if(!BattleDialogModelEffect::getEffectIdFromItem(item).isNull())
        {
            _mouseDown = true;
            _mouseDownPos = mouseEvent->scenePos() - item->scenePos();
            _mouseDownItem = item;

            if(mouseEvent->button() == Qt::RightButton)
            {
                _previousRotation = _mouseDownItem->rotation();
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
                 qDebug() << "[Battle Dialog Scene] right mouse down on " << _mouseDownItem << " identified: pos=" << _mouseDownPos << ", rot=" << _previousRotation;
#endif
                mouseEvent->accept();
                return false;
            }
            else if(mouseEvent->button() == Qt::LeftButton)
            {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
                qDebug() << "[Battle Dialog Scene] left mouse down on " << _mouseDownItem << " identified: pos=" << _mouseDownPos << ".";
#endif
                emit effectChanged(item);
            }
            else
            {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
                qDebug() << "[Battle Dialog Scene] other mouse button down on " << _mouseDownItem << " identified: pos=" << _mouseDownPos << ".";
#endif
            }
        }
        else if((item->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable)
        {
            QGraphicsPixmapItem* pixItem = dynamic_cast<QGraphicsPixmapItem*>(item);
            if(pixItem)
            {
                if((mouseEvent->button() == Qt::LeftButton) ||
                   (mouseEvent->button() == Qt::RightButton))
                {
                    _mouseDown = true;
                    _mouseDownPos = mouseEvent->scenePos() - item->scenePos();
                    _mouseDownItem = item;

                    if(mouseEvent->button() == Qt::RightButton)
                        _previousRotation = _mouseDownItem->rotation();

#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
                    if(mouseEvent->button() == Qt::LeftButton)
                        qDebug() << "[Battle Dialog Scene] left mouse down on combatant " << pixItem;
                    else
                        qDebug() << "[Battle Dialog Scene] right mouse down on combatant " << pixItem;
#endif
                    emit itemMouseDown(pixItem, (mouseEvent->button() == Qt::LeftButton));
                    mouseEvent->accept();
                }
            }
        }
    }
    else
    {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
        qDebug() << "[Battle Dialog Scene] ignoring mouse click for non-selectable item " << item;
#endif
        mouseEvent->ignore();
        return false;
    }

#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
    qDebug() << "[Battle Dialog Scene] mouse press default handling triggered " << mouseEvent;
#endif

    return true;
}

bool BattleDialogGraphicsScene::handleMouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if((mouseEvent->button() == Qt::RightButton) && (!_isRotation))
    {
        QGraphicsItem* item = findTopObject(mouseEvent->scenePos());
        QGraphicsPixmapItem* pixItem = dynamic_cast<QGraphicsPixmapItem*>(item);

#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
        qDebug() << "[Battle Dialog Scene] right mouse released at " << mouseEvent->scenePos() << " for item " << item;
#endif

        emit itemMouseUp(pixItem);

        QMenu menu(views().constFirst());
        if((item) && (!BattleDialogModelEffect::getEffectIdFromItem(item).isNull()))
        {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
            qDebug() << "[Battle Dialog Scene] right click identified on effect " << item;
#endif
            if(_mouseDownPos == mouseEvent->scenePos() - item->scenePos())
            {
                _contextMenuItem = item;

                QAction* rollItem = new QAction(QString("Apply Effect..."), &menu);
                connect(rollItem, SIGNAL(triggered()), this, SLOT(rollItem()));
                menu.addAction(rollItem);

                BattleDialogModelObject* object = getFinalObjectFromItem(item);
                if(object)
                {
                    if(object->getLinkedObject() == nullptr)
                    {
                        QAction* linkItemAction = new QAction(QString("Link Effect..."), &menu);
                        connect(linkItemAction, SIGNAL(triggered()), this, SLOT(linkItem()));
                        menu.addAction(linkItemAction);
                    }
                    else
                    {
                        QAction* unlinkItemAction = new QAction(QString("Unlink Effect"), &menu);
                        connect(unlinkItemAction, SIGNAL(triggered()), this, SLOT(unlinkItem()));
                        menu.addAction(unlinkItemAction);
                    }
                }

                if((_model) && (_model->getLayerScene().layerCount(DMHelper::LayerType_Tokens) > 1))
                {
                    QAction* shiftItem = new QAction(QString("Change Layer..."), &menu);
                    connect(shiftItem, SIGNAL(triggered()), this, SLOT(changeEffectLayer()));
                    menu.addAction(shiftItem);
                }

                menu.addSeparator();

                QAction* edtItem = new QAction(QString("Edit Effect..."), &menu);
                connect(edtItem, SIGNAL(triggered()), this, SLOT(editItem()));
                menu.addAction(edtItem);

                QAction* deleteItem = new QAction(QString("Delete Effect..."), &menu);
                connect(deleteItem, SIGNAL(triggered()), this, SLOT(deleteItem()));
                menu.addAction(deleteItem);

                QAction* duplicateItem = new QAction(QString("Duplicate..."), &menu);
                connect(duplicateItem, SIGNAL(triggered()), this, SIGNAL(duplicateSelection()));
                menu.addAction(duplicateItem);

                menu.addSeparator();
            }
        }
        else if((item) && (pixItem) && ((item->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable))
        {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
            qDebug() << "[Battle Dialog Scene] right click identified on combatant " << pixItem;
#endif
            _contextMenuItem = item;
            _mouseDownPos = mouseEvent->scenePos();

            QAction* activateItem = new QAction(QString("Activate"), &menu);
            connect(activateItem, SIGNAL(triggered()), this, SLOT(activateCombatant()));
            menu.addAction(activateItem);

            QAction* removeItem = new QAction(QString("Remove"), &menu);
            connect(removeItem, SIGNAL(triggered()), this, SLOT(removeCombatant()));
            menu.addAction(removeItem);

            BattleDialogModelObject* object = getFinalObjectFromItem(item);
            if(object)
            {
                if(object->getLinkedObject() == nullptr)
                {
                    QAction* linkItemAction = new QAction(QString("Link Combatant..."), &menu);
                    connect(linkItemAction, SIGNAL(triggered()), this, SLOT(linkItem()));
                    menu.addAction(linkItemAction);
                }
                else
                {
                    QAction* unlinkItemAction = new QAction(QString("Unlink Combatant"), &menu);
                    connect(unlinkItemAction, SIGNAL(triggered()), this, SLOT(unlinkItem()));
                    menu.addAction(unlinkItemAction);
                }
            }

            if((_model) && (_model->getLayerScene().layerCount(DMHelper::LayerType_Tokens) > 1))
            {
                QAction* shiftItem = new QAction(QString("Change Layer..."), &menu);
                connect(shiftItem, SIGNAL(triggered()), this, SLOT(changeCombatantLayer()));
                menu.addAction(shiftItem);
            }

            QAction* duplicateItem = new QAction(QString("Duplicate..."), &menu);
            connect(duplicateItem, SIGNAL(triggered()), this, SIGNAL(duplicateSelection()));
            menu.addAction(duplicateItem);

            menu.addSeparator();

            QAction* damageItem = new QAction(QString("Damage..."), &menu);
            connect(damageItem, SIGNAL(triggered()), this, SLOT(damageCombatant()));
            menu.addAction(damageItem);

            QAction* healItem = new QAction(QString("Heal..."), &menu);
            connect(healItem, SIGNAL(triggered()), this, SLOT(healCombatant()));
            menu.addAction(healItem);

            menu.addSeparator();

            if(object)
            {
                BattleDialogModelMonsterClass* monster = dynamic_cast<BattleDialogModelMonsterClass*>(object);
                if(monster)
                {
                    MonsterClass* monsterClass = monster->getMonsterClass();
                    if(monsterClass)
                    {
                        QStringList iconList = monsterClass->getIconList();
                        if(iconList.count() > 0)
                        {
                            QMenu* tokenMenu = new QMenu(QString("Select Token..."));
                            for(int i = 0; i < iconList.count(); ++i)
                            {
                                QAction* tokenAction = new QAction(iconList.at(i), tokenMenu);
                                connect(tokenAction, &QAction::triggered, [this, i, monster](){this->changeMonsterToken(monster, i);});
                                tokenMenu->addAction(tokenAction);
                            }
                            menu.addMenu(tokenMenu);
                            menu.addSeparator();
                        }
                    }
                }
            }
        }
        else
        {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
            qDebug() << "[Battle Dialog Scene] right click identified on background";
#endif
            _mouseDownPos = mouseEvent->scenePos();
        }

        QAction* addRadiusItem = new QAction(QString("Create Radius Effect"), &menu);
        connect(addRadiusItem, SIGNAL(triggered()), this, SIGNAL(addEffectRadius()));
        menu.addAction(addRadiusItem);

        QAction* addConeItem = new QAction(QString("Create Cone Effect"), &menu);
        connect(addConeItem, SIGNAL(triggered()), this, SIGNAL(addEffectCone()));
        menu.addAction(addConeItem);

        QAction* addCubeItem = new QAction(QString("Create Cube Effect"), &menu);
        connect(addCubeItem, SIGNAL(triggered()), this, SIGNAL(addEffectCube()));
        menu.addAction(addCubeItem);

        QAction* addLineItem = new QAction(QString("Create Line Effect"), &menu);
        connect(addLineItem, SIGNAL(triggered()), this, SIGNAL(addEffectLine()));
        menu.addAction(addLineItem);

        menu.addSeparator();

        QAction* addPCItem = new QAction(QString("Add PC..."), &menu);
        connect(addPCItem, SIGNAL(triggered()), this, SIGNAL(addPC()));
        menu.addAction(addPCItem);

        QAction* addMonsterItem = new QAction(QString("Add Monsters..."), &menu);
        connect(addMonsterItem, SIGNAL(triggered()), this, SIGNAL(addMonsters()));
        menu.addAction(addMonsterItem);

        QAction* addNPCItem = new QAction(QString("Add NPC..."), &menu);
        connect(addNPCItem, SIGNAL(triggered()), this, SIGNAL(addNPC()));
        menu.addAction(addNPCItem);

        QAction* addObjectItem = new QAction(QString("Add Image Object..."), &menu);
        connect(addObjectItem, SIGNAL(triggered()), this, SIGNAL(addEffectObject()));
        menu.addAction(addObjectItem);

        QAction* addObjectVideoItem = new QAction(QString("Add Video Object..."), &menu);
        connect(addObjectVideoItem, SIGNAL(triggered()), this, SIGNAL(addEffectObjectVideo()));
        menu.addAction(addObjectVideoItem);

        QAction* castItem = new QAction(QString("Cast Spell..."), &menu);
        connect(castItem, SIGNAL(triggered()), this, SIGNAL(castSpell()));
        menu.addAction(castItem);

        _commandPosition = _mouseDownPos;
        menu.exec(mouseEvent->screenPos());
        _commandPosition = DMHelper::INVALID_POINT;
    }
    else if(mouseEvent->button() == Qt::LeftButton)
    {
        QGraphicsPixmapItem* pixItem = dynamic_cast<QGraphicsPixmapItem*>(_mouseDownItem);

        if(pixItem)
        {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
            qDebug() << "[Battle Dialog Scene] left mouse released at " << mouseEvent->scenePos() << " for item " << pixItem;
#endif
            emit itemMouseUp(pixItem);
        }
        else
        {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
            qDebug() << "[Battle Dialog Scene] left mouse released on background";
#endif
        }
    }

    _contextMenuItem = nullptr;
    _mouseDown = false;
    _mouseDownItem = nullptr;

#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
    qDebug() << "[Battle Dialog Scene] mouse release default handling triggered " << mouseEvent;
#endif

    return true;
}

QPointF BattleDialogGraphicsScene::getViewportCenter()
{
    QPointF combatantPos(DMHelper::INVALID_POINT);

    QGraphicsView* view = views().constFirst();
    if(view)
    {
        QRectF viewportRect = view->mapToScene(view->viewport()->rect()).boundingRect().toAlignedRect();
        combatantPos = viewportRect.topLeft() + QPointF(viewportRect.width() / 2, viewportRect.height() / 2);
    }

    return combatantPos;
}

void BattleDialogGraphicsScene::setDistanceHeight(qreal heightDelta)
{
    _distanceMouseHandler.setHeightDelta(heightDelta);
    _freeDistanceMouseHandler.setHeightDelta(heightDelta);
}

void BattleDialogGraphicsScene::setDistanceScale(int scale)
{
    if(scale <= 0)
        return;

    _distanceMouseHandler.setDistanceScale(scale);
    _freeDistanceMouseHandler.setDistanceScale(scale);
}

void BattleDialogGraphicsScene::setDistanceLineColor(const QColor& color)
{
    _distanceMouseHandler.setDistanceLineColor(color);
    _freeDistanceMouseHandler.setDistanceLineColor(color);
}

void BattleDialogGraphicsScene::setDistanceLineType(int lineType)
{
    _distanceMouseHandler.setDistanceLineType(lineType);
    _freeDistanceMouseHandler.setDistanceLineType(lineType);
}

void BattleDialogGraphicsScene::setDistanceLineWidth(int lineWidth)
{
    _distanceMouseHandler.setDistanceLineWidth(lineWidth);
    _freeDistanceMouseHandler.setDistanceLineWidth(lineWidth);
}

void BattleDialogGraphicsScene::setInputMode(int inputMode)
{
    if(((_inputMode == DMHelper::BattleFrameState_Distance) || (_inputMode == DMHelper::BattleFrameState_FreeDistance)) &&
       ((inputMode != DMHelper::BattleFrameState_Distance) && (inputMode != DMHelper::BattleFrameState_FreeDistance)))
    {
        _distanceMouseHandler.cleanup();
        _freeDistanceMouseHandler.cleanup();
    }

    _inputMode = inputMode;
}

void BattleDialogGraphicsScene::setSelectedIcon(const QString& selectedIcon)
{
    _selectedIcon = selectedIcon;
}

void BattleDialogGraphicsScene::editItem()
{
    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to edit item, no model exists!";
        return;
    }

    if(!_contextMenuItem)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to edit item, no context menu item known! ";
        return;
    }

    BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(_contextMenuItem);
    if(!effect)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to edit item, no model data available! " << _contextMenuItem;
        return;
    }

    BattleDialogEffectSettingsBase* settings = effect->getEffectEditor();
    if(!settings)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to edit item, not effect editor available for this effect: " << _contextMenuItem;
        return;
    }

    // Merge in any other selected effects of the same type
    QList<QGraphicsItem*> selected = selectedItems();
    foreach(QGraphicsItem* effectItem, selected)
    {
        BattleDialogModelEffect* selectedEffect = BattleDialogModelEffect::getEffectFromItem(effectItem);
        if((selectedEffect) && (selectedEffect != effect) && (selectedEffect->getEffectType() == effect->getEffectType()))
            settings->mergeValuesToSettings(*selectedEffect);
    }

    settings->exec();
    if(settings->result() == QDialog::Accepted)
    {
        QList<Layer*> tokenLayers = _model->getLayerScene().getLayers(DMHelper::LayerType_Tokens);

        foreach(QGraphicsItem* effectItem, selected)
        {
            BattleDialogModelEffect* selectedEffect = BattleDialogModelEffect::getEffectFromItem(effectItem);
            if((selectedEffect) && (selectedEffect->getEffectType() == effect->getEffectType()))
            {
                settings->copyValuesFromSettings(*selectedEffect);
                LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(_model->getLayerFromEffect(tokenLayers, selectedEffect));
                selectedEffect->applyEffectValues(*effectItem, tokenLayer->getScale());
                emit effectChanged(effectItem);
            }
        }
    }
    settings->deleteLater();
}

void BattleDialogGraphicsScene::rollItem()
{
    qDebug() << "[Battle Dialog Scene] Roll Item triggered for " << _contextMenuItem;
    if(_contextMenuItem)
        emit applyEffect(_contextMenuItem);
}

void BattleDialogGraphicsScene::deleteItem()
{
    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to delete item, no model exists!";
        return;
    }

    if(!_contextMenuItem)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to delete item, no context menu item known!";
        return;
    }

    QGraphicsItem* deleteItem = _contextMenuItem;
    const QList<QGraphicsItem*> deleteChildItems = deleteItem->childItems();
    for(QGraphicsItem* childItem : deleteChildItems)
    {
        if(childItem->data(BATTLE_DIALOG_MODEL_EFFECT_ROLE).toInt() == BattleDialogModelEffect::BattleDialogModelEffectRole_Area)
            deleteItem = childItem;
    }

    if(!deleteItem)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to delete item, unexpected error finding the right item to delete!";
        return;
    }

    BattleDialogModelEffect* deleteEffect = BattleDialogModelEffect::getEffectFromItem(deleteItem);
    if(!deleteEffect)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to delete item, no model data available! " << deleteItem;
        return;
    }

    QMessageBox::StandardButton result = QMessageBox::critical(nullptr, QString("Confirm Delete Effect"), QString("Are you sure you wish to delete this effect?"), QMessageBox::Yes | QMessageBox::No);
    if(result == QMessageBox::Yes)
    {
        qDebug() << "[Battle Dialog Scene] confirmed deleting effect " << deleteEffect;
        if(_mouseDownItem == _contextMenuItem)
        {
            _mouseDown = false;
            _mouseDownItem = nullptr;
        }
        emit effectRemoved(_contextMenuItem);
        _model->removeEffect(deleteEffect);
    }
}

void BattleDialogGraphicsScene::linkItem()
{
    if(!_contextMenuItem)
        return;

    BattleDialogModelObject* object = getFinalObjectFromItem(_contextMenuItem);
    if(object)
        emit itemLink(object);

    _contextMenuItem = nullptr;
}

void BattleDialogGraphicsScene::unlinkItem()
{
    if(!_contextMenuItem)
        return;

    BattleDialogModelObject* object = getFinalObjectFromItem(_contextMenuItem);
    if(object)
        emit itemUnlink(object);

    _contextMenuItem = nullptr;
}

void BattleDialogGraphicsScene::activateCombatant()
{
    UnselectedPixmap* pixmap = dynamic_cast<UnselectedPixmap*>(_contextMenuItem);
    if(!pixmap)
        return;

    BattleDialogModelCombatant* combatant = dynamic_cast<BattleDialogModelCombatant*>(pixmap->getObject());
    if(combatant)
        emit combatantActivate(combatant);
}

void BattleDialogGraphicsScene::removeCombatant()
{
    UnselectedPixmap* pixmap = dynamic_cast<UnselectedPixmap*>(_contextMenuItem);
    if(!pixmap)
        return;

    BattleDialogModelCombatant* combatant = dynamic_cast<BattleDialogModelCombatant*>(pixmap->getObject());
    if(combatant)
        emit combatantRemove(combatant);
}

void BattleDialogGraphicsScene::changeCombatantLayer()
{
    UnselectedPixmap* pixmap = dynamic_cast<UnselectedPixmap*>(_contextMenuItem);
    if(!pixmap)
        return;

    BattleDialogModelCombatant* combatant = dynamic_cast<BattleDialogModelCombatant*>(pixmap->getObject());
    if(combatant)
        emit itemChangeLayer(combatant);
}

void BattleDialogGraphicsScene::damageCombatant()
{
    UnselectedPixmap* pixmap = dynamic_cast<UnselectedPixmap*>(_contextMenuItem);
    if(!pixmap)
        return;

    BattleDialogModelCombatant* combatant = dynamic_cast<BattleDialogModelCombatant*>(pixmap->getObject());
    if(combatant)
        emit combatantDamage(combatant);
}

void BattleDialogGraphicsScene::healCombatant()
{
    UnselectedPixmap* pixmap = dynamic_cast<UnselectedPixmap*>(_contextMenuItem);
    if(!pixmap)
        return;

    BattleDialogModelCombatant* combatant = dynamic_cast<BattleDialogModelCombatant*>(pixmap->getObject());
    if(combatant)
        emit combatantHeal(combatant);
}

void BattleDialogGraphicsScene::changeMonsterToken(BattleDialogModelMonsterClass* monster, int iconIndex)
{
    if(monster)
        emit monsterChangeToken(monster, iconIndex);
}

void BattleDialogGraphicsScene::changeEffectLayer()
{
    BattleDialogModelEffect* effect = dynamic_cast<BattleDialogModelEffect*>(getFinalObjectFromItem(_contextMenuItem));
    if(effect)
        emit itemChangeLayer(effect);
}

void BattleDialogGraphicsScene::handleSelectionChanged()
{
    _selectionCount = selectedItems().count();
}

void BattleDialogGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(!mouseEvent)
        return;

    BattleDialogGraphicsSceneMouseHandlerBase* mouseHandler = getMouseHandler(mouseEvent);
    if(mouseHandler)
    {
        if(!mouseHandler->mouseDoubleClickEvent(mouseEvent))
            return;
    }

#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEEVENTS
    qDebug() << "[Battle Dialog Scene] mouse double click default handling triggered " << mouseEvent;
#endif
    // If the function reaches this point, default handling is expected
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void BattleDialogGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to handle mouse move event, no model exists.";
        return;
    }

    if(!mouseEvent)
        return;

    BattleDialogGraphicsSceneMouseHandlerBase* mouseHandler = getMouseHandler(mouseEvent);
    if(mouseHandler)
    {
        if(!mouseHandler->mouseMoveEvent(mouseEvent))
            return;
    }

    // If the function reaches this point, default handling (ie drag and move) is expected
    QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void BattleDialogGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(!mouseEvent)
        return;

    BattleDialogGraphicsSceneMouseHandlerBase* mouseHandler = getMouseHandler(mouseEvent);
    if(mouseHandler)
    {
        if((mouseHandler == &_distanceMouseHandler) || (mouseHandler == &_freeDistanceMouseHandler))
        {
            _distanceMouseHandler.cleanup();
            _freeDistanceMouseHandler.cleanup();
        }

        if(!mouseHandler->mousePressEvent(mouseEvent))
            return;
    }

    // If the function reaches this point, default handling is expected
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void BattleDialogGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(!mouseEvent)
        return;

    BattleDialogGraphicsSceneMouseHandlerBase* mouseHandler = getMouseHandler(mouseEvent);
    if(mouseHandler)
    {
        if(!mouseHandler->mouseReleaseEvent(mouseEvent))
            return;
    }

    // If the function reaches this point, default handling is expected
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void BattleDialogGraphicsScene::wheelEvent(QGraphicsSceneWheelEvent *wheelEvent)
{
    if((wheelEvent) &&
       ((wheelEvent->orientation() & Qt::Vertical) == Qt::Vertical) &&
       ((wheelEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier))
    {
        wheelEvent->accept();
        emit mapZoom(wheelEvent->delta());
    }
}

void BattleDialogGraphicsScene::keyPressEvent(QKeyEvent *keyEvent)
{
    if((!_spaceDown) && (keyEvent) && (!keyEvent->isAutoRepeat()) && (keyEvent->key() == Qt::Key_Space))
    {
        _spaceDown = true;
        emit mapMoveToggled();
    }

    QGraphicsScene::keyPressEvent(keyEvent);
}

void BattleDialogGraphicsScene::keyReleaseEvent(QKeyEvent *keyEvent)
{
    if((_spaceDown) && (keyEvent) && (!keyEvent->isAutoRepeat()) && (keyEvent->key() == Qt::Key_Space))
    {
        _spaceDown = false;
        emit mapMoveToggled();
    }

    QGraphicsScene::keyReleaseEvent(keyEvent);
}

void BattleDialogGraphicsScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if(!event)
        return;

    if(isMimeDataImage(event->mimeData()))
    {
        event->acceptProposedAction();
        return;
    }

    QGraphicsScene::dragEnterEvent(event);
}

void BattleDialogGraphicsScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if(!event)
        return;

    event->acceptProposedAction();
}

void BattleDialogGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if(!event)
        return;

    if(isMimeDataImage(event->mimeData()))
    {
        // Create a QMessageBox
        QMessageBox msgBox;
        msgBox.setText("Do you want to add the image as a layer or a token?");
        msgBox.setIcon(QMessageBox::Question);
        msgBox.addButton("Layer", QMessageBox::ActionRole);
        msgBox.addButton("Token", QMessageBox::ActionRole);
        msgBox.addButton("Cancel", QMessageBox::RejectRole);
        int result = msgBox.exec();

        if(result == 0)
        {
            event->acceptProposedAction();
            emit addLayerImageFile(getMimeDataImageFile(event->mimeData()));
        }
        else if(result == 1)
        {
            event->acceptProposedAction();
            emit addEffectObjectFile(getMimeDataImageFile(event->mimeData()));
        }
        else
        {
            event->ignore();
        }

        return;
    }

    QGraphicsScene::dropEvent(event);
}

bool BattleDialogGraphicsScene::isMimeDataImage(const QMimeData* mimeData) const
{
    if(!mimeData)
        return false;

    if(mimeData->hasUrls())
    {
        foreach(QUrl url, mimeData->urls())
        {
            if(url.isLocalFile())
            {
                QString filePath = url.toLocalFile();
                QMimeType mimeType = QMimeDatabase().mimeTypeForFile(filePath);
                if((mimeType.isValid()) &&
                    (mimeType.name().startsWith("image/")))
                    return true;
            }
        }
    }

    return false;
}

QString BattleDialogGraphicsScene::getMimeDataImageFile(const QMimeData* mimeData) const
{
    if(!mimeData)
        return QString();

    if(mimeData->hasUrls())
    {
        foreach(QUrl url, mimeData->urls())
        {
            if(url.isLocalFile())
            {
                QString filePath = url.toLocalFile();
                QMimeType mimeType = QMimeDatabase().mimeTypeForFile(filePath);
                if((mimeType.isValid()) && (mimeType.name().startsWith("image/")))
                    return filePath;
            }
        }
    }

    return QString();
}

/*
BattleDialogModelEffect* BattleDialogGraphicsScene::createEffect(int type, int size, int width, const QColor& color, const QString& filename)
{
    BattleDialogModelEffect* result = nullptr;
    qreal scaledHalfSize;
    QPointF effectPosition = _commandPosition != DMHelper::INVALID_POINT ? _commandPosition : getViewportCenter();

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
        case BattleDialogModelEffect::BattleDialogModelEffect_ObjectVideo:
            result = BattleDialogModelEffectFactory::createEffectObjectVideo(effectPosition, QSize(width, size), color, filename);
            break;
        default:
            break;
    }

    return result;
}
*/

BattleDialogGraphicsSceneMouseHandlerBase* BattleDialogGraphicsScene::getMouseHandler(QGraphicsSceneMouseEvent *mouseEvent)
{
    BattleDialogGraphicsSceneMouseHandlerBase* result = nullptr;

    if((mouseEvent) && (((mouseEvent->buttons() & Qt::MiddleButton) == Qt::MiddleButton) || (_spaceDown)))
    {
        result = &_mapsMouseHandler;
    }
    else
    {
        switch(_inputMode)
        {
            case DMHelper::BattleFrameState_FoWEdit:
                result = &_rawMouseHandler;
                break;
            case DMHelper::BattleFrameState_Pointer:
                result = &_pointerMouseHandler;
                break;
            case DMHelper::BattleFrameState_Distance:
                result = &_distanceMouseHandler;
                break;
            case DMHelper::BattleFrameState_FreeDistance:
                result = &_freeDistanceMouseHandler;
                break;
            case DMHelper::BattleFrameState_CameraEdit:
                result = &_cameraMouseHandler;
                break;
            case DMHelper::BattleFrameState_CombatantEdit:
                result = &_combatantMouseHandler;
                break;
            case DMHelper::BattleFrameState_ZoomSelect:
            case DMHelper::BattleFrameState_CameraSelect:
            case DMHelper::BattleFrameState_FoWSelect:
                result = &_combatantMouseHandler;
                break;
            default:
                result = nullptr;
                break;
        }

        if((result == &_combatantMouseHandler) &&
           (mouseEvent) &&
           ((mouseEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) &&
           (mouseEvent->buttons() != Qt::NoButton) &&
           (_selectionCount == 0))
        {
            QGraphicsItem* item = findTopObject(mouseEvent->scenePos());
            if((!item) || ((item->flags() & QGraphicsItem::ItemIsSelectable) != QGraphicsItem::ItemIsSelectable))
                result = &_mapsMouseHandler;
        }
    }

    return result;
}

