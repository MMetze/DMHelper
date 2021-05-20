#include "battledialogmodeleffectobject.h"
#include "battledialogeffectsettings.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QGraphicsPixmapItem>
#include <QDir>
#include <QDebug>

BattleDialogModelEffectObject::BattleDialogModelEffectObject(const QString& name, QObject *parent) :
    BattleDialogModelEffect(name, parent),
    _width(5),
    _imageRotation(0),
    _imageFile()
{
    setEffectActive(false);
}

BattleDialogModelEffectObject::BattleDialogModelEffectObject(int size, int width, const QPointF& position, qreal rotation, const QString& imageFile, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, QColor(), tip),
    _width(width),
    _imageRotation(0),
    _imageFile(imageFile)
{
    setEffectActive(false);
}

BattleDialogModelEffectObject::~BattleDialogModelEffectObject()
{
}

void BattleDialogModelEffectObject::inputXML(const QDomElement &element, bool isImport)
{
    _width = element.attribute("width", QString::number(5)).toInt();
    _imageRotation = element.attribute("imageRotation", QString::number(0)).toInt();
    _imageFile = element.attribute("filename");

    BattleDialogModelEffect::inputXML(element, isImport);
}

BattleDialogModelEffect* BattleDialogModelEffectObject::clone() const
{
    BattleDialogModelEffectObject* newEffect = new BattleDialogModelEffectObject(getName());
    newEffect->copyValues(*this);
    return newEffect;
}

int BattleDialogModelEffectObject::getEffectType() const
{
    return BattleDialogModelEffect_Object;
}

BattleDialogEffectSettings* BattleDialogModelEffectObject::getEffectEditor() const
{
    BattleDialogEffectSettings* result = new BattleDialogEffectSettings(*this);
    result->setSizeLabel(QString("Height"));
    result->setShowWidth(true);
    result->setShowColor(false);
    result->setShowActive(false);
    return result;
}

QGraphicsItem* BattleDialogModelEffectObject::createEffectShape(qreal gridScale)
{
    QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem();
    setEffectItemData(pixmapItem);
    prepareItem(*pixmapItem);

    qDebug() << "[Battle Dialog Model Effect Object] applying extra object effect values...";

    // Now correct the special case information for the object effect
    QPixmap itemPixmap(_imageFile);
    if(itemPixmap.isNull())
    {
        qDebug() << "[Battle Dialog Model Effect Object] ERROR: unable to load image file: " << _imageFile;
        delete pixmapItem;
        return nullptr;
    }

    //_imageScaleFactor = 100.0 / (getWidth() >= getSize() ? getWidth() : getSize());
    _imageScaleFactor = 100.0 / itemPixmap.width();
    //int pixmapWidth = getWidth() >= getSize() ? 500 : static_cast<int>(500.0 * getWidth() / getSize());
    //int pixmapHeight = getSize() >= getWidth() ? 500 : static_cast<int>(500.0 * getSize() / getWidth());
    if(_imageRotation != 0)
    {
        itemPixmap = itemPixmap.transformed(QTransform().rotate(_imageRotation));
    }
    //QPixmap scaledPixmap = itemPixmap.scaled(pixmapWidth, pixmapHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pixmapItem->setPixmap(itemPixmap);
    pixmapItem->setOffset(-itemPixmap.width() / 2, -itemPixmap.height() / 2);

    applyEffectValues(*pixmapItem, gridScale);

    return pixmapItem;
}

void BattleDialogModelEffectObject::applyEffectValues(QGraphicsItem& item, qreal gridScale) const
{
    // First apply the base information
    //BattleDialogModelEffect::applyEffectValues(item, gridScale);

    item.setPos(getPosition());
    item.setRotation(getRotation());
    item.setToolTip(getTip());

    setItemScale(&item, static_cast<qreal>(getSize()) * gridScale / 500.0);
}

int BattleDialogModelEffectObject::getWidth() const
{
    return _width;
}

void BattleDialogModelEffectObject::setWidth(int width)
{
    _width = width;
}

void BattleDialogModelEffectObject::setItemScale(QGraphicsItem* item, qreal scaleFactor) const
{
    if(item)
        item->setScale(scaleFactor * _imageScaleFactor);
}

int BattleDialogModelEffectObject::getImageRotation() const
{
    return _imageRotation;
}

void BattleDialogModelEffectObject::setImageRotation(int imageRotation)
{
    _imageRotation = imageRotation;
}

QString BattleDialogModelEffectObject::getImageFile() const
{
    return _imageFile;
}

void BattleDialogModelEffectObject::setImageFile(const QString& imageFile)
{
    if(_imageFile != imageFile)
    {
        _imageFile = imageFile;
        setMD5(QString());
    }
}

qreal BattleDialogModelEffectObject::getImageScaleFactor() const
{
    return _imageScaleFactor;
}

void BattleDialogModelEffectObject::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("width", getWidth());
    element.setAttribute("imageRotation", getImageRotation());

    QString objectImage = getImageFile();
    if(objectImage.isEmpty())
        element.setAttribute("filename", QString(""));
    else
        element.setAttribute("filename", targetDirectory.relativeFilePath(objectImage));

    BattleDialogModelEffect::internalOutputXML(doc, element, targetDirectory, isExport);
}

void BattleDialogModelEffectObject::prepareItem(QGraphicsItem& item) const
{
    BattleDialogModelEffect::prepareItem(item);
    item.setZValue(DMHelper::BattleDialog_Z_Camera);
}
