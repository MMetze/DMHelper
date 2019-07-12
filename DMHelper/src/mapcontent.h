#ifndef MAPCONTENT_H
#define MAPCONTENT_H

#include <QPoint>
#include <QString>
#include <QColor>
#include <QDomElement>
#include "dmconstants.h"

class MapContent
{
public:
    MapContent();
    MapContent(const MapContent &obj);
    virtual ~MapContent();

    int getID() const;

protected:
    int _id;

private:
    static int createId();
    static int _id_global;

};


class MapMarker : public MapContent
{
public:
    MapMarker(const QPoint& position, const QString& title, const QString& description);
    MapMarker(const MapMarker &obj);
    virtual ~MapMarker();

    virtual const QPoint& position() const;
    virtual const QString& title() const;
    virtual const QString& description() const;

    virtual void setPosition(const QPoint& position);
    virtual void setX(int x);
    virtual void setY(int y);
    virtual void setTitle(const QString& title);
    virtual void setDescription(const QString& description);

    virtual void outputXML(QDomElement &element, bool isExport) const;
    virtual void inputXML(const QDomElement &element, bool isImport);

private:
    QPoint _position;
    QString _title;
    QString _description;

};


class MapEdit : public MapContent
{
public:
    MapEdit();
    MapEdit(const MapEdit &obj);
    virtual ~MapEdit();
};


class MapEditFill : public MapEdit
{
public:
    explicit MapEditFill(const QColor& color);
    MapEditFill(const MapEditFill &obj);
    virtual ~MapEditFill();

    virtual const QColor& color() const;

    virtual void setColor(const QColor& color);
    virtual void setRed(int red);
    virtual void setGreen(int green);
    virtual void setBlue(int blue);
    virtual void setAlpha(int alpha);

protected:
    QColor _color;
};


class MapDraw : public MapEdit
{
public:
    MapDraw(int radius = 0, int brushType = DMHelper::BrushType_Circle, bool erase = true);
    MapDraw(const MapDraw &obj);
    virtual ~MapDraw();

    virtual int radius() const;
    virtual int brushType() const;
    virtual bool erase() const;

    virtual void setRadius(int radius);
    virtual void setBrushType(int brushType);
    virtual void setErase(bool erase);

protected:
    int _radius;
    int _brushType;
    bool _erase;
};


class MapDrawPoint : public MapDraw
{
public:
    MapDrawPoint(int radius, int brushType, bool erase, const QPoint& point);
    MapDrawPoint(const MapDrawPoint &obj);
    virtual ~MapDrawPoint();

    virtual const QPoint& point() const;
    virtual void setX(int x);
    virtual void setY(int y);

    virtual void setPoint(const QPoint& point);

protected:
    QPoint _point;
};


class MapDrawPath : public MapDraw
{
public:
    MapDrawPath();
    MapDrawPath(int radius, int brushType, bool erase, const QPoint& point);
    MapDrawPath(const MapDrawPath &obj);
    virtual ~MapDrawPath();

    virtual void addPoint(const QPoint& point);

    virtual QList<QPoint> points() const;

protected:
    QList<QPoint> _points;

};



#endif // MAPCONTENT_H
