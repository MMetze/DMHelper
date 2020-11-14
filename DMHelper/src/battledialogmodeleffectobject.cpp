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

QGraphicsItem* BattleDialogModelEffectObject::createEffectShape(qreal gridScale) const
{
    QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem();
    setEffectItemData(pixmapItem);

    prepareItem(*pixmapItem);
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

    qDebug() << "[Battle Dialog Model Effect Object] applying extra object effect values...";

    // Now correct the special case information for the object effect
    QGraphicsPixmapItem* pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(&item);
    if(pixmapItem)
    {
        QPixmap itemPixmap(_imageFile);
        if(itemPixmap.isNull())
        {
            qDebug() << "[Battle Dialog Model Effect Object] ERROR: unable to load image file: " << _imageFile;
            return;
        }

        //pixmapItem->setPixmap(itemPixmap.scaled(getWidth() * 100, getSize() * 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        int pixmapWidth = getWidth() >= getSize() ? 500 : static_cast<int>(500.0 * getWidth() / getSize());
        int pixmapHeight = getSize() >= getWidth() ? 500 : static_cast<int>(500.0 * getSize() / getWidth());
        if(_imageRotation != 0)
        {
            itemPixmap = itemPixmap.transformed(QTransform().rotate(_imageRotation));
        }
        QPixmap scaledPixmap = itemPixmap.scaled(pixmapWidth, pixmapHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        pixmapItem->setPixmap(scaledPixmap);
        //pixmapItem->setPixmap(itemPixmap);
    }
    else
    {
        qDebug() << "[Battle Dialog Model Effect Object] ERROR: Object Effect found without QGraphicsPixmapItem!";
    }

    //item.setScale(gridScale / 500.0);
    item.setScale(static_cast<qreal>(getSize()) * gridScale / 2500.0);

}

int BattleDialogModelEffectObject::getWidth() const
{
    return _width;
}

void BattleDialogModelEffectObject::setWidth(int width)
{
    _width = width;
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
    _imageFile = imageFile;
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
