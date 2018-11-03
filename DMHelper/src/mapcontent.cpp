#include "mapcontent.h"

int MapContent::_id_global = 1000000;

MapContent::MapContent() :
    _id(createId())
{
}

MapContent::MapContent(const MapContent &obj)
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




MapMarker::MapMarker(const QPoint& position, const QString& title, const QString& description) :
    MapContent(),
    _position(position),
    _title(title),
    _description(description)
{
}

MapMarker::MapMarker(const MapMarker &obj) :
    MapContent(obj),
    _position(obj.position()),
    _title(obj.title()),
    _description(obj.description())
{
}

MapMarker::~MapMarker()
{
}

const QPoint& MapMarker::position() const
{
    return _position;
}

const QString& MapMarker::title() const
{
    return _title;
}

const QString& MapMarker::description() const
{
    return _description;
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

void MapMarker::outputXML(QDomElement &element) const
{
    element.setAttribute( "x", _position.x() );
    element.setAttribute( "y", _position.y() );
    element.setAttribute( "title", _title );
    element.setAttribute( "description", _description );
}

void MapMarker::inputXML(const QDomElement &element)
{
    setX(element.attribute(QString("x")).toInt());
    setY(element.attribute(QString("y")).toInt());
    setTitle(element.attribute( QString("title") ));
    setDescription(element.attribute( QString("description") ));
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

const QColor& MapEditFill::color() const
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


MapDraw::MapDraw(int radius, int brushType, bool erase) :
    MapEdit(),
    _radius(radius),
    _brushType(brushType),
    _erase(erase)
{
}

MapDraw::MapDraw(const MapDraw &obj) :
    MapEdit(obj),
    _radius(obj.radius()),
    _brushType(obj.brushType()),
    _erase(obj.erase())
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
    _erase= erase;
}





MapDrawPoint::MapDrawPoint(int radius, int brushType, bool erase, const QPoint& point) :
    MapDraw(radius, brushType, erase),
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

const QPoint& MapDrawPoint::point() const
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

MapDrawPath::MapDrawPath(int radius, int brushType, bool erase, const QPoint& point) :
    MapDraw(radius, brushType, erase),
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
