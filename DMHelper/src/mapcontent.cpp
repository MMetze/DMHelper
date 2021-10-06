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




MapMarker::MapMarker() :
    MapContent(),
    _position(),
    _playerVisible(false),
    _title(),
    _description(),
    _color(115,18,0),
    _iconFile(),
    _iconScale(40),
    _coloredIcon(false),
    _encounter()
{
}


MapMarker::MapMarker(const QPoint& position, bool playerVisible, const QString& title, const QString& description, const QColor& color, const QString& iconFile, int iconScale, bool coloredIcon, const QUuid& encounter) :
    MapContent(),
    _position(position),
    _playerVisible(playerVisible),
    _title(title),
    _description(description),
    _color(color),
    _iconFile(iconFile),
    _iconScale(iconScale),
    _coloredIcon(coloredIcon),
    _encounter(encounter)
{
}

MapMarker::MapMarker(const MapMarker &obj) :
    MapContent(obj),
    _position(obj.getPosition()),
    _playerVisible(obj.isPlayerVisible()),
    _title(obj.getTitle()),
    _description(obj.getDescription()),
    _color(obj.getColor()),
    _iconFile(obj.getIconFile()),
    _iconScale(obj.getIconScale()),
    _coloredIcon(obj.isColoredIcon()),
    _encounter(obj.getEncounter())
{
}

MapMarker::~MapMarker()
{
}

QPoint MapMarker::getPosition() const
{
    return _position;
}

bool MapMarker::isPlayerVisible() const
{
    return _playerVisible;
}

QString MapMarker::getTitle() const
{
    return _title;
}

QString MapMarker::getDescription() const
{
    return _description;
}

QColor MapMarker::getColor() const
{
    return _color;
}

QString MapMarker::getIconFile() const
{
    return _iconFile;
}

int MapMarker::getIconScale() const
{
    return _iconScale;
}

bool MapMarker::isColoredIcon() const
{
    return _coloredIcon;
}

const QUuid& MapMarker::getEncounter() const
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

void MapMarker::setPlayerVisible(bool playerVisible)
{
    _playerVisible = playerVisible;
}

void MapMarker::setTitle(const QString& title)
{
    _title = title;
}

void MapMarker::setDescription(const QString& description)
{
    _description = description;
}

void MapMarker::setColor(const QColor& color)
{
    _color = color;
}

void MapMarker::setIconFile(const QString& iconFile)
{
    _iconFile = iconFile;
}

void MapMarker::setIconScale(int iconScale)
{
    _iconScale = iconScale;
}

void MapMarker::setColoredIcon(bool coloredIcon)
{
    _coloredIcon = coloredIcon;
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
