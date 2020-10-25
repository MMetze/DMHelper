#include "battledialoggraphicsscene.h"
#include "battledialogeffectsettings.h"
#include "battledialogmodel.h"
#include "battledialogmodeleffect.h"
#include "battledialogmodeleffectfactory.h"
#include "unselectedpixmap.h"
#include "grid.h"
#include "spellbook.h"
#include "spell.h"
#include <QMenu>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QtMath>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDebug>

// TODO: adjust grid offsets to really match resized battle contents.

// Uncomment this to log all mouse movement actions
//#define BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE

BattleDialogGraphicsScene::BattleDialogGraphicsScene(QObject *parent) :
    QGraphicsScene(parent),
    _contextMenuItem(nullptr),
    _grid(nullptr),
    _model(nullptr),
    _itemList(),
    _mouseDown(false),
    _mouseDownPos(),
    _mouseDownItem(nullptr),
    _mouseHoverItem(nullptr),
    _previousRotation(0.0),
    _inputMode(-1),
    _pointerPixmapItem(nullptr),
    _pointerVisible(false),
    _pointerPixmap(),
    _selectedIcon(),
    _distanceMouseHandler(*this),
    _pointerMouseHandler(*this),
    _rawMouseHandler(*this),
    _cameraMouseHandler(*this),
    _combatantMouseHandler(*this)//,
    //_selectMouseHandler(*this)
{
    connect(&_distanceMouseHandler, &BattleDialogGraphicsSceneMouseHandlerDistance::distanceChanged, this, &BattleDialogGraphicsScene::distanceChanged);
    connect(&_rawMouseHandler, &BattleDialogGraphicsSceneMouseHandlerRaw::rawMousePress, this, &BattleDialogGraphicsScene::battleMousePress);
    connect(&_rawMouseHandler, &BattleDialogGraphicsSceneMouseHandlerRaw::rawMouseMove, this, &BattleDialogGraphicsScene::battleMouseMove);
    connect(&_rawMouseHandler, &BattleDialogGraphicsSceneMouseHandlerRaw::rawMouseRelease, this, &BattleDialogGraphicsScene::battleMouseRelease);
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

void BattleDialogGraphicsScene::createBattleContents(const QRect& rect)
{
    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to create scene contents, no model exists.";
        return;
    }

    if(!isSceneEmpty())
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to create scene contents: " << rect << ". Contents already exist!";
        return;
    }

    qDebug() << "[Battle Dialog Scene] Creating scene contents: " << rect;
    _grid = new Grid(*this, rect);
    _grid->rebuildGrid(*_model);

    QList<BattleDialogModelEffect*> effects = _model->getEffectList();
    for(BattleDialogModelEffect* effect : effects)
    {
        if(effect)
            addEffectShape(*effect);
    }

    QList<QGraphicsView*> viewList = views();
    if((viewList.count() > 0) && (viewList.at(0)))
    {
        QGraphicsView* view = viewList.at(0);
        if(_pointerPixmap.isNull())
            _pointerPixmap.load(":/img/data/arrow.png");
        _pointerPixmapItem = addPixmap(_pointerPixmap.scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        _pointerPixmapItem->setTransformationMode(Qt::SmoothTransformation);
        QRectF sizeInScene = view->mapToScene(0, 0, DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE).boundingRect();
        _pointerPixmapItem->setScale(sizeInScene.width() / static_cast<qreal>(DMHelper::CURSOR_SIZE));
        //scaleW = static_cast<qreal>(_background->pixmap().width()) / static_cast<qreal>(pointerPmp.width());
        //scaleH = static_cast<qreal>(_background->pixmap().height()) / static_cast<qreal>(pointerPmp.height());
        //_pointerPixmapItem->setScale(COMPASS_SCALE * qMin(scaleW, scaleH));
        _pointerPixmapItem->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
        _pointerPixmapItem->setVisible(_pointerVisible);
    }
}

void BattleDialogGraphicsScene::resizeBattleContents(const QRect& rect)
{
    qDebug() << "[Battle Dialog Scene] Resizing scene contents";

    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to resize scene contents, no model exists.";
        return;
    }

    if(_grid)
    {
        qDebug() << "[Battle Dialog Scene]     Resizing grid, grid shape = " << rect;
        _grid->setGridShape(rect);
        _grid->rebuildGrid(*_model);
    }

    for(QGraphicsItem* item : _itemList)
    {
        if(item)
        {
            QPoint mapPos = item->pos().toPoint() + _model->getPreviousMapRect().topLeft();
            item->setPos(mapPos - _model->getMapRect().topLeft());
            //_model->getEffectById(QUuid(item->data(BATTLE_DIALOG_MODEL_EFFECT_ID).toString()))->setPosition(item->pos());
            BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(item);
            if(effect)
                effect->setPosition(item->pos());
            qDebug() << "[Battle Dialog Scene]     Setting position for item " << item << " to " << item->pos();
        }
    }
}

void BattleDialogGraphicsScene::updateBattleContents()
{
    qDebug() << "[Battle Dialog Scene] Updating scene contents";

    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to update scene contents, no model exists.";
        return;
    }

    if(_grid)
    {
        qDebug() << "[Battle Dialog Scene]     Rebuilding grid, grid scale = " << _model->getGridScale();
        _grid->rebuildGrid(*_model);
    }

    for(QGraphicsItem* item : _itemList)
    {
        if(item)
        {
            BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(item);
            if(effect)
            {
                qreal newScale = static_cast<qreal>(effect->getSize()) * static_cast<qreal>(_model->getGridScale()) / 500.0;
                qDebug() << "[Battle Dialog Scene]     Setting scale for item " << item << " to " << newScale;
                qreal oldScale = item->scale();
                item->setScale(newScale);
                item->setPos(item->pos() * newScale/oldScale);
                effect->setPosition(item->pos());
            }
        }
    }
}

void BattleDialogGraphicsScene::scaleBattleContents()
{
    if(_pointerPixmapItem)
    {
        QList<QGraphicsView*> viewList = views();
        if((viewList.count() > 0) && (viewList.at(0)))
        {
            QGraphicsView* view = viewList.at(0);
            QRectF sizeInScene = view->mapToScene(0, 0, DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE).boundingRect();
            _pointerPixmapItem->setScale(sizeInScene.width() / static_cast<qreal>(DMHelper::CURSOR_SIZE));
        }
    }
}

void BattleDialogGraphicsScene::clearBattleContents()
{
    qDebug() << "[Battle Dialog Scene] Clearing battle contents.";
    if(_grid)
    {
        _grid->clear();
        delete _grid;
        _grid = nullptr;
    }

    qDeleteAll(_itemList);
    _itemList.clear();

    delete _pointerPixmapItem; _pointerPixmapItem = nullptr;
}

void BattleDialogGraphicsScene::setEffectVisibility(bool visible, bool allEffects)
{
    //qDebug() << "[Battle Dialog Scene] Setting effect visibility to " << visible;

    bool newVisible = visible;

    for(QGraphicsItem* item : _itemList)
    {
        if(item)
        {
            if(!allEffects)
                newVisible = visible && BattleDialogModelEffect::getEffectVisibleFromItem(item);

            if(item->isVisible() != newVisible)
                item->setVisible(newVisible);
        }
    }
}

void BattleDialogGraphicsScene::setGridVisibility(bool visible)
{
    //qDebug() << "[Battle Dialog Scene] Setting grid visibility to " << visible;

    if(_grid)
    {
        _grid->setGridVisible(visible);
    }
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

QList<QGraphicsItem*> BattleDialogGraphicsScene::getEffectItems() const
{
    return _itemList;
}

bool BattleDialogGraphicsScene::isSceneEmpty() const
{
    return((_grid == nullptr) && (_itemList.count() == 0));
}

void BattleDialogGraphicsScene::handleItemChanged(QGraphicsItem* item)
{
    emit itemChanged(item);
}

QGraphicsItem* BattleDialogGraphicsScene::findTopObject(const QPointF &pos)
{
    QGraphicsItem* result = nullptr;

    QGraphicsView* localView = views().first();
    if(!localView)
        return nullptr;

    QList<QGraphicsItem *> itemList = items(pos, Qt::IntersectsItemShape, Qt::DescendingOrder, localView->transform());
    if(itemList.count() <= 0)
        return nullptr;

    // Search for the first selectable item
    for(QGraphicsItem* item : itemList)
    {
        if((item)&&((item->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable))
        {
            //return dynamic_cast<QAbstractGraphicsShapeItem*>(item);
            //return dynamic_cast<QGraphicsItem*>(item);
            result = dynamic_cast<QGraphicsItem*>(item);
            //qDebug() << "[Battle Dialog Scene] find top object found selectable object: " << result;
            return result;
        }
    }

    // If we get here, nothing selectable was found, so return the top-most item
    //return dynamic_cast<QAbstractGraphicsShapeItem*>(itemList.first());
    //result = dynamic_cast<QGraphicsItem*>(itemList.first());
    // WRONG - return no item - the callers assume a returned item is selectable!
    result = nullptr;
    //qDebug() << "[Battle Dialog Scene] find top object found, but not selectable: " << result;
    return result;
}

bool BattleDialogGraphicsScene::handleMouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    qDebug() << "[Battle Dialog Scene] doubleclick detected at " << mouseEvent->scenePos();

    if(mouseEvent->button() == Qt::LeftButton)
    {
        QGraphicsItem* item = findTopObject(mouseEvent->scenePos());
        QUuid itemId = BattleDialogModelEffect::getEffectIdFromItem(item);
        if((item)&&(!(itemId.isNull())))
        {
            qDebug() << "[Battle Dialog Scene] doubleclick identified on item " << itemId;
            _contextMenuItem = item;
            editItem();
            _contextMenuItem = nullptr;
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
            newHoverItem = pixmap->getCombatant();

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
        //QAbstractGraphicsShapeItem* abstractShape = dynamic_cast<QAbstractGraphicsShapeItem*>(_mouseDownItem);
        QGraphicsItem* abstractShape = _mouseDownItem;
        QUuid effectId = BattleDialogModelEffect::getEffectIdFromItem(_mouseDownItem);

        if(mouseEvent->buttons() & Qt::RightButton)
        {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
            qDebug() << "[Battle Dialog Scene] right button mouse move detected on " << abstractShape << " at " << mouseEvent->scenePos() << " mousedown=" << _mouseDown;
#endif

            if((_mouseDown) && (abstractShape) && (!effectId.isNull()))
            {
                // |A·B| = |A| |B| COS(θ)
                // |A×B| = |A| |B| SIN(θ)
                QPointF eventPos = mouseEvent->scenePos() - _mouseDownItem->scenePos();
                qreal cross = _mouseDownPos.x()*eventPos.y()-_mouseDownPos.y()*eventPos.x();
                qreal dot = _mouseDownPos.x()*eventPos.x()+_mouseDownPos.y()*eventPos.y();
                qreal angle = qRadiansToDegrees(qAtan2(cross,dot));
                _mouseDownItem->setRotation(_previousRotation + angle);
                BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(_mouseDownItem);
                if(effect)
                    effect->setRotation(_previousRotation + angle);
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
                else
                    qDebug() << "[Battle Dialog Scene] ERROR: unable to find effect model data for rotation" << _mouseDownItem;
#endif
                emit effectChanged(abstractShape);
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
    //QAbstractGraphicsShapeItem* abstractShape = dynamic_cast<QAbstractGraphicsShapeItem*>(item);
    QGraphicsItem* abstractShape = item;

    qDebug() << "[Battle Dialog Scene] mouse press at " << mouseEvent->scenePos() << " item " << item << " shape " << abstractShape;

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
                 qDebug() << "[Battle Dialog Scene] right mouse down on " << _mouseDownItem << " identified: pos=" << _mouseDownPos << ", rot=" << _previousRotation;
                mouseEvent->accept();
                return false;
            }
            else if(mouseEvent->button() == Qt::LeftButton)
            {
                qDebug() << "[Battle Dialog Scene] left mouse down on " << _mouseDownItem << " identified: pos=" << _mouseDownPos << ".";
                emit effectChanged(abstractShape);
            }
            else
            {
                qDebug() << "[Battle Dialog Scene] other mouse button down on " << _mouseDownItem << " identified: pos=" << _mouseDownPos << ".";
            }
        }
        else if((item->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable)
        {
            QGraphicsPixmapItem* pixItem = dynamic_cast<QGraphicsPixmapItem*>(item);
            if(pixItem)
            {
                _mouseDown = true;
                _mouseDownItem = item;
                qDebug() << "[Battle Dialog Scene] left mouse down on combatant " << pixItem;
                emit itemMouseDown(pixItem);
                mouseEvent->accept();
            }
        }
    }
    else
    {
        qDebug() << "[Battle Dialog Scene] ignoring mouse click for non-selectable item " << item;
        mouseEvent->ignore();
        return false;
    }

    qDebug() << "[Battle Dialog Scene] mouse press default handling triggered " << mouseEvent;

    return true;
}

bool BattleDialogGraphicsScene::handleMouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(mouseEvent->button() == Qt::RightButton)
    {
        QGraphicsItem* item = findTopObject(mouseEvent->scenePos());
        qDebug() << "[Battle Dialog Scene] right mouse released at " << mouseEvent->scenePos() << " for item " << item;

        QWidget* parentWidget = nullptr;
        if(views().count() > 0 )
            parentWidget = views().first();
        QMenu menu(parentWidget);
        if((item)&&(!BattleDialogModelEffect::getEffectIdFromItem(item).isNull()))
        {
            qDebug() << "[Battle Dialog Scene] right click identified on effect " << item;
            if(_mouseDownPos == mouseEvent->scenePos() - item->scenePos())
            {
                _contextMenuItem = item;

                QAction* rollItem = new QAction(QString("Apply Effect..."), &menu);
                connect(rollItem,SIGNAL(triggered()),this,SLOT(rollItem()));
                menu.addAction(rollItem);

                menu.addSeparator();

                QAction* edtItem = new QAction(QString("Edit Effect..."), &menu);
                connect(edtItem,SIGNAL(triggered()),this,SLOT(editItem()));
                menu.addAction(edtItem);

                QAction* deleteItem = new QAction(QString("Delete Effect..."), &menu);
                connect(deleteItem,SIGNAL(triggered()),this,SLOT(deleteItem()));
                menu.addAction(deleteItem);
            }
        }
        else
        {
            qDebug() << "[Battle Dialog Scene] right click identified on background";
            _mouseDownPos = mouseEvent->scenePos();

            QAction* addRadiusItem = new QAction(QString("Create Radius Effect"), &menu);
            connect(addRadiusItem,SIGNAL(triggered()),this,SLOT(addEffectRadius()));
            menu.addAction(addRadiusItem);

            QAction* addConeItem = new QAction(QString("Create Cone Effect"), &menu);
            connect(addConeItem,SIGNAL(triggered()),this,SLOT(addEffectCone()));
            menu.addAction(addConeItem);

            QAction* addCubeItem = new QAction(QString("Create Cube Effect"), &menu);
            connect(addCubeItem,SIGNAL(triggered()),this,SLOT(addEffectCube()));
            menu.addAction(addCubeItem);

            QAction* addLineItem = new QAction(QString("Create Line Effect"), &menu);
            connect(addLineItem,SIGNAL(triggered()),this,SLOT(addEffectLine()));
            menu.addAction(addLineItem);

            menu.addSeparator();

            QAction* castItem = new QAction(QString("Cast Spell..."), &menu);
            connect(castItem,SIGNAL(triggered()),this,SLOT(castSpell()));
            menu.addAction(castItem);
        }


        menu.exec(mouseEvent->screenPos());

        //_contextMenuItem = nullptr;
        //_mouseDown = false;
        //_mouseDownItem = nullptr;

        // Check: These two lines removed to avoid weird rubber band when cancelling context menu
        // mouseEvent->accept();
        // return;
    }
    else if(mouseEvent->button() == Qt::LeftButton)
    {
        //QGraphicsView* localView = views().first();
        //QGraphicsPixmapItem* item = dynamic_cast<QGraphicsPixmapItem*>(itemAt(mouseEvent->scenePos(),localView->transform()));
        QGraphicsPixmapItem* pixItem = dynamic_cast<QGraphicsPixmapItem*>(_mouseDownItem);

        if(pixItem)
        {
            qDebug() << "[Battle Dialog Scene] left mouse released at " << mouseEvent->scenePos() << " for item " << pixItem;
            emit itemMouseUp(pixItem);
        }
        else
        {
            qDebug() << "[Battle Dialog Scene] left mouse released on background";
        }
    }

    _contextMenuItem = nullptr;
    _mouseDown = false;
    _mouseDownItem = nullptr;

    qDebug() << "[Battle Dialog Scene] mouse release default handling triggered " << mouseEvent;

    return true;
}

void BattleDialogGraphicsScene::setDistanceHeight(qreal heightDelta)
{
    _distanceMouseHandler.setHeightDelta(heightDelta);
}

void BattleDialogGraphicsScene::setInputMode(int inputMode)
{
    _inputMode = inputMode;
}

void BattleDialogGraphicsScene::addEffectObject()
{
    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to create object effect, no model exists.";
        return;
    }

    QString filename = QFileDialog::getOpenFileName(nullptr, QString("Select object image file.."));
    BattleDialogModelEffect* effect = createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Object, 20, 20, QColor(), filename);
    
    if(effect)
    {
        _contextMenuItem = addEffect(effect);
        if(_contextMenuItem)
        {
            editItem();
            _contextMenuItem = nullptr;
        }
    }
}

void BattleDialogGraphicsScene::addEffectRadius()
{
    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to create radius effect, no model exists.";
        return;
    }

    /*
    BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Radius);
    if(!effect)
        return;

    // Note: The size doubled to convert radius to diameter
    qreal scaledSize = static_cast<qreal>(effect->getSize()) * 2.0 * _model->getGridScale() / 5.0;
    effect->setPosition(_mouseDownPos.x() - (scaledSize / 2.0),_mouseDownPos.y() - (scaledSize / 2.0));
    */

    // Note: The size doubled to convert radius to diameter
    /*
    qreal scaledHalfSize = 20.0 * 2.0 * _model->getGridScale() / (5.0 * 2.0);
    QPointF effectPosition = _mouseDownPos - QPointF(scaledHalfSize, scaledHalfSize);
    BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffectRadius(effectPosition, 20, QColor(115,18,0,64));
    */
    BattleDialogModelEffect* effect = createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Radius, 20, 0, QColor(115,18,0,64), QString());

    _contextMenuItem = addEffect(effect);
    if(_contextMenuItem)
    {
        editItem();
        _contextMenuItem = nullptr;
    }
}

void BattleDialogGraphicsScene::addEffectCone()
{
    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to create cone effect, no model exists.";
        return;
    }

    /*
    BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Cone);
    if(!effect)
        return;

    effect->setPosition(_mouseDownPos.x(), _mouseDownPos.y());
    */
    // BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffectCone(_mouseDownPos, 20, QColor(115,18,0,64));
    BattleDialogModelEffect* effect = createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Cone, 20, 0, QColor(115,18,0,64), QString());

    _contextMenuItem = addEffect(effect);
    if(_contextMenuItem)
    {
        editItem();
        _contextMenuItem = nullptr;
    }
}

void BattleDialogGraphicsScene::addEffectCube()
{
    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to create cube effect, no model exists.";
        return;
    }

    /*
    BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Cube);
    if(!effect)
        return;

    qreal scaledSize = static_cast<qreal>(effect->getSize()) * _model->getGridScale() / 5.0;
    effect->setPosition(_mouseDownPos.x() - (scaledSize / 2.0), _mouseDownPos.y() - (scaledSize / 2.0));
    */

    /*
    qreal scaledHalfSize = 20.0 * _model->getGridScale() / (5.0 * 2.0);
    QPointF effectPosition = _mouseDownPos - QPointF(scaledHalfSize, scaledHalfSize);
    BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffectCube(effectPosition, 20, QColor(115,18,0,64));
    */
    BattleDialogModelEffect* effect = createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Cube, 20, 0, QColor(115,18,0,64), QString());

    _contextMenuItem = addEffect(effect);
    if(_contextMenuItem)
    {
        editItem();
        _contextMenuItem = nullptr;
    }
}

void BattleDialogGraphicsScene::addEffectLine()
{
    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to create a line effect, no model exists.";
        return;
    }

    /*
    BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Line);
    if(!effect)
        return;

    effect->setPosition(_mouseDownPos.x(), _mouseDownPos.y());
    */

    // BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffectLine(_mouseDownPos, 20, 5, QColor(115,18,0,64));
    BattleDialogModelEffect* effect = createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Line, 20, 5, QColor(115,18,0,64), QString());

    _contextMenuItem = addEffect(effect);
    if(_contextMenuItem)
    {
        editItem();
        _contextMenuItem = nullptr;
    }
}

void BattleDialogGraphicsScene::castSpell()
{
    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to cast a spell, no model exists.";
        return;
    }

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
        qDebug() << "[Battle Dialog Scene] Spell cast aborted: spell selection dialog cancelled.";
        return;
    }

    Spell* spell = Spellbook::Instance()->getSpell(selectedSpell);
    if(!spell)
    {
        qDebug() << "[Battle Dialog Scene] Spell cast aborted: not able to find selected spell in the Spellbook.";
        return;
    }

    BattleDialogModelEffect* effect = createEffect(spell->getEffectType(),
                                                   spell->getEffectSize().height(),
                                                   spell->getEffectSize().width(),
                                                   spell->getEffectColor(),
                                                   spell->getEffectToken());
    if(!effect)
        return;

    _contextMenuItem = addEffect(effect);
    if(_contextMenuItem)
    {
        editItem();
        _contextMenuItem = nullptr;
    }
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

    //QAbstractGraphicsShapeItem* abstractShape = dynamic_cast<QAbstractGraphicsShapeItem*>(_contextMenuItem);
    QGraphicsItem* abstractShape = _contextMenuItem;
    if(!abstractShape)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to edit an item that is not an effect! ";
        return;
    }

    BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(abstractShape);
    if(!effect)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to edit item, no model data available! " << abstractShape;
        return;
    }

    BattleDialogEffectSettings* settings = effect->getEffectEditor();
    if(!settings)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to edit item, not effect editor available for this effect: " << abstractShape;
        return;
    }

    settings->exec();
    if(settings->result() == QDialog::Accepted)
    {
        qDebug() << "[Battle Dialog Scene] Applying effect settings for effect " << abstractShape;

        settings->copyValues(*effect);
        effect->applyEffectValues(*abstractShape, _model->getGridScale());
        emit effectChanged(abstractShape);
    }
    settings->deleteLater();
}

void BattleDialogGraphicsScene::rollItem()
{
    qDebug() << "[Battle Dialog Scene] Roll Item triggered for " << _contextMenuItem;
    if(_contextMenuItem)
    {
        /*
        QAbstractGraphicsShapeItem* abstractShape = dynamic_cast<QAbstractGraphicsShapeItem*>(_contextMenuItem);
        if(abstractShape)
            emit applyEffect(abstractShape);
            */
        emit applyEffect(_contextMenuItem);
    }
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

    BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(_contextMenuItem);
    if(!effect)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to delete item, no model data available! " << _contextMenuItem;
        return;
    }

    QMessageBox::StandardButton result = QMessageBox::critical(nullptr, QString("Confirm Delete Effect"), QString("Are you sure you wish to delete this effect?"), QMessageBox::Yes | QMessageBox::No);
    if(result == QMessageBox::Yes)
    {
        qDebug() << "[Battle Dialog Scene] confirmed deleting effect " << effect;
        _model->removeEffect(effect);
        _itemList.removeOne(_contextMenuItem);
        if(_mouseDownItem == _contextMenuItem)
        {
            _mouseDown = false;
            _mouseDownItem = nullptr;
        }
        emit effectRemoved(_contextMenuItem);
        delete _contextMenuItem;
        _contextMenuItem = nullptr;
    }
}

void BattleDialogGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(!mouseEvent)
        return;

    BattleDialogGraphicsSceneMouseHandlerBase* mouseHandler = getMouseHandler();
    if(mouseHandler)
    {
        if(!mouseHandler->mouseDoubleClickEvent(mouseEvent))
            return;
    }

    qDebug() << "[Battle Dialog Scene] mouse double click default handling triggered " << mouseEvent;
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

    BattleDialogGraphicsSceneMouseHandlerBase* mouseHandler = getMouseHandler();
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

    BattleDialogGraphicsSceneMouseHandlerBase* mouseHandler = getMouseHandler();
    if(mouseHandler)
    {
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

    BattleDialogGraphicsSceneMouseHandlerBase* mouseHandler = getMouseHandler();
    if(mouseHandler)
    {
        if(!mouseHandler->mouseReleaseEvent(mouseEvent))
            return;
    }

    // If the function reaches this point, default handling is expected
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

BattleDialogModelEffect* BattleDialogGraphicsScene::createEffect(int type, int size, int width, const QColor& color, const QString& filename)
{
    BattleDialogModelEffect* result = nullptr;
    qreal scaledHalfSize;
    QPointF effectPosition;
    switch(type)
    {
        case BattleDialogModelEffect::BattleDialogModelEffect_Radius:
            scaledHalfSize = static_cast<qreal>(size) * 2.0 * _model->getGridScale() / (5.0 * 2.0);
            effectPosition = _mouseDownPos - QPointF(scaledHalfSize, scaledHalfSize);
            result = BattleDialogModelEffectFactory::createEffectRadius(effectPosition, size, color);
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Cone:
            result = BattleDialogModelEffectFactory::createEffectCone(_mouseDownPos, size, color);
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Cube:
            scaledHalfSize = static_cast<qreal>(size) * _model->getGridScale() / (5.0 * 2.0);
            effectPosition = _mouseDownPos - QPointF(scaledHalfSize, scaledHalfSize);
            result = BattleDialogModelEffectFactory::createEffectCube(effectPosition, size, color);
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Line:
            result = BattleDialogModelEffectFactory::createEffectLine(_mouseDownPos, size, width, color);
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Object:
            result = BattleDialogModelEffectFactory::createEffectObject(_mouseDownPos, QSize(size, width), filename);
            break;
        default:
            break;
    }

    return result;
}

QGraphicsItem* BattleDialogGraphicsScene::addEffect(BattleDialogModelEffect* effect)
{
    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to add new scene effect, no model exists.";
        return nullptr;
    }

    if(!effect)
        return nullptr;

    _model->appendEffect(effect);

    return addEffectShape(*effect);
}

QGraphicsItem* BattleDialogGraphicsScene::addEffectShape(BattleDialogModelEffect& effect)
{
    if(!_model)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to add a new effect shape, no model exists.";
        return nullptr;
    }

    QGraphicsItem* shape = effect.createEffectShape(_model->getGridScale());
    if(shape)
    {
        addItem(shape);
        _itemList.append(shape);
    }

    return shape;
}

BattleDialogGraphicsSceneMouseHandlerBase* BattleDialogGraphicsScene::getMouseHandler()
{
    switch(_inputMode)
    {
        case DMHelper::BattleFrameState_FoWEdit:
            return &_rawMouseHandler;
        case DMHelper::BattleFrameState_Pointer:
            return &_pointerMouseHandler;
        case DMHelper::BattleFrameState_Distance:
            return &_distanceMouseHandler;
        case DMHelper::BattleFrameState_CameraEdit:
            return &_cameraMouseHandler;
        case DMHelper::BattleFrameState_CombatantEdit:
            return &_combatantMouseHandler;
        case DMHelper::BattleFrameState_ZoomSelect:
        case DMHelper::BattleFrameState_CameraSelect:
        case DMHelper::BattleFrameState_FoWSelect:
//            return &_selectMouseHandler;
            return &_combatantMouseHandler;
        default:
            return nullptr;
    }
}
