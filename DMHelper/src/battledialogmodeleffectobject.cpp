#include "battledialogmodeleffectobject.h"
#include "battledialogeffectsettings.h"
#include "dmconstants.h"
#include "unselectedpixmap.h"
#include <QDomElement>
#include <QGraphicsPixmapItem>
#include <QDir>
#include <QDebug>

BattleDialogModelEffectObject::BattleDialogModelEffectObject(const QString& name, QObject *parent) :
    BattleDialogModelEffect(name, parent),
    _width(5),
    _imageRotation(0),
    _imageScaleFactor(0.0),
    _imageFile()
{
    setEffectActive(false);
}

BattleDialogModelEffectObject::BattleDialogModelEffectObject(int size, int width, const QPointF& position, qreal rotation, const QString& imageFile, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, QColor(), tip),
    _width(width),
    _imageRotation(0),
    _imageScaleFactor(0.0),
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

void BattleDialogModelEffectObject::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelEffectObject* otherEffect = dynamic_cast<const BattleDialogModelEffectObject*>(other);
    if(!otherEffect)
        return;

    _width = otherEffect->_width;
    _imageRotation = otherEffect->_imageRotation;
    _imageFile = otherEffect->_imageFile;

    BattleDialogModelEffect::copyValues(other);
}

QString BattleDialogModelEffectObject::getName() const
{
    return _tip.isEmpty() ? QString("Object") : _tip;
}

BattleDialogModelEffect* BattleDialogModelEffectObject::clone() const
{
    BattleDialogModelEffectObject* newEffect = new BattleDialogModelEffectObject(getName());
    newEffect->copyValues(this);
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
    QPixmap itemPixmap(_imageFile);
    if(itemPixmap.isNull())
    {
        qDebug() << "[Battle Dialog Model Effect Object] ERROR: unable to load image file: " << _imageFile;
        //delete pixmapItem;
        return nullptr;
    }

    //QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(); add effect ref and set position, use UnselectedPixmap
    QGraphicsPixmapItem* pixmapItem = new UnselectedPixmap(this);
    setEffectItemData(pixmapItem);
    prepareItem(*pixmapItem);

    qDebug() << "[Battle Dialog Model Effect Object] applying extra object effect values...";

    // Now correct the special case information for the object effect
    _imageScaleFactor = 100.0 / itemPixmap.width();
    if(_imageRotation != 0)
    {
        itemPixmap = itemPixmap.transformed(QTransform().rotate(_imageRotation));
    }
    pixmapItem->setPixmap(itemPixmap);
    pixmapItem->setOffset(-itemPixmap.width() / 2, -itemPixmap.height() / 2);

    applyEffectValues(*pixmapItem, gridScale);

    return pixmapItem;
}

void BattleDialogModelEffectObject::applyEffectValues(QGraphicsItem& item, qreal gridScale)
{
    beginBatchChanges();
    item.setPos(getPosition());
    item.setRotation(getRotation());
    item.setToolTip(getTip());
    item.setOpacity(_color.alphaF());
    setItemScale(&item, static_cast<qreal>(getSize()) * gridScale / 500.0);
    endBatchChanges();
}

int BattleDialogModelEffectObject::getWidth() const
{
    return _width;
}

void BattleDialogModelEffectObject::setWidth(int width)
{
    if(_width != width)
    {
        _width = width;
        emit effectChanged(this);
    }
}

void BattleDialogModelEffectObject::setItemScale(QGraphicsItem* item, qreal scaleFactor)
{
    if((item) && (item->scale() != scaleFactor * _imageScaleFactor))
    {
        item->setScale(scaleFactor * _imageScaleFactor);
        registerChange();
    }
}

int BattleDialogModelEffectObject::getImageRotation() const
{
    return _imageRotation;
}

void BattleDialogModelEffectObject::setImageRotation(int imageRotation)
{
    if(_imageRotation != imageRotation)
    {
        _imageRotation = imageRotation;
        registerChange();
    }
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
        registerChange();
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
