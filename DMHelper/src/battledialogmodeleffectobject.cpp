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
    _imageFile()
{
    setEffectActive(false);
}

BattleDialogModelEffectObject::BattleDialogModelEffectObject(int size, int width, const QPointF& position, qreal rotation, const QString& imageFile, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, QColor(), tip),
    _width(width),
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
    BattleDialogModelEffect::applyEffectValues(item, gridScale);

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
        int pixmapWidth = getWidth() >= getSize() ? 100 : static_cast<int>(100.0 * getWidth() / getSize());
        int pixmapHeight = getSize() >= getWidth() ? 100 : static_cast<int>(100.0 * getSize() / getWidth());
        pixmapItem->setPixmap(itemPixmap.scaled(pixmapWidth, pixmapHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    else
    {
        qDebug() << "[Battle Dialog Model Effect Object] ERROR: Object Effect found without QGraphicsPixmapItem!";
    }

    //item.setScale(gridScale / 500.0);
}

int BattleDialogModelEffectObject::getWidth() const
{
    return _width;
}

void BattleDialogModelEffectObject::setWidth(int width)
{
    _width = width;
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
