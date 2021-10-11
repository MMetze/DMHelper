#include "mapcontent.h"

int MapContent::_id_global = 1000000;

MapContent::MapContent() :
    _id(createId())
{
}

MapContent::MapContent(const MapContent &obj):
    _id(obj._id)
{
    Q_UNUSED(obj);
}

MapContent::~MapContent()
{
}

int MapContent::getID() const
{
    return _id;
}

int MapContent::createId()
{
    return _id_global++;
}




MapMarker::MapMarker(const QPoint& position, const QString& title, const QString& description, const QUuid& encounter) :
    MapContent(),
    _position(position),
    _title(title),
    _description(description),
    _encounter(encounter)
{
}

MapMarker::MapMarker(const MapMarker &obj) :
    MapContent(obj),
    _position(obj.position()),
    _title(obj.title()),
    _description(obj.description()),
    _encounter(obj.encounter())
{
}

MapMarker::~MapMarker()
{
}

QPoint MapMarker::position() const
{
    return _position;
}

QString MapMarker::title() const
{
    return _title;
}

QString MapMarker::description() const
{
    return _description;
}

const QUuid& MapMarker::encounter() const
{
    return _encounter;
}

void MapMarker::setPosition(const QPoint& position)
{
    _position = position;
}

void MapMarker::setX(int x)
{
    _position.setX(x);
}

void MapMarker::setY(int y)
{
    _position.setY(y);
}

void MapMarker::setTitle(const QString& title)
{
    _title = title;
}

void MapMarker::setDescription(const QString& description)
{
    _description = description;
}

void MapMarker::setEncounter(const QUuid& encounter)
{
    _encounter = encounter;
}




MapEdit::MapEdit() :
    MapContent()
{
}

MapEdit::MapEdit(const MapEdit &obj) :
    MapContent(obj)
{
}

MapEdit::~MapEdit()
{
}




MapEditFill::MapEditFill(const QColor& color) :
    MapEdit(),
    _color(color)
{
}

MapEditFill::MapEditFill(const MapEditFill &obj) :
    MapEdit(obj),
    _color(obj.color())
{
}

MapEditFill::~MapEditFill()
{
}

QColor MapEditFill::color() const
{
    return _color;
}

void MapEditFill::setColor(const QColor& color)
{
    _color = color;
}

void MapEditFill::setRed(int red)
{
    _color.setRed(red);
}

void MapEditFill::setGreen(int green)
{
    _color.setGreen(green);
}

void MapEditFill::setBlue(int blue)
{
    _color.setBlue(blue);
}

void MapEditFill::setAlpha(int alpha)
{
    _color.setAlpha(alpha);
}


MapDraw::MapDraw(int radius, int brushType, bool erase, bool smooth) :
    MapEdit(),
    _radius(radius),
    _brushType(brushType),
    _erase(erase),
    _smooth(smooth)
{
}

MapDraw::MapDraw(const MapDraw &obj) :
    MapEdit(obj),
    _radius(obj.radius()),
    _brushType(obj.brushType()),
    _erase(obj.erase()),
    _smooth(obj.smooth())
{
}

MapDraw::~MapDraw()
{
}

int MapDraw::radius() const
{
    return _radius;
}

int MapDraw::brushType() const
{
    return _brushType;
}

bool MapDraw::erase() const
{
    return _erase;
}

bool MapDraw::smooth() const
{
    return _smooth;
}

void MapDraw::setRadius(int radius)
{
    _radius = radius;
}

void MapDraw::setBrushType(int brushType)
{
    _brushType = brushType;
}

void MapDraw::setErase(bool erase)
{
    _erase = erase;
}

void MapDraw::setSmooth(bool smooth)
{
    _smooth = smooth;
}





MapDrawPoint::MapDrawPoint(int radius, int brushType, bool erase, bool smooth, const QPoint& point) :
    MapDraw(radius, brushType, erase, smooth),
    _point(point)
{
}

MapDrawPoint::MapDrawPoint(const MapDrawPoint &obj) :
    MapDraw(obj),
    _point(obj.point())
{
}

MapDrawPoint::~MapDrawPoint()
{
}

QPoint MapDrawPoint::point() const
{
    return _point;
}

void MapDrawPoint::setPoint(const QPoint& point)
{
    _point = point;
}

void MapDrawPoint::setX(int x)
{
    _point.setX(x);
}

void MapDrawPoint::setY(int y)
{
    _point.setY(y);
}




MapDrawPath::MapDrawPath() :
    MapDraw(),
    _points()
{
}

MapDrawPath::MapDrawPath(int radius, int brushType, bool erase, bool smooth, const QPoint& point) :
    MapDraw(radius, brushType, erase, smooth),
    _points()
{
    _points.append(point);
}

MapDrawPath::MapDrawPath(const MapDrawPath &obj) :
    MapDraw(obj),
    _points()
{
    _points.append(obj.points());
}

MapDrawPath::~MapDrawPath()
{
}

void MapDrawPath::addPoint(const QPoint& point)
{
    _points.append(point);
}

QList<QPoint> MapDrawPath::points() const
{
    return _points;
}



MapEditShape::MapEditShape(const QRect& rect, bool erase, bool smooth) :
    MapEdit(),
    _rect(rect),
    _erase(erase),
    _smooth(smooth)
{
}

MapEditShape::MapEditShape(const MapEditShape &obj) :
    MapEdit(obj),
    _rect(obj._rect),
    _erase(obj._erase),
    _smooth(obj._smooth)
{
}

MapEditShape::~MapEditShape()
{
}

QRect MapEditShape::rect() const
{
    return _rect;
}

bool MapEditShape::erase() const
{
    return _erase;
}

bool MapEditShape::smooth() const
{
    return _smooth;
}

void MapEditShape::setRect(const QRect& rect)
{
    _rect = rect;
}

void MapEditShape::setErase(bool erase)
{
    _erase = erase;
}

void MapEditShape::setSmooth(bool smooth)
{
    _smooth = smooth;
}
