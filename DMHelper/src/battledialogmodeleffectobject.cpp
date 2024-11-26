#include "battledialogmodeleffectobject.h"
#include "battledialogeffectsettings.h"
#include "unselectedpixmap.h"
#include <QDomElement>
#include <QGraphicsPixmapItem>
#include <QDir>
#include <QDebug>

BattleDialogModelEffectObject::BattleDialogModelEffectObject(const QString& name, QObject *parent) :
    BattleDialogModelEffect(name, parent),
    _width(5),
    _imageRotation(0),
    _gridScale(1.0),
    _imageScaleFactors(0.0, 0.0),
    _imageFile()
{
    setEffectActive(false);
}

BattleDialogModelEffectObject::BattleDialogModelEffectObject(int size, int width, const QPointF& position, qreal rotation, const QString& imageFile, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, QColor(), tip),
    _width(width),
    _imageRotation(0),
    _gridScale(1.0),
    _imageScaleFactors(0.0, 0.0),
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

BattleDialogEffectSettingsBase* BattleDialogModelEffectObject::getEffectEditor() const
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
        return nullptr;
    }

    return createPixmapShape(gridScale, itemPixmap);
}

void BattleDialogModelEffectObject::applyEffectValues(QGraphicsItem& item, qreal gridScale)
{
    beginBatchChanges();
    item.setPos(getPosition());
    item.setToolTip(getTip());
    item.setOpacity(_color.alphaF());
    applyScale(item, gridScale);
    endBatchChanges();
}

void BattleDialogModelEffectObject::applyScale(QGraphicsItem& item, qreal gridScale)
{
    _gridScale = gridScale;
    updateTransform(&item);
}

qreal BattleDialogModelEffectObject::getScale()
{
    return static_cast<qreal>(getSize()) * _imageScaleFactors.width() / 500.0;
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

bool BattleDialogModelEffectObject::hasEffectTransform() const
{
    return true;
}

void BattleDialogModelEffectObject::updateTransform(QGraphicsItem* graphicsItem) const
{
    if(!graphicsItem)
        return;

    QTransform transform;
    transform.rotate(getRotation());
    transform.scale(static_cast<qreal>(getWidth()) * _imageScaleFactors.width() * _gridScale / 500.0,
                    static_cast<qreal>(getSize()) * _imageScaleFactors.height() * _gridScale / 500.0);
    graphicsItem->setTransform(transform);
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
    return _imageScaleFactors.width();
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
}

QGraphicsItem* BattleDialogModelEffectObject::createPixmapShape(qreal gridScale, const QPixmap& pixmap)
{
    QPixmap itemPixmap(pixmap);
    if((itemPixmap.width() <= 0) || (itemPixmap.height() <= 0))
    {
        qDebug() << "[BattleDialogModelEffectObject] ERROR: invalid image size: " << itemPixmap.size();
        return nullptr;
    }

    QGraphicsPixmapItem* pixmapItem = new UnselectedPixmap(this);
    setEffectItemData(pixmapItem);
    prepareItem(*pixmapItem);

    qDebug() << "[Battle Dialog Model Effect Object] applying extra object effect values...";

    // Now correct the special case information for the object effect
    _imageScaleFactors.setWidth(100.0 / itemPixmap.width());
    _imageScaleFactors.setHeight(100.0 / itemPixmap.height());
    if(_imageRotation != 0)
        itemPixmap = itemPixmap.transformed(QTransform().rotate(_imageRotation));

    pixmapItem->setPixmap(itemPixmap);
    pixmapItem->setOffset(-itemPixmap.width() / 2, -itemPixmap.height() / 2);

    applyEffectValues(*pixmapItem, gridScale);

    return pixmapItem;
}
