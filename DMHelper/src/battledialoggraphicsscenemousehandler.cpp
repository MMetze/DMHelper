#include "battledialoggraphicsscenemousehandler.h"
#include "battledialoggraphicsscene.h"
#include "battledialogmodel.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QBrush>
#include <QLine>
#include <QtMath>

BattleDialogGraphicsSceneMouseHandlerBase::BattleDialogGraphicsSceneMouseHandlerBase(BattleDialogGraphicsScene& scene) :
    QObject(),
    _scene(scene)
{
}

BattleDialogGraphicsSceneMouseHandlerBase::~BattleDialogGraphicsSceneMouseHandlerBase()
{
}

bool BattleDialogGraphicsSceneMouseHandlerBase::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    Q_UNUSED(mouseEvent);
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerBase::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    Q_UNUSED(mouseEvent);
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerBase::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    Q_UNUSED(mouseEvent);
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerBase::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    Q_UNUSED(mouseEvent);
    return false;
}


/******************************************************************************************************/


BattleDialogGraphicsSceneMouseHandlerDistance::BattleDialogGraphicsSceneMouseHandlerDistance(BattleDialogGraphicsScene& scene) :
    BattleDialogGraphicsSceneMouseHandlerBase(scene),
    _heightDelta(0.0),
    _distanceLine(nullptr),
    _distanceText(nullptr)
{
}

BattleDialogGraphicsSceneMouseHandlerDistance::~BattleDialogGraphicsSceneMouseHandlerDistance()
{
}

bool BattleDialogGraphicsSceneMouseHandlerDistance::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if((!_distanceLine) || (!_distanceText) || (!_scene.getModel()) || (_scene.getModel()->getGridScale() <= 0.0))
        return false;

    QLineF line = _distanceLine->line();
    line.setP2(mouseEvent->scenePos());
    _distanceLine->setLine(line);
    qreal lineDistance = 5.0 * line.length() / _scene.getModel()->getGridScale();
    QString distanceText;
    if(_heightDelta == 0.0)
    {
        distanceText = QString::number(lineDistance, 'f', 1);
    }
    else
    {
        qreal diagonal = qSqrt((lineDistance*lineDistance) + (_heightDelta*_heightDelta));
        distanceText = QString::number(diagonal, 'f', 1) + QChar::LineFeed + QString("(") + QString::number(lineDistance, 'f', 1) + QString (")");
    }
    _distanceText->setText(distanceText);
    _distanceText->setPos(line.center());
    emit distanceChanged(distanceText);
#ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
    qDebug() << "[Battle Dialog Scene] line set to " << line << ", text to " << _distanceText->text();
#endif
    mouseEvent->accept();
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerDistance::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(!_scene.getModel())
        return false;

    if(_distanceLine)
        delete _distanceLine;
    _distanceLine = _scene.addLine(QLineF(mouseEvent->scenePos(), mouseEvent->scenePos()));

    if(_distanceText)
        delete _distanceText;
    _distanceText = _scene.addSimpleText(QString("0"));
    _distanceText->setBrush(QBrush(Qt::yellow));
    _distanceText->setPos(mouseEvent->scenePos());
    //qDebug() << "[BattleDialogGraphicsSceneMouseHandlerDistance] line set to " << _distanceLine->line() << ", text to " << _distanceText->text();
    mouseEvent->accept();
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerDistance::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    delete _distanceLine; _distanceLine = nullptr;
    delete _distanceText; _distanceText = nullptr;
    mouseEvent->accept();
    return false;
}

void BattleDialogGraphicsSceneMouseHandlerDistance::setHeightDelta(qreal heightDelta)
{
    _heightDelta = heightDelta;
}


/******************************************************************************************************/


BattleDialogGraphicsSceneMouseHandlerPointer::BattleDialogGraphicsSceneMouseHandlerPointer(BattleDialogGraphicsScene& scene) :
    BattleDialogGraphicsSceneMouseHandlerBase(scene)
{
}

BattleDialogGraphicsSceneMouseHandlerPointer::~BattleDialogGraphicsSceneMouseHandlerPointer()
{
}

bool BattleDialogGraphicsSceneMouseHandlerPointer::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    _scene.setPointerPos(mouseEvent->scenePos());
    emit pointerMoved(mouseEvent->scenePos());
    mouseEvent->ignore();
    return true;
}

bool BattleDialogGraphicsSceneMouseHandlerPointer::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    mouseEvent->ignore();
    return true;
}

bool BattleDialogGraphicsSceneMouseHandlerPointer::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    mouseEvent->ignore();
    return true;
}



/******************************************************************************************************/


BattleDialogGraphicsSceneMouseHandlerRaw::BattleDialogGraphicsSceneMouseHandlerRaw(BattleDialogGraphicsScene& scene) :
    BattleDialogGraphicsSceneMouseHandlerBase(scene)
{
}

BattleDialogGraphicsSceneMouseHandlerRaw::~BattleDialogGraphicsSceneMouseHandlerRaw()
{
}

bool BattleDialogGraphicsSceneMouseHandlerRaw::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    emit rawMouseMove(mouseEvent->scenePos());
    mouseEvent->accept();
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerRaw::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    emit rawMousePress(mouseEvent->scenePos());
    mouseEvent->accept();
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerRaw::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    emit rawMouseRelease(mouseEvent->scenePos());
    mouseEvent->accept();
    return false;
}


/******************************************************************************************************/


BattleDialogGraphicsSceneMouseHandlerCamera::BattleDialogGraphicsSceneMouseHandlerCamera(BattleDialogGraphicsScene& scene) :
    BattleDialogGraphicsSceneMouseHandlerBase(scene)
{
}

BattleDialogGraphicsSceneMouseHandlerCamera::~BattleDialogGraphicsSceneMouseHandlerCamera()
{
}

bool BattleDialogGraphicsSceneMouseHandlerCamera::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    return checkMouseEvent(mouseEvent);
}

bool BattleDialogGraphicsSceneMouseHandlerCamera::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    return checkMouseEvent(mouseEvent);
}

bool BattleDialogGraphicsSceneMouseHandlerCamera::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    return checkMouseEvent(mouseEvent);
}

bool BattleDialogGraphicsSceneMouseHandlerCamera::checkMouseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsItem* item = _scene.findTopObject(mouseEvent->scenePos());
    if((item) && (item->zValue() < DMHelper::BattleDialog_Z_Overlay))
    {
        // Ignore any interactions with items other than overlays. The camera rect is set to Z_Overlay when active.
        mouseEvent->ignore();
        return false;
    }
    else
    {
        // Continue with normal processing
        return true;
    }
}


/******************************************************************************************************/


BattleDialogGraphicsSceneMouseHandlerCombatants::BattleDialogGraphicsSceneMouseHandlerCombatants(BattleDialogGraphicsScene& scene) :
    BattleDialogGraphicsSceneMouseHandlerBase(scene)
{
}

BattleDialogGraphicsSceneMouseHandlerCombatants::~BattleDialogGraphicsSceneMouseHandlerCombatants()
{
}

bool BattleDialogGraphicsSceneMouseHandlerCombatants::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    return _scene.handleMouseDoubleClickEvent(mouseEvent);
}

bool BattleDialogGraphicsSceneMouseHandlerCombatants::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    return _scene.handleMouseMoveEvent(mouseEvent);
}

bool BattleDialogGraphicsSceneMouseHandlerCombatants::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    return _scene.handleMousePressEvent(mouseEvent);
}

bool BattleDialogGraphicsSceneMouseHandlerCombatants::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    return _scene.handleMouseReleaseEvent(mouseEvent);
}


/******************************************************************************************************/


BattleDialogGraphicsSceneMouseHandlerMaps::BattleDialogGraphicsSceneMouseHandlerMaps(BattleDialogGraphicsScene& scene) :
    BattleDialogGraphicsSceneMouseHandlerBase(scene)
{
}

BattleDialogGraphicsSceneMouseHandlerMaps::~BattleDialogGraphicsSceneMouseHandlerMaps()
{
}

bool BattleDialogGraphicsSceneMouseHandlerMaps::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    emit mapMouseMove(mouseEvent->scenePos());
    mouseEvent->accept();
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerMaps::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    emit mapMousePress(mouseEvent->scenePos());
    mouseEvent->accept();
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerMaps::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    emit mapMouseRelease(mouseEvent->scenePos());
    mouseEvent->accept();
    return false;
}
