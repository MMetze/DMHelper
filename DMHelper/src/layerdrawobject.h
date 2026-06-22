#ifndef LAYERDRAWOBJECT_H
#define LAYERDRAWOBJECT_H

#include <QObject>
#include <QString>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QColor>
#include <QUuid>

class QDomElement;
class QDomDocument;
class QDir;

class LayerDrawObject : public QObject
{
    Q_OBJECT
public:
    explicit LayerDrawObject();
    virtual ~LayerDrawObject();

    virtual int getType() const = 0;    
    QUuid getId() const;

    virtual void inputXML(const QDomElement &element, bool isImport);
    virtual void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport);

    virtual QPointF getPosition() const;
    virtual void setPosition(const QPointF& position);

signals:
    void objectMoved(LayerDrawObject* object);

private:
    QUuid _id;
    QPointF _position;
};



class LayerDrawObjectPath : public LayerDrawObject
{
public:
    explicit LayerDrawObjectPath();
    explicit LayerDrawObjectPath(const QPointF& startPos, const QColor& penColor = Qt::black, int penWidth = 1, Qt::PenStyle penStyle = Qt::SolidLine);
    virtual ~LayerDrawObjectPath();

    virtual int getType() const override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    void addPoint(const QPointF& point);

    const QList<QPointF>& getPoints() const;
    QColor getPenColor() const;
    int getPenWidth() const;
    Qt::PenStyle getPenStyle() const;


protected:
    QColor _penColor;
    int _penWidth;
    Qt::PenStyle _penStyle;
    QList<QPointF> _points;
};



class LayerDrawObjectLine : public LayerDrawObject
{
public:
    explicit LayerDrawObjectLine();
    explicit LayerDrawObjectLine(const QPointF& startPos, const QPointF& endPos, const QColor& penColor = Qt::black, int penWidth = 1, Qt::PenStyle penStyle = Qt::SolidLine);
    virtual ~LayerDrawObjectLine();

    virtual int getType() const override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    QPointF getStartPoint() const;
    QPointF getEndPoint() const;
    void setEndPoint(const QPointF& endPoint);
    QColor getPenColor() const;
    int getPenWidth() const;
    Qt::PenStyle getPenStyle() const;

protected:
    QPointF _startPoint;
    QPointF _endPoint;
    QColor _penColor;
    int _penWidth;
    Qt::PenStyle _penStyle;
};



class LayerDrawObjectRect : public LayerDrawObject
{
public:
    explicit LayerDrawObjectRect();
    explicit LayerDrawObjectRect(const QRectF& rect, const QColor& penColor = Qt::black, int penWidth = 1, Qt::PenStyle penStyle = Qt::SolidLine, const QColor& fillColor = Qt::transparent, bool filled = false);
    virtual ~LayerDrawObjectRect();

    virtual int getType() const override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    QRectF getRect() const;
    void setRect(const QRectF& rect);
    QColor getPenColor() const;
    int getPenWidth() const;
    Qt::PenStyle getPenStyle() const;
    QColor getFillColor() const;
    bool isFilled() const;

protected:
    QRectF _rect;
    QColor _penColor;
    int _penWidth;
    Qt::PenStyle _penStyle;
    QColor _fillColor;
    bool _filled;
};



class LayerDrawObjectEllipse : public LayerDrawObject
{
public:
    explicit LayerDrawObjectEllipse();
    explicit LayerDrawObjectEllipse(const QRectF& rect, const QColor& penColor = Qt::black, int penWidth = 1, Qt::PenStyle penStyle = Qt::SolidLine, const QColor& fillColor = Qt::transparent, bool filled = false);
    virtual ~LayerDrawObjectEllipse();

    virtual int getType() const override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    QRectF getRect() const;
    void setRect(const QRectF& rect);
    QColor getPenColor() const;
    int getPenWidth() const;
    Qt::PenStyle getPenStyle() const;
    QColor getFillColor() const;
    bool isFilled() const;

protected:
    QRectF _rect;
    QColor _penColor;
    int _penWidth;
    Qt::PenStyle _penStyle;
    QColor _fillColor;
    bool _filled;
};



class LayerDrawObjectText : public LayerDrawObject
{
public:
    explicit LayerDrawObjectText();
    explicit LayerDrawObjectText(const QPointF& position, const QString& text, const QColor& textColor = Qt::black, const QString& fontFamily = QString("Arial"), int fontSize = 12);
    virtual ~LayerDrawObjectText();

    virtual int getType() const override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    QString getText() const;
    QColor getTextColor() const;
    QString getFontFamily() const;
    int getFontSize() const;

protected:
    QString _text;
    QColor _textColor;
    QString _fontFamily;
    int _fontSize;
};


#endif // LAYERDRAWOBJECT_H
