#include "monsterclassv2.h"
#include "monsterfactory.h"
#include "bestiary.h"
#include <QDomElement>

MonsterClassv2::MonsterClassv2(const QString& name, QObject *parent) :
    QObject{parent},
    TemplateObject{MonsterFactory::Instance()},
    _private(false),
    _icons(),
    _batchChanges(false),
    _changesMade(false),
    _iconChanged(false),
    _scaledPixmaps()
{
    setStringValue("name", name);
}

MonsterClassv2::MonsterClassv2(const QDomElement &element, bool isImport, QObject *parent) :
    QObject{parent},
    TemplateObject{MonsterFactory::Instance()}
{
    inputXML(element, isImport);
}

void MonsterClassv2::inputXML(const QDomElement &element, bool isImport)
{
    beginBatchChanges();

    setPrivate(static_cast<bool>(element.attribute("private", QString::number(0)).toInt()));
    readXMLValues(element, isImport);
    readIcons(element, isImport);

    endBatchChanges();
}

QDomElement MonsterClassv2::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
{
    element.setAttribute("private", static_cast<int>(getPrivate()));
    writeIcons(doc, element, targetDirectory, isExport);
    writeXMLValues(doc, element, targetDirectory, isExport);

    return element;
}

void MonsterClassv2::beginBatchChanges()
{
    _batchChanges = true;
    _changesMade = false;
    _iconChanged = false;
}

void MonsterClassv2::endBatchChanges()
{
    if(_batchChanges)
    {
        _batchChanges = false;
        if(_iconChanged)
            emit iconChanged(this);

        if(_changesMade)
            emit dirty();
    }
}

int MonsterClassv2::getType() const
{
    return DMHelper::CombatantType_Monster;
}

bool MonsterClassv2::getPrivate() const
{
    return _private;
}

int MonsterClassv2::getIconCount() const
{
    return _icons.count();
}

QStringList MonsterClassv2::getIconList() const
{
    return _icons;
}

QString MonsterClassv2::getIcon(int index) const
{
    if((index < 0) || (index >= _icons.count()))
        return QString();
    else
        return _icons.at(index);
}

QPixmap MonsterClassv2::getIconPixmap(DMHelper::PixmapSize iconSize, int index)
{
    if((index < 0) || (index >= _scaledPixmaps.count()))
        return ScaledPixmap::defaultPixmap()->getPixmap(iconSize);
    else
        return _scaledPixmaps[index].getPixmap(iconSize);
}

void MonsterClassv2::cloneMonster(MonsterClassv2& other)
{
    beginBatchChanges();

    _private = other._private;
    foreach(const QString& key, _allValues.keys())
    {
        _allValues.insert(key, other._allValues.value(key));
    }

    endBatchChanges();

}

int MonsterClassv2::convertSizeToCategory(const QString& monsterSize)
{
    if(QString::compare(monsterSize, QString("Tiny"), Qt::CaseInsensitive) == 0)
        return DMHelper::CombatantSize_Tiny;
    else if(QString::compare(monsterSize, QString("Small"), Qt::CaseInsensitive) == 0)
        return DMHelper::CombatantSize_Small;
    else if(QString::compare(monsterSize, QString("Medium"), Qt::CaseInsensitive) == 0)
        return DMHelper::CombatantSize_Medium;
    else if(QString::compare(monsterSize, QString("Large"), Qt::CaseInsensitive) == 0)
        return DMHelper::CombatantSize_Large;
    else if(QString::compare(monsterSize, QString("Huge"), Qt::CaseInsensitive) == 0)
        return DMHelper::CombatantSize_Huge;
    else if(QString::compare(monsterSize, QString("Gargantuan"), Qt::CaseInsensitive) == 0)
        return DMHelper::CombatantSize_Gargantuan;
    else if(QString::compare(monsterSize, QString("Colossal"), Qt::CaseInsensitive) == 0)
        return DMHelper::CombatantSize_Colossal;
    else
        return DMHelper::CombatantSize_Unknown;
}

QString MonsterClassv2::convertCategoryToSize(int category)
{
    switch(category)
    {
    case DMHelper::CombatantSize_Tiny:
        return QString("Tiny");
    case DMHelper::CombatantSize_Small:
        return QString("Small");
    case DMHelper::CombatantSize_Medium:
        return QString("Medium");
    case DMHelper::CombatantSize_Large:
        return QString("Large");
    case DMHelper::CombatantSize_Huge:
        return QString("Huge");
    case DMHelper::CombatantSize_Gargantuan:
        return QString("Gargantuan");
    case DMHelper::CombatantSize_Colossal:
        return QString("Colossal");
    default:
        return QString("Medium");
    }
}

qreal MonsterClassv2::convertSizeCategoryToScaleFactor(int category)
{
    switch(category)
    {
    case DMHelper::CombatantSize_Tiny:
        return 0.5;
    case DMHelper::CombatantSize_Small:
        return 0.75;
    case DMHelper::CombatantSize_Medium:
        return 1.0;
    case DMHelper::CombatantSize_Large:
        return 2.0;
    case DMHelper::CombatantSize_Huge:
        return 3.0;
    case DMHelper::CombatantSize_Gargantuan:
        return 4.0;
    case DMHelper::CombatantSize_Colossal:
        return 8.0;
    default:
        return 1.0;
    }
}

qreal MonsterClassv2::convertSizeToScaleFactor(const QString& monsterSize)
{
    return convertSizeCategoryToScaleFactor(convertSizeToCategory(monsterSize));
}

void MonsterClassv2::outputValue(QDomDocument &doc, QDomElement &element, bool isExport, const QString& valueName, const QString& valueText)
{
    Q_UNUSED(isExport);

    QDomElement newChild = doc.createElement(valueName);
    newChild.appendChild(doc.createTextNode(valueText));
    element.appendChild(newChild);
}

void MonsterClassv2::setPrivate(bool isPrivate)
{
    if(isPrivate == _private)
        return;

    _private = isPrivate;
    registerChange();
}

void MonsterClassv2::addIcon(const QString &newIcon)
{
    if((newIcon.isEmpty()) || (_icons.contains(newIcon)))
        return;

    ScaledPixmap newPixmap;
    QString basePixmap = Bestiary::Instance()->getDirectory().filePath(newIcon);
    if(!newPixmap.setBasePixmap(basePixmap))
    {
        qDebug() << "[MonsterClassv2] ERROR: Unable to set icon pixmap for monster: " << getStringValue("name") << " - " << basePixmap;
        return;
    }

    _icons.append(newIcon);
    _scaledPixmaps.append(newPixmap);
    registerChange();

    if(_batchChanges)
        _iconChanged = true;
    else
        emit iconChanged(this);
}

void MonsterClassv2::setIcon(int index, const QString& iconFile)
{
    if((index < 0) || (index >= _icons.count()))
        return;

    QString searchResult = Bestiary::Instance()->findMonsterImage(getStringValue("name"), iconFile);
    if(searchResult.isEmpty())
        return;

    ScaledPixmap newPixmap;
    QString basePixmap = Bestiary::Instance()->getDirectory().filePath(searchResult);
    if(!newPixmap.setBasePixmap(basePixmap))
    {
        qDebug() << "[MonsterClassv2] ERROR: Unable to set icon pixmap for monster: " << getStringValue("name") << " - " << basePixmap;
        return;
    }

    _icons[index] = searchResult;
    _scaledPixmaps[index] = newPixmap;
    registerChange();

    if(_batchChanges)
        _iconChanged = true;
    else
        emit iconChanged(this);
}

void MonsterClassv2::removeIcon(int index)
{
    if((index < 0) || (index >= _icons.count()))
        return;

    _icons.removeAt(index);
    _scaledPixmaps.removeAt(index);
}

void MonsterClassv2::removeIcon(const QString& iconFile)
{
    removeIcon(_icons.indexOf(iconFile));
}

void MonsterClassv2::searchForIcons()
{
    QStringList searchResult = Bestiary::Instance()->findMonsterImages(getStringValue("name"));
    for(int i = 0; i < searchResult.count(); ++i)
        addIcon(searchResult.at(i));
}

void MonsterClassv2::refreshIconPixmaps()
{
    _scaledPixmaps.clear();
    for(int i = 0; i < _icons.count(); ++i)
    {
        ScaledPixmap newPixmap;
        if(newPixmap.setBasePixmap(Bestiary::Instance()->getDirectory().filePath(_icons.at(i))))
            _scaledPixmaps.append(newPixmap);
    }

    registerChange();

    if(_batchChanges)
        _iconChanged = true;
    else
        emit iconChanged(this);
}

void MonsterClassv2::clearIcon()
{
    _icons.clear();
    _scaledPixmaps.clear();
    registerChange();

    if(_batchChanges)
        _iconChanged = true;
    else
        emit iconChanged(this);
}

QHash<QString, QVariant>* MonsterClassv2::valueHash()
{
    return &_allValues;
}

const QHash<QString, QVariant>* MonsterClassv2::valueHash() const
{
    return &_allValues;
}

void MonsterClassv2::declareDirty()
{
    registerChange();
}

void MonsterClassv2::registerChange()
{
    if(_batchChanges)
        _changesMade = true;
    else
        emit dirty();
}

void MonsterClassv2::readIcons(const QDomElement& element, bool isImport)
{
    Q_UNUSED(isImport);

    if(element.hasAttribute("icon"))
        addIcon(element.attribute("icon"));

    QDomElement iconElement = element.firstChildElement("icon");
    while(!iconElement.isNull())
    {
        addIcon(iconElement.attribute("filename"));
        iconElement = iconElement.nextSiblingElement("icon");
    }
}

void MonsterClassv2::writeIcons(QDomDocument &doc, QDomElement& element, QDir& targetDirectory, bool isExport) const
{
    Q_UNUSED(isExport);

    if(_icons.count() == 1)
    {
        if(!_icons.at(0).isEmpty())
            element.setAttribute("icon", targetDirectory.relativeFilePath(_icons.at(0)));
    }
    else
    {
        for(int i = 0; i < _icons.count(); ++i)
        {
            if(!_icons.at(i).isEmpty())
            {
                QDomElement iconElement = doc.createElement("icon");
                iconElement.setAttribute("filename", _icons.at(i));
                element.appendChild(iconElement);
            }
        }
    }
}
