#include "battledialoggraphicsscene.h"
#include "battledialogeffectsettings.h"
#include "battledialogmodel.h"
#include "battledialogmodeleffect.h"
#include "battledialogmodeleffectfactory.h"
#include "grid.h"
#include <QMenu>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QLine>
#include <QtMath>
#include <QMessageBox>
#include <QDebug>

// TODO: adjust grid offsets to really match resized battle contents.

// Uncomment this to log all mouse movement actions
//#define BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE

BattleDialogGraphicsScene::BattleDialogGraphicsScene(BattleDialogModel& model, QObject *parent) :
    QGraphicsScene(parent),
    _contextMenuItem(nullptr),
    _grid(nullptr),
    _model(model),
    _itemList(),
    _mouseDown(false),
    _mouseDownPos(),
    _mouseDownItem(nullptr),
    _distanceShown(false),
    _distanceLine(nullptr),
    _distanceText(nullptr)
{
}

BattleDialogGraphicsScene::~BattleDialogGraphicsScene()
{
}

void BattleDialogGraphicsScene::createBattleContents(const QRect& rect)
{
    if((_grid) || (_itemList.count() > 0))
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to create scene contents: " << rect << ". Contents already exist!";
        return;
    }

    qDebug() << "[Battle Dialog Scene] Creating scene contents: " << rect;
    _grid = new Grid(*this, rect);
    _grid->rebuildGrid(_model);

    QList<BattleDialogModelEffect*> effects = _model.getEffectList();
    for(BattleDialogModelEffect* effect : effects)
    {
        if(effect)
            addEffectShape(*effect);
    }
}

void BattleDialogGraphicsScene::resizeBattleContents(const QRect& rect)
{
    qDebug() << "[Battle Dialog Scene] Resizing scene contents";

    if(_grid)
    {
        qDebug() << "[Battle Dialog Scene]     Resizing grid, grid shape = " << rect;
        _grid->setGridShape(rect);
        _grid->rebuildGrid(_model);
    }

    for(QGraphicsItem* item : _itemList)
    {
        if(item)
        {
            QPoint mapPos = item->pos().toPoint() + _model.getPreviousMapRect().topLeft();
            item->setPos(mapPos - _model.getMapRect().topLeft());
            _model.getEffectById(item->data(0).toInt())->setPosition(item->pos());
            qDebug() << "[Battle Dialog Scene]     Setting position for item " << item << " to " << item->pos();
        }
    }
}

void BattleDialogGraphicsScene::updateBattleContents()
{
    qDebug() << "[Battle Dialog Scene] Updating scene contents";

    if(_grid)
    {
        qDebug() << "[Battle Dialog Scene]     Rebuilding grid, grid scale = " << _model.getGridScale();
        _grid->rebuildGrid(_model);
    }

    for(QGraphicsItem* item : _itemList)
    {
        if(item)
        {
            qreal newScale = static_cast<qreal>(_model.getEffectById(item->data(0).toInt())->getSize()) * static_cast<qreal>(_model.getGridScale()) / 500.0;
            qDebug() << "[Battle Dialog Scene]     Setting scale for item " << item << " to " << newScale;
            qreal oldScale = item->scale();
            item->setScale(newScale);
            item->setPos(item->pos() * newScale/oldScale);
            _model.getEffectById(item->data(0).toInt())->setPosition(item->pos());
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
}

void BattleDialogGraphicsScene::setEffectVisibility(bool visible)
{
    qDebug() << "[Battle Dialog Scene] Setting effect visibility to " << visible;

    for(QGraphicsItem* item : _itemList)
    {
        item->setVisible(visible);
    }
}

void BattleDialogGraphicsScene::setGridVisibility(bool visible)
{
    qDebug() << "[Battle Dialog Scene] Setting grid visibility to " << visible;

    if(_grid)
    {
        _grid->setGridVisible(visible);
    }
}

QList<QGraphicsItem*> BattleDialogGraphicsScene::getEffectItems() const
{
    return _itemList;
}

void BattleDialogGraphicsScene::setShowDistance(bool showDistance)
{
    _distanceShown = showDistance;
}

void BattleDialogGraphicsScene::editItem()
{
    if(!_contextMenuItem)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to edit item, no context menu item known! ";
        return;
    }

    QAbstractGraphicsShapeItem* abstractShape = dynamic_cast<QAbstractGraphicsShapeItem*>(_contextMenuItem);
    if(!abstractShape)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to edit an item that is not an effect! ";
        return;
    }

    BattleDialogModelEffect* effect = _model.getEffectById(abstractShape->data(0).toInt());
    if(!effect)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to edit item, no model data available! " << abstractShape;
        return;
    }

    BattleDialogEffectSettings settings(*effect);
    settings.exec();
    if(settings.result() == QDialog::Accepted)
    {
        qDebug() << "[Battle Dialog Scene] Applying effect settings for effect " << abstractShape;

        settings.copyValues(*effect);
        effect->applyEffectValues(*abstractShape, _model.getGridScale());
        //applyEffectValues(*abstractShape, *effect);
        emit effectChanged(abstractShape);
    }
}

void BattleDialogGraphicsScene::rollItem()
{
    qDebug() << "[Battle Dialog Scene] Roll Item triggered for " << _contextMenuItem;
    if(_contextMenuItem)
    {
        QAbstractGraphicsShapeItem* abstractShape = dynamic_cast<QAbstractGraphicsShapeItem*>(_contextMenuItem);
        if(abstractShape)
            emit applyEffect(abstractShape);
    }
}

void BattleDialogGraphicsScene::deleteItem()
{
    if(!_contextMenuItem)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to delete item, no context menu item known! ";
        return;
    }

    BattleDialogModelEffect* effect = _model.getEffectById(_contextMenuItem->data(0).toInt());
    if(!effect)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: attempted to delete item, no model data available! " << _contextMenuItem;
        return;
    }

    QMessageBox::StandardButton result = QMessageBox::critical(nullptr, QString("Confirm Delete Effect"), QString("Are you sure you wish to delete this effect?"), QMessageBox::Yes | QMessageBox::No);
    if(result == QMessageBox::Yes)
    {
        qDebug() << "[Battle Dialog Scene] confirmed deleting effect " << effect;
        _model.removeEffect(effect);
        _itemList.removeOne(_contextMenuItem);
        if(_mouseDownItem == _contextMenuItem)
        {
            _mouseDown = false;
            _mouseDownItem = nullptr;
        }
        delete _contextMenuItem;
        _contextMenuItem = nullptr;

        emit effectChanged(nullptr);
    }
}

void BattleDialogGraphicsScene::addEffectRadius()
{
    BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Radius);
    if(!effect)
        return;

    qreal scaledSize = static_cast<qreal>(effect->getSize()) * _model.getGridScale() / 5.0;
    effect->setPosition(_mouseDownPos.x() - (scaledSize / 2.0),_mouseDownPos.y() - (scaledSize / 2.0));

    addEffect(effect);
}

void BattleDialogGraphicsScene::addEffectCone()
{
    BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Cone);
    if(!effect)
        return;

    effect->setPosition(_mouseDownPos.x(), _mouseDownPos.y());

    addEffect(effect);
}

void BattleDialogGraphicsScene::addEffectCube()
{
    BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Cube);
    if(!effect)
        return;

    qreal scaledSize = static_cast<qreal>(effect->getSize()) * _model.getGridScale() / 5.0;
    effect->setPosition(_mouseDownPos.x() - (scaledSize / 2.0), _mouseDownPos.y() - (scaledSize / 2.0));

    addEffect(effect);
}

void BattleDialogGraphicsScene::addEffectLine()
{
    BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Line);
    if(!effect)
        return;

    effect->setPosition(_mouseDownPos.x(), _mouseDownPos.y());

    addEffect(effect);
}

void BattleDialogGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(!mouseEvent)
        return;

    qDebug() << "[Battle Dialog Scene] doubleclick detected at " << mouseEvent->scenePos();
    if(mouseEvent->button() == Qt::LeftButton)
    {
        QGraphicsItem* item = findTopObject(mouseEvent->scenePos());

        if((item)&&(item->data(0).toInt() > 0))
        {
            qDebug() << "[Battle Dialog Scene] doubleclick identified on item " << item->data(0).toInt();
            _contextMenuItem = item;
            editItem();
            _contextMenuItem = nullptr;
        }

        _mouseDown = false;
        _mouseDownItem = nullptr;
        mouseEvent->accept();
        return;
    }

    qDebug() << "[Battle Dialog Scene] mouse double click default handling triggered " << mouseEvent;
    // If the function reaches this point, default handling is expected
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void BattleDialogGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(!mouseEvent)
        return;

    if(_distanceShown)
    {
        if((!_distanceLine) || (!_distanceText))
            return;

        QLineF line = _distanceLine->line();
        line.setP2(mouseEvent->scenePos());
        _distanceLine->setLine(line);
        QString distanceText = QString::number(5.0 * line.length() / _model.getGridScale(), 'f', 1);
        _distanceText->setText(distanceText);
        _distanceText->setPos(line.center());
        emit distanceChanged(distanceText);
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
        qDebug() << "[Battle Dialog Scene] line set to " << line << ", text to " << _distanceText->text();
#endif
        mouseEvent->accept();
        return;
    }

    QAbstractGraphicsShapeItem* abstractShape = dynamic_cast<QAbstractGraphicsShapeItem*>(_mouseDownItem);

    if(mouseEvent->buttons() & Qt::RightButton)
    {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
        qDebug() << "[Battle Dialog Scene] right button mouse move detected on " << abstractShape << " at " << mouseEvent->scenePos() << " mousedown=" << _mouseDown;
#endif

        if((_mouseDown) && (abstractShape))
        {
            // |A·B| = |A| |B| COS(θ)
            // |A×B| = |A| |B| SIN(θ)
            QPointF eventPos = mouseEvent->scenePos() - _mouseDownItem->scenePos();
            qreal cross = _mouseDownPos.x()*eventPos.y()-_mouseDownPos.y()*eventPos.x();
            qreal dot = _mouseDownPos.x()*eventPos.x()+_mouseDownPos.y()*eventPos.y();
            qreal angle = qRadiansToDegrees(qAtan2(cross,dot));
            _mouseDownItem->setRotation(_previousRotation + angle);
            BattleDialogModelEffect* effect = _model.getEffectById(_mouseDownItem->data(0).toInt());
            if(effect)
                effect->setRotation(_previousRotation + angle);
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
            else
                qDebug() << "[Battle Dialog Scene] ERROR: unable to find effect model data for rotation" << _mouseDownItem;
#endif
            emit effectChanged(abstractShape);
        }

        mouseEvent->accept();
        return;
    }
    else if(mouseEvent->buttons() & Qt::LeftButton)
    {
        if(abstractShape)
        {
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
            qDebug() << "[Battle Dialog Scene] left button mouse move detected on " << abstractShape << " at " << mouseEvent->scenePos() << " mousedown=" << _mouseDown;
#endif
            BattleDialogModelEffect* effect = _model.getEffectById(abstractShape->data(0).toInt());
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
                    return;
            }
        }
    }

#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
    qDebug() << "[Battle Dialog Scene] mouse move default handling triggered " << mouseEvent;
#endif
    // If the function reaches this point, default handling (ie drag and move) is expected
    QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void BattleDialogGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(!mouseEvent)
        return;

    if(_distanceShown)
    {
        if(_distanceLine)
            delete _distanceLine;
        _distanceLine = addLine(QLineF(mouseEvent->scenePos(), mouseEvent->scenePos()));

        if(_distanceText)
            delete _distanceText;
        _distanceText = addSimpleText(QString("0"));
        _distanceText->setBrush(QBrush(Qt::yellow));
        _distanceText->setPos(mouseEvent->scenePos());
        //qDebug() << "[Battle Dialog Scene] line set to " << _distanceLine->line() << ", text to " << _distanceText->text();
        mouseEvent->accept();
        return;
    }

    QGraphicsItem* item = findTopObject(mouseEvent->scenePos());
    QAbstractGraphicsShapeItem* abstractShape = dynamic_cast<QAbstractGraphicsShapeItem*>(item);

    qDebug() << "[Battle Dialog Scene] mouse press at " << mouseEvent->scenePos() << " item " << item << " shape " << abstractShape;

    if(item)
    {
        if(item->data(0).toInt() > 0)
        {
            _mouseDown = true;
            _mouseDownPos = mouseEvent->scenePos() - item->scenePos();
            _mouseDownItem = item;

            if(mouseEvent->button() == Qt::RightButton)
            {
                _previousRotation = _mouseDownItem->rotation();
                 qDebug() << "[Battle Dialog Scene] right mouse down on " << _mouseDownItem << " identified: pos=" << _mouseDownPos << ", rot=" << _previousRotation;
                mouseEvent->accept();
                return;
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
        return;
    }

    qDebug() << "[Battle Dialog Scene] mouse press default handling triggered " << mouseEvent;
    // If the function reaches this point, default handling is expected
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void BattleDialogGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(!mouseEvent)
        return;

    if(_distanceShown)
    {
        delete _distanceLine; _distanceLine = nullptr;
        delete _distanceText; _distanceText = nullptr;
        mouseEvent->accept();
        return;
    }

    if(mouseEvent->button() == Qt::RightButton)
    {
        QGraphicsItem* item = findTopObject(mouseEvent->scenePos());
        qDebug() << "[Battle Dialog Scene] right mouse released at " << mouseEvent->scenePos() << " for item " << item;

        QWidget* parentWidget = nullptr;
        if(views().count() > 0 )
            parentWidget = views().first();
        QMenu menu(parentWidget);
        if((item)&&(item->data(0).toInt() > 0))
        {
            qDebug() << "[Battle Dialog Scene] right click identified on effect " << item;
            if(_mouseDownPos == mouseEvent->scenePos() - item->scenePos())
            {
                _contextMenuItem = item;

                QAction* edtItem = new QAction(QString("Edit..."), &menu);
                connect(edtItem,SIGNAL(triggered()),this,SLOT(editItem()));
                menu.addAction(edtItem);

                QAction* rollItem = new QAction(QString("Roll..."), &menu);
                connect(rollItem,SIGNAL(triggered()),this,SLOT(rollItem()));
                menu.addAction(rollItem);

                QAction* deleteItem = new QAction(QString("Delete..."), &menu);
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
    // If the function reaches this point, default handling is expected
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void BattleDialogGraphicsScene::addEffect(BattleDialogModelEffect* effect)
{
    if(!effect)
        return;

    _model.appendEffect(effect);

    _contextMenuItem =addEffectShape(*effect);
    editItem();
    _contextMenuItem = nullptr;
}

QAbstractGraphicsShapeItem* BattleDialogGraphicsScene::addEffectShape(BattleDialogModelEffect& effect)
{
    QAbstractGraphicsShapeItem* shape = effect.createEffectShape(_model.getGridScale());
    if(shape)
    {
        addItem(shape);
        _itemList.append(shape);
    }

    return shape;
}

//QAbstractGraphicsShapeItem* BattleDialogGraphicsScene::findTopObject(const QPointF &pos)
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
            qDebug() << "[Battle Dialog Scene] find top object found selectable object: " << result;
            return result;
        }
    }

    // If we get here, nothing selectable was found, so return the top-most item
    //return dynamic_cast<QAbstractGraphicsShapeItem*>(itemList.first());
    //result = dynamic_cast<QGraphicsItem*>(itemList.first());
    // WRONG - return no item - the callers assume a returned item is selectable!
    result = nullptr;
    qDebug() << "[Battle Dialog Scene] find top object found, but not selectable: " << result;
    return result;
}
