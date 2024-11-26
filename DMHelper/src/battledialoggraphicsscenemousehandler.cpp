#include "battledialoggraphicsscenemousehandler.h"
#include "battledialoggraphicsscene.h"
#include "battledialogmodel.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsSimpleTextItem>
#include <QBrush>
#include <QPen>
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


BattleDialogGraphicsSceneMouseHandlerDistanceBase::BattleDialogGraphicsSceneMouseHandlerDistanceBase(BattleDialogGraphicsScene& scene)  :
    BattleDialogGraphicsSceneMouseHandlerBase(scene),
    _heightDelta(0.0),
    _scale(5.0),
    _color(Qt::yellow),
    _lineType(Qt::SolidLine),
    _lineWidth(1)
{
}

BattleDialogGraphicsSceneMouseHandlerDistanceBase::~BattleDialogGraphicsSceneMouseHandlerDistanceBase()
{
}

void BattleDialogGraphicsSceneMouseHandlerDistanceBase::setHeightDelta(qreal heightDelta)
{
    _heightDelta = heightDelta;
    updateDistance();
}

void BattleDialogGraphicsSceneMouseHandlerDistanceBase::setDistanceScale(int scale)
{
    _scale = scale;
}

void BattleDialogGraphicsSceneMouseHandlerDistanceBase::setDistanceLineColor(const QColor& color)
{
    _color = color;
}

void BattleDialogGraphicsSceneMouseHandlerDistanceBase::setDistanceLineType(int lineType)
{
    _lineType = lineType;
}

void BattleDialogGraphicsSceneMouseHandlerDistanceBase::setDistanceLineWidth(int lineWidth)
{
    _lineWidth = lineWidth;
}

QString BattleDialogGraphicsSceneMouseHandlerDistanceBase::createDistanceString(qreal lineDistance) const
{
    if(_heightDelta == 0.0)
        return QString::number(lineDistance, 'f', 1);

    qreal diagonal = qSqrt((lineDistance*lineDistance) + (_heightDelta*_heightDelta));
    return QString::number(diagonal, 'f', 1) + QChar::LineFeed + QString("(") + QString::number(lineDistance, 'f', 1) + QString (")");
}


/******************************************************************************************************/


BattleDialogGraphicsSceneMouseHandlerDistance::BattleDialogGraphicsSceneMouseHandlerDistance(BattleDialogGraphicsScene& scene) :
    BattleDialogGraphicsSceneMouseHandlerDistanceBase(scene),
    _distanceLine(nullptr),
    _distanceText(nullptr)
{
}

BattleDialogGraphicsSceneMouseHandlerDistance::~BattleDialogGraphicsSceneMouseHandlerDistance()
{
}

void BattleDialogGraphicsSceneMouseHandlerDistance::cleanup()
{
    emit distanceItemChanged(nullptr, nullptr);
    delete _distanceLine; _distanceLine = nullptr;
    delete _distanceText; _distanceText = nullptr;
}

bool BattleDialogGraphicsSceneMouseHandlerDistance::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if((!_distanceLine) || (!_distanceText) || (_scale <= 0.0) || (mouseEvent->buttons() == Qt::NoButton))
        return false;

    QLineF line = _distanceLine->line();
    line.setP2(mouseEvent->scenePos());
    _distanceLine->setLine(line);

    updateDistance();

    mouseEvent->accept();
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerDistance::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(_distanceLine)
        delete _distanceLine;

    _distanceLine = _scene.addLine(QLineF(mouseEvent->scenePos(), mouseEvent->scenePos()), QPen(QBrush(_color), _lineWidth, static_cast<Qt::PenStyle>(_lineType)));
    _distanceLine->setPen(QPen(QBrush(_color), _lineWidth, static_cast<Qt::PenStyle>(_lineType)));
    _distanceLine->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);

    if(_distanceText)
        delete _distanceText;
    _distanceText = _scene.addSimpleText(QString("0"));
    _distanceText->setBrush(QBrush(_color));
    _distanceText->setPos(mouseEvent->scenePos());
    _distanceText->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);

    emit distanceItemChanged(_distanceLine, _distanceText);

    mouseEvent->accept();
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerDistance::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    mouseEvent->accept();
    return false;
}

QGraphicsItem* BattleDialogGraphicsSceneMouseHandlerDistance::getDistanceLine() const
{
    return _distanceLine;
}

QGraphicsSimpleTextItem* BattleDialogGraphicsSceneMouseHandlerDistance::getDistanceText() const
{
    return _distanceText;
}

void BattleDialogGraphicsSceneMouseHandlerDistance::updateDistance()
{
    if((!_distanceLine) || (!_distanceText) || (_scale <= 0.0))
        return;

    QLineF line = _distanceLine->line();
    qreal lineDistance = 5.0 * line.length() / _scale;

    QString distanceText = createDistanceString(lineDistance);

    _distanceText->setText(distanceText);
    _distanceText->setPos(line.center());
    emit distanceChanged(distanceText);
}


/******************************************************************************************************/


BattleDialogGraphicsSceneMouseHandlerFreeDistance::BattleDialogGraphicsSceneMouseHandlerFreeDistance(BattleDialogGraphicsScene& scene) :
    BattleDialogGraphicsSceneMouseHandlerDistanceBase(scene),
    _mouseDownPos(),
    _distancePath(nullptr),
    _distanceText(nullptr)
{
}

BattleDialogGraphicsSceneMouseHandlerFreeDistance::~BattleDialogGraphicsSceneMouseHandlerFreeDistance()
{
}

void BattleDialogGraphicsSceneMouseHandlerFreeDistance::cleanup()
{
    emit distanceItemChanged(nullptr, nullptr);

    delete _distancePath; _distancePath = nullptr;
    delete _distanceText; _distanceText = nullptr;
}

bool BattleDialogGraphicsSceneMouseHandlerFreeDistance::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if((!_distanceText) || (_scale <= 0.0) || (mouseEvent->buttons() == Qt::NoButton))
        return false;

    QPointF scenePos = mouseEvent->scenePos();
    QPainterPath currentPath;
    if(_distancePath)
    {
        currentPath = _distancePath->path();
        currentPath.lineTo(scenePos);
        _distancePath->setPath(currentPath);
    }
    else
    {
        currentPath.moveTo(_mouseDownPos);
        currentPath.lineTo(scenePos);
        _distancePath = _scene.addPath(currentPath, QPen(QBrush(_color), _lineWidth, static_cast<Qt::PenStyle>(_lineType)));
        _distancePath->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
        emit distanceItemChanged(_distancePath, _distanceText);
    }
    _distanceText->setPos(scenePos + QPointF(5.0, 5.0));

    updateDistance();

    mouseEvent->accept();
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerFreeDistance::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    _mouseDownPos = mouseEvent->scenePos();

    _distanceText = _scene.addSimpleText(QString("0"));
    if(_heightDelta != 0.0)
        _distanceText->setText(QString::number(_heightDelta, 'f', 1) + QChar::LineFeed + QString("(0)"));
    _distanceText->setBrush(QBrush(_color));
    QFont textFont = _distanceText->font();
    textFont.setPointSize(DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0] / 20);
    _distanceText->setFont(textFont);
    _distanceText->setPos(_mouseDownPos + QPointF(5.0, 5.0));
    _distanceText->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);

    mouseEvent->accept();
    return false;
}

bool BattleDialogGraphicsSceneMouseHandlerFreeDistance::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    mouseEvent->accept();
    return false;
}

QGraphicsItem* BattleDialogGraphicsSceneMouseHandlerFreeDistance::getDistanceLine() const
{
    return _distancePath;
}

QGraphicsSimpleTextItem* BattleDialogGraphicsSceneMouseHandlerFreeDistance::getDistanceText() const
{
    return _distanceText;
}

void BattleDialogGraphicsSceneMouseHandlerFreeDistance::updateDistance()
{
    if((!_distancePath) || (!_distanceText) || (_scale <= 0.0))
        return;

    qreal lineDistance = 5.0 * _distancePath->path().length() / _scale;
    QString distanceText = createDistanceString(lineDistance);
    _distanceText->setText(distanceText);

    emit distanceChanged(distanceText);
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
