#include "gridconfig.h"
#include "dmconstants.h"
#include "grid.h"
#include <QDomElement>

GridConfig::GridConfig(QObject *parent) :
    QObject{parent},
    _gridType(Grid::GridType_Square),
    _gridScale(DMHelper::STARTING_GRID_SCALE),
    _gridAngle(50),
    _gridOffsetX(0),
    _gridOffsetY(0),
    _gridPen()
{
}

GridConfig::~GridConfig()
{
}

void GridConfig::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _gridType = element.attribute("gridType", QString::number(0)).toInt();
    if(element.hasAttribute("gridScale"))
        _gridScale = element.attribute("gridScale").toInt();
    _gridAngle = element.attribute("gridAngle", QString::number(50)).toInt();
    _gridOffsetX = element.attribute("gridOffsetX", QString::number(0)).toInt();
    _gridOffsetY = element.attribute("gridOffsetY", QString::number(0)).toInt();
    int gridWidth = element.attribute("gridWidth", QString::number(1)).toInt();
    QColor gridColor = element.attribute("gridColor", QString("#000000"));
    _gridPen = QPen(QBrush(gridColor), gridWidth);
    _gridOffsetY = element.attribute("gridOffsetY", QString::number(0)).toInt();
}

void GridConfig::outputXML(QDomDocument &doc, QDomElement &parentElement, bool isExport) const
{
    Q_UNUSED(isExport);

    QDomElement gridElement = doc.createElement("grid");

    gridElement.setAttribute("gridType", _gridType);
    gridElement.setAttribute("gridScale", _gridScale);
    gridElement.setAttribute("gridAngle", _gridAngle);
    gridElement.setAttribute("gridOffsetX", _gridOffsetX);
    gridElement.setAttribute("gridOffsetY", _gridOffsetY);
    gridElement.setAttribute("gridWidth", _gridPen.width());
    gridElement.setAttribute("gridColor", _gridPen.color().name());

    parentElement.appendChild(gridElement);
}

void GridConfig::copyValues(const GridConfig& other)
{
    _gridType = other._gridType;
    _gridScale = other._gridScale;
    _gridAngle = other._gridAngle;
    _gridOffsetX = other._gridOffsetX;
    _gridOffsetY = other._gridOffsetY;
    _gridPen = other._gridPen;
}

int GridConfig::getGridType() const
{
    return _gridType;
}

int GridConfig::getGridScale() const
{
    return _gridScale;
}

int GridConfig::getGridAngle() const
{
    return _gridAngle;
}

int GridConfig::getGridOffsetX() const
{
    return _gridOffsetX;
}

int GridConfig::getGridOffsetY() const
{
    return _gridOffsetY;
}

const QPen& GridConfig::getGridPen() const
{
    return _gridPen;
}

void GridConfig::setGridType(int gridType)
{
    if(_gridType != gridType)
    {
        _gridType = gridType;
        emit gridTypeChanged(_gridType);
        emit dirty();
    }
}

void GridConfig::setGridScale(int gridScale)
{
    if(_gridScale != gridScale)
    {
        _gridScale = gridScale;
        emit gridScaleChanged(_gridScale);
        emit dirty();
    }
}

void GridConfig::setGridAngle(int gridAngle)
{
    if(_gridAngle != gridAngle)
    {
        _gridAngle = gridAngle;
        emit gridAngleChanged(_gridAngle);
        emit dirty();
    }
}

void GridConfig::setGridOffsetX(int gridOffsetX)
{
    if(_gridOffsetX != gridOffsetX)
    {
        _gridOffsetX = gridOffsetX;
        emit gridOffsetXChanged(_gridOffsetX);
        emit dirty();
    }
}

void GridConfig::setGridOffsetY(int gridOffsetY)
{
    if(_gridOffsetY != gridOffsetY)
    {
        _gridOffsetY = gridOffsetY;
        emit gridOffsetYChanged(_gridOffsetY);
        emit dirty();
    }
}

void GridConfig::setGridWidth(int gridWidth)
{
    if(_gridPen.width() != gridWidth)
    {
        _gridPen.setWidth(gridWidth);
        emit gridPenChanged(_gridPen);
        emit dirty();
    }
}

void GridConfig::setGridColor(const QColor& gridColor)
{
    if(_gridPen.color() != gridColor)
    {
        _gridPen.setColor(gridColor);
        emit gridPenChanged(_gridPen);
        emit dirty();
    }
}

