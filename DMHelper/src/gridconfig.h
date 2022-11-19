#ifndef GRIDCONFIG_H
#define GRIDCONFIG_H

#include <QObject>
#include <QPen>

class QDomDocument;
class QDomElement;

class GridConfig : public QObject
{
    Q_OBJECT
public:
    explicit GridConfig(QObject *parent = nullptr);
    virtual ~GridConfig();

    void inputXML(const QDomElement &element, bool isImport);
    void outputXML(QDomDocument &doc, QDomElement &parentElement, bool isExport) const;
    void copyValues(const GridConfig& other);

    bool getGridOn() const;
    int getGridType() const;
    int getGridScale() const;
    int getGridAngle() const;
    int getGridOffsetX() const;
    int getGridOffsetY() const;
    const QPen& getGridPen() const;

public slots:
    void setGridOn(bool gridOn);
    void setGridType(int gridType);
    void setGridScale(int gridScale);
    void setGridAngle(int gridAngle);
    void setGridOffsetX(int gridOffsetX);
    void setGridOffsetY(int gridOffsetY);
    void setGridWidth(int gridWidth);
    void setGridColor(const QColor& gridColor);

signals:
    void gridOnChanged(bool gridOn);
    void gridTypeChanged(int gridType);
    void gridScaleChanged(int gridScale);
    void gridAngleChanged(int gridAngle);
    void gridOffsetXChanged(int gridOffsetX);
    void gridOffsetYChanged(int gridOffsetY);
    void gridPenChanged(const QPen& gridPen);
    void dirty();

protected:
    bool _gridOn;
    int _gridType;
    int _gridScale;
    int _gridAngle;
    int _gridOffsetX;
    int _gridOffsetY;
    QPen _gridPen;

};

#endif // GRIDCONFIG_H
