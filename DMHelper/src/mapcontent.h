#ifndef MAPCONTENT_H
#define MAPCONTENT_H

#include <QPoint>
#include <QLine>
#include <QRect>
#include <QString>
#include <QColor>
#include <QUuid>
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
    MapMarker();
    MapMarker(const QPoint& position, bool playerVisible, const QString& title, const QString& description, const QColor& color, const QString& iconFile, int iconScale, bool coloredIcon, const QUuid& encounter);
    MapMarker(const MapMarker &obj);
    virtual ~MapMarker() override;

    QPoint getPosition() const;
    bool isPlayerVisible() const;
    QString getTitle() const;
    QString getDescription() const;
    QColor getColor() const;
    QString getIconFile() const;
    int getIconScale() const;
    bool isColoredIcon() const;
    const QUuid& getEncounter() const;

    void setPosition(const QPoint& position);
    void setX(int x);
    void setY(int y);
    void setPlayerVisible(bool playerVisible);
    void setTitle(const QString& title);
    void setDescription(const QString& description);
    void setColor(const QColor& color);
    void setIconFile(const QString& iconFile);
    void setIconScale(int iconScale);
    void setColoredIcon(bool coloredIcon);
    void setEncounter(const QUuid& encounter);

private:
    QPoint _position;
    bool _playerVisible;
    QString _title;
    QString _description;
    QColor _color;
    QString _iconFile;
    int _iconScale;
    bool _coloredIcon;
    QUuid _encounter;
};




class MapEdit : public MapContent
{
public:
    MapEdit();
    MapEdit(const MapEdit &obj);
    virtual ~MapEdit() override;
};




class MapEditFill : public MapEdit
{
public:
    explicit MapEditFill(const QColor& color);
    MapEditFill(const MapEditFill &obj);
    virtual ~MapEditFill() override;

    QColor color() const;

    void setColor(const QColor& color);
    void setRed(int red);
    void setGreen(int green);
    void setBlue(int blue);
    void setAlpha(int alpha);

protected:
    QColor _color;
};




class MapDraw : public MapEdit
{
public:
    MapDraw(int radius = 0, int brushType = DMHelper::BrushType_Circle, const QColor& penColor = Qt::black, int penWidth = 1, Qt::PenStyle penStyle = Qt::SolidLine, bool erase = true, bool smooth = true);
    MapDraw(const MapDraw &obj);
    virtual ~MapDraw() override;

    virtual int radius() const;
    virtual int brushType() const;
    virtual QColor penColor() const;
    virtual int penWidth() const;
    virtual Qt::PenStyle penStyle() const;
    virtual bool erase() const;
    virtual bool smooth() const;

    virtual void setRadius(int radius);
    virtual void setBrushType(int brushType);
    virtual void setPenColor(const QColor& penColor);
    virtual void setPenWidth(int penWidth);
    virtual void setPenStyle(Qt::PenStyle penStyle);
    virtual void setErase(bool erase);
    virtual void setSmooth(bool smooth);

protected:
    int _radius;
    int _brushType;
    QColor _penColor;
    int _penWidth;
    Qt::PenStyle _penStyle;
    bool _erase;
    bool _smooth;
};




class MapDrawPoint : public MapDraw
{
public:
    MapDrawPoint(int radius, int brushType, bool erase, bool smooth, const QPoint& point, const QColor& penColor = Qt::black, int penWidth = 1, Qt::PenStyle penStyle = Qt::SolidLine);
    MapDrawPoint(const MapDrawPoint &obj);
    virtual ~MapDrawPoint() override;

    virtual QPoint point() const;
    virtual void setX(int x);
    virtual void setY(int y);

    virtual void setPoint(const QPoint& point);

protected:
    QPoint _point;
};




class MapDrawLine : public MapDraw
{
public:
    MapDrawLine();
    MapDrawLine(const QLine& line, bool erase, bool smooth, const QColor& penColor = Qt::black, int penWidth = 1, Qt::PenStyle penStyle = Qt::SolidLine);
    MapDrawLine(const MapDrawLine &obj);
    virtual ~MapDrawLine() override;

    virtual void setLine(const QLine& line);
    virtual void setP1(const QPoint &p1);
    virtual void setP2(const QPoint &p2);
    virtual qreal length() const;
    virtual QLine line() const;
    virtual QSize lineSize() const;
    virtual QPoint origin() const;
    virtual QPoint originCenter() const;
    virtual QLine originLine() const;

protected:
    QLine _line;

};




class MapDrawPath : public MapDraw
{
public:
    MapDrawPath();
    MapDrawPath(int radius, int brushType, bool erase, bool smooth, const QPoint& point, const QColor& penColor = Qt::black, int penWidth = 1, Qt::PenStyle penStyle = Qt::SolidLine);
    MapDrawPath(const MapDrawPath &obj);
    virtual ~MapDrawPath() override;

    virtual void addPoint(const QPoint& point);

    virtual QList<QPoint> points() const;
    virtual QRect pathRect() const;
    virtual QSize pathSize() const;

protected:
    QList<QPoint> _points;

};




class MapEditShape : public MapEdit
{
public:
    MapEditShape(const QRect& rect, bool erase, bool smooth);
    MapEditShape(const MapEditShape &obj);
    virtual ~MapEditShape() override;

    virtual QRect rect() const;
    virtual bool erase() const;
    virtual bool smooth() const;

    virtual void setRect(const QRect& rect);
    virtual void setErase(bool erase);
    virtual void setSmooth(bool smooth);

protected:
    QRect _rect;
    bool _erase;
    bool _smooth;
};




#endif // MAPCONTENT_H
