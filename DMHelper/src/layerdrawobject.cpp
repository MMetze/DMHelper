#include "layerdrawobject.h"
#include "dmconstants.h"
#include <QDomElement>

LayerDrawObject::LayerDrawObject() :
    QObject(),
    _id(QUuid::createUuid()),
    _position()
{
}

LayerDrawObject::~LayerDrawObject()
{
}

QUuid LayerDrawObject::getId() const
{
    return _id;
}

void LayerDrawObject::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _id = QUuid(element.attribute(QString("id")));
    _position = QPointF(element.attribute("positionX", QString::number(0)).toDouble(),
                        element.attribute("positionY", QString::number(0)).toDouble());
}

void LayerDrawObject::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute("id", _id.toString());
    element.setAttribute("type", QString::number(getType()));
    if(_position.x() != 0.0)
        element.setAttribute("positionX", _position.x());
    if(_position.y() != 0.0)
        element.setAttribute("positionY", _position.y());
}

QPointF LayerDrawObject::getPosition() const
{
    return _position;
}

void LayerDrawObject::setPosition(const QPointF& position)
{
    if(_position == position)
        return;

    _position = position;
    emit objectMoved(this);
}



LayerDrawObjectPath::LayerDrawObjectPath() :
    LayerDrawObject(),
    _penColor{Qt::black},
    _penWidth{1},
    _penStyle{Qt::SolidLine},
    _points{}
{
}

LayerDrawObjectPath::LayerDrawObjectPath(const QPointF& startPos, const QColor& penColor, int penWidth, Qt::PenStyle penStyle) :
    LayerDrawObject(),
    _penColor{penColor},
    _penWidth{penWidth},
    _penStyle{penStyle},
    _points{startPos}
{
}

LayerDrawObjectPath::~LayerDrawObjectPath()
{
}

int LayerDrawObjectPath::getType() const
{
    return DMHelper::ActionType_Path;
}

void LayerDrawObjectPath::inputXML(const QDomElement &element, bool isImport)
{
    LayerDrawObject::inputXML(element, isImport);

    // Parse pen attributes
    _penColor = QColor(element.attribute("penColor", "#000000"));
    _penWidth = element.attribute("penWidth", "1").toInt();
    _penStyle = static_cast<Qt::PenStyle>(element.attribute("penStyle", "1").toInt());

    // Parse points
    _points.clear();
    QDomElement pointElement = element.firstChildElement("point");
    while(!pointElement.isNull())
    {
        qreal x = pointElement.attribute("x").toDouble();
        qreal y = pointElement.attribute("y").toDouble();
        _points.append(QPointF(x, y));
        pointElement = pointElement.nextSiblingElement("point");
    }
}

void LayerDrawObjectPath::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    LayerDrawObject::outputXML(doc, element, targetDirectory, isExport);

    // Set pen attributes
    element.setAttribute("penColor", _penColor.name());
    element.setAttribute("penWidth", QString::number(_penWidth));
    element.setAttribute("penStyle", QString::number(static_cast<int>(_penStyle)));

    // Add points as child elements
    for(const QPointF& point : std::as_const(_points))
    {
        QDomElement pointElement = doc.createElement("point");
        pointElement.setAttribute("x", QString::number(point.x()));
        pointElement.setAttribute("y", QString::number(point.y()));
        element.appendChild(pointElement);
    }
}

void LayerDrawObjectPath::addPoint(const QPointF& point)
{
    _points.append(point);
}

const QList<QPointF>& LayerDrawObjectPath::getPoints() const
{
    return _points;
}

QColor LayerDrawObjectPath::getPenColor() const
{
    return _penColor;
}

int LayerDrawObjectPath::getPenWidth() const
{
    return _penWidth;
}

Qt::PenStyle LayerDrawObjectPath::getPenStyle() const
{
    return static_cast<Qt::PenStyle>(_penStyle);
}



// --- LayerDrawObjectLine ---

LayerDrawObjectLine::LayerDrawObjectLine() :
    LayerDrawObject(),
    _startPoint{},
    _endPoint{},
    _penColor{Qt::black},
    _penWidth{1},
    _penStyle{Qt::SolidLine}
{
}

LayerDrawObjectLine::LayerDrawObjectLine(const QPointF& startPos, const QPointF& endPos, const QColor& penColor, int penWidth, Qt::PenStyle penStyle) :
    LayerDrawObject(),
    _startPoint{startPos},
    _endPoint{endPos},
    _penColor{penColor},
    _penWidth{penWidth},
    _penStyle{penStyle}
{
}

LayerDrawObjectLine::~LayerDrawObjectLine()
{
}

int LayerDrawObjectLine::getType() const
{
    return DMHelper::ActionType_Line;
}

void LayerDrawObjectLine::inputXML(const QDomElement &element, bool isImport)
{
    LayerDrawObject::inputXML(element, isImport);

    _penColor = QColor(element.attribute("penColor", "#000000"));
    _penWidth = element.attribute("penWidth", "1").toInt();
    _penStyle = static_cast<Qt::PenStyle>(element.attribute("penStyle", "1").toInt());
    _startPoint = QPointF(element.attribute("startX", "0").toDouble(),
                          element.attribute("startY", "0").toDouble());
    _endPoint = QPointF(element.attribute("endX", "0").toDouble(),
                        element.attribute("endY", "0").toDouble());
}

void LayerDrawObjectLine::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    LayerDrawObject::outputXML(doc, element, targetDirectory, isExport);

    element.setAttribute("penColor", _penColor.name());
    element.setAttribute("penWidth", QString::number(_penWidth));
    element.setAttribute("penStyle", QString::number(static_cast<int>(_penStyle)));
    element.setAttribute("startX", QString::number(_startPoint.x()));
    element.setAttribute("startY", QString::number(_startPoint.y()));
    element.setAttribute("endX", QString::number(_endPoint.x()));
    element.setAttribute("endY", QString::number(_endPoint.y()));
}

QPointF LayerDrawObjectLine::getStartPoint() const
{
    return _startPoint;
}

QPointF LayerDrawObjectLine::getEndPoint() const
{
    return _endPoint;
}

void LayerDrawObjectLine::setEndPoint(const QPointF& endPoint)
{
    _endPoint = endPoint;
}

QColor LayerDrawObjectLine::getPenColor() const
{
    return _penColor;
}

int LayerDrawObjectLine::getPenWidth() const
{
    return _penWidth;
}

Qt::PenStyle LayerDrawObjectLine::getPenStyle() const
{
    return _penStyle;
}



// --- LayerDrawObjectRect ---

LayerDrawObjectRect::LayerDrawObjectRect() :
    LayerDrawObject(),
    _rect{},
    _penColor{Qt::black},
    _penWidth{1},
    _penStyle{Qt::SolidLine},
    _fillColor{Qt::transparent},
    _filled{false}
{
}

LayerDrawObjectRect::LayerDrawObjectRect(const QRectF& rect, const QColor& penColor, int penWidth, Qt::PenStyle penStyle, const QColor& fillColor, bool filled) :
    LayerDrawObject(),
    _rect{rect},
    _penColor{penColor},
    _penWidth{penWidth},
    _penStyle{penStyle},
    _fillColor{fillColor},
    _filled{filled}
{
}

LayerDrawObjectRect::~LayerDrawObjectRect()
{
}

int LayerDrawObjectRect::getType() const
{
    return DMHelper::ActionType_Rect;
}

void LayerDrawObjectRect::inputXML(const QDomElement &element, bool isImport)
{
    LayerDrawObject::inputXML(element, isImport);

    _penColor = QColor(element.attribute("penColor", "#000000"));
    _penWidth = element.attribute("penWidth", "1").toInt();
    _penStyle = static_cast<Qt::PenStyle>(element.attribute("penStyle", "1").toInt());
    _fillColor = QColor(element.attribute("fillColor", "#00000000"));
    _filled = element.attribute("filled", "0").toInt() != 0;
    _rect = QRectF(element.attribute("rectX", "0").toDouble(),
                   element.attribute("rectY", "0").toDouble(),
                   element.attribute("rectW", "0").toDouble(),
                   element.attribute("rectH", "0").toDouble());
}

void LayerDrawObjectRect::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    LayerDrawObject::outputXML(doc, element, targetDirectory, isExport);

    element.setAttribute("penColor", _penColor.name());
    element.setAttribute("penWidth", QString::number(_penWidth));
    element.setAttribute("penStyle", QString::number(static_cast<int>(_penStyle)));
    element.setAttribute("fillColor", _fillColor.name(QColor::HexArgb));
    element.setAttribute("filled", _filled ? 1 : 0);
    element.setAttribute("rectX", QString::number(_rect.x()));
    element.setAttribute("rectY", QString::number(_rect.y()));
    element.setAttribute("rectW", QString::number(_rect.width()));
    element.setAttribute("rectH", QString::number(_rect.height()));
}

QRectF LayerDrawObjectRect::getRect() const
{
    return _rect;
}

void LayerDrawObjectRect::setRect(const QRectF& rect)
{
    _rect = rect;
}

QColor LayerDrawObjectRect::getPenColor() const
{
    return _penColor;
}

int LayerDrawObjectRect::getPenWidth() const
{
    return _penWidth;
}

Qt::PenStyle LayerDrawObjectRect::getPenStyle() const
{
    return _penStyle;
}

QColor LayerDrawObjectRect::getFillColor() const
{
    return _fillColor;
}

bool LayerDrawObjectRect::isFilled() const
{
    return _filled;
}



// --- LayerDrawObjectEllipse ---

LayerDrawObjectEllipse::LayerDrawObjectEllipse() :
    LayerDrawObject(),
    _rect{},
    _penColor{Qt::black},
    _penWidth{1},
    _penStyle{Qt::SolidLine},
    _fillColor{Qt::transparent},
    _filled{false}
{
}

LayerDrawObjectEllipse::LayerDrawObjectEllipse(const QRectF& rect, const QColor& penColor, int penWidth, Qt::PenStyle penStyle, const QColor& fillColor, bool filled) :
    LayerDrawObject(),
    _rect{rect},
    _penColor{penColor},
    _penWidth{penWidth},
    _penStyle{penStyle},
    _fillColor{fillColor},
    _filled{filled}
{
}

LayerDrawObjectEllipse::~LayerDrawObjectEllipse()
{
}

int LayerDrawObjectEllipse::getType() const
{
    return DMHelper::ActionType_Ellipse;
}

void LayerDrawObjectEllipse::inputXML(const QDomElement &element, bool isImport)
{
    LayerDrawObject::inputXML(element, isImport);

    _penColor = QColor(element.attribute("penColor", "#000000"));
    _penWidth = element.attribute("penWidth", "1").toInt();
    _penStyle = static_cast<Qt::PenStyle>(element.attribute("penStyle", "1").toInt());
    _fillColor = QColor(element.attribute("fillColor", "#00000000"));
    _filled = element.attribute("filled", "0").toInt() != 0;
    _rect = QRectF(element.attribute("rectX", "0").toDouble(),
                   element.attribute("rectY", "0").toDouble(),
                   element.attribute("rectW", "0").toDouble(),
                   element.attribute("rectH", "0").toDouble());
}

void LayerDrawObjectEllipse::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    LayerDrawObject::outputXML(doc, element, targetDirectory, isExport);

    element.setAttribute("penColor", _penColor.name());
    element.setAttribute("penWidth", QString::number(_penWidth));
    element.setAttribute("penStyle", QString::number(static_cast<int>(_penStyle)));
    element.setAttribute("fillColor", _fillColor.name(QColor::HexArgb));
    element.setAttribute("filled", _filled ? 1 : 0);
    element.setAttribute("rectX", QString::number(_rect.x()));
    element.setAttribute("rectY", QString::number(_rect.y()));
    element.setAttribute("rectW", QString::number(_rect.width()));
    element.setAttribute("rectH", QString::number(_rect.height()));
}

QRectF LayerDrawObjectEllipse::getRect() const
{
    return _rect;
}

void LayerDrawObjectEllipse::setRect(const QRectF& rect)
{
    _rect = rect;
}

QColor LayerDrawObjectEllipse::getPenColor() const
{
    return _penColor;
}

int LayerDrawObjectEllipse::getPenWidth() const
{
    return _penWidth;
}

Qt::PenStyle LayerDrawObjectEllipse::getPenStyle() const
{
    return _penStyle;
}

QColor LayerDrawObjectEllipse::getFillColor() const
{
    return _fillColor;
}

bool LayerDrawObjectEllipse::isFilled() const
{
    return _filled;
}



// --- LayerDrawObjectText ---

LayerDrawObjectText::LayerDrawObjectText() :
    LayerDrawObject(),
    _text{},
    _textColor{Qt::black},
    _fontFamily{QString("Arial")},
    _fontSize{12}
{
}

LayerDrawObjectText::LayerDrawObjectText(const QPointF& position, const QString& text, const QColor& textColor, const QString& fontFamily, int fontSize) :
    LayerDrawObject(),
    _text{text},
    _textColor{textColor},
    _fontFamily{fontFamily},
    _fontSize{fontSize}
{
    setPosition(position);
}

LayerDrawObjectText::~LayerDrawObjectText()
{
}

int LayerDrawObjectText::getType() const
{
    return DMHelper::ActionType_Text;
}

void LayerDrawObjectText::inputXML(const QDomElement &element, bool isImport)
{
    LayerDrawObject::inputXML(element, isImport);

    _text = element.attribute("text");
    _textColor = QColor(element.attribute("textColor", "#000000"));
    _fontFamily = element.attribute("fontFamily", "Arial");
    _fontSize = element.attribute("fontSize", "12").toInt();
}

void LayerDrawObjectText::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    LayerDrawObject::outputXML(doc, element, targetDirectory, isExport);

    element.setAttribute("text", _text);
    element.setAttribute("textColor", _textColor.name());
    element.setAttribute("fontFamily", _fontFamily);
    element.setAttribute("fontSize", QString::number(_fontSize));
}

QString LayerDrawObjectText::getText() const
{
    return _text;
}

QColor LayerDrawObjectText::getTextColor() const
{
    return _textColor;
}

QString LayerDrawObjectText::getFontFamily() const
{
    return _fontFamily;
}

int LayerDrawObjectText::getFontSize() const
{
    return _fontSize;
}
