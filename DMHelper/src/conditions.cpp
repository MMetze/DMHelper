#include "conditions.h"
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QDebug>

// Old bitmask condition ID strings — used only in migrateFromBitmask()
namespace {
    const QString LEGACY_ID_BLINDED       = QStringLiteral("blinded");
    const QString LEGACY_ID_CHARMED       = QStringLiteral("charmed");
    const QString LEGACY_ID_DEAFENED      = QStringLiteral("deafened");
    const QString LEGACY_ID_EXHAUSTION_1  = QStringLiteral("exhaustion1");
    const QString LEGACY_ID_EXHAUSTION_2  = QStringLiteral("exhaustion2");
    const QString LEGACY_ID_EXHAUSTION_3  = QStringLiteral("exhaustion3");
    const QString LEGACY_ID_EXHAUSTION_4  = QStringLiteral("exhaustion4");
    const QString LEGACY_ID_EXHAUSTION_5  = QStringLiteral("exhaustion5");
    const QString LEGACY_ID_FRIGHTENED    = QStringLiteral("frightened");
    const QString LEGACY_ID_GRAPPLED      = QStringLiteral("grappled");
    const QString LEGACY_ID_INCAPACITATED = QStringLiteral("incapacitated");
    const QString LEGACY_ID_INVISIBLE     = QStringLiteral("invisible");
    const QString LEGACY_ID_PARALYZED     = QStringLiteral("paralyzed");
    const QString LEGACY_ID_PETRIFIED     = QStringLiteral("petrified");
    const QString LEGACY_ID_POISONED      = QStringLiteral("poisoned");
    const QString LEGACY_ID_PRONE         = QStringLiteral("prone");
    const QString LEGACY_ID_RESTRAINED    = QStringLiteral("restrained");
    const QString LEGACY_ID_STUNNED       = QStringLiteral("stunned");
    const QString LEGACY_ID_UNCONSCIOUS   = QStringLiteral("unconscious");
}

Conditions* Conditions::_activeConditions = nullptr;

// --- ConditionDefinition ---

bool ConditionDefinition::operator==(const ConditionDefinition& other) const
{
    return id == other.id
        && title == other.title
        && description == other.description
        && iconName == other.iconName
        && customIconPath == other.customIconPath
        && group == other.group;
}

bool ConditionDefinition::operator!=(const ConditionDefinition& other) const
{
    return !(*this == other);
}

// --- Conditions ---

Conditions::Conditions(QObject *parent) :
    QObject(parent),
    _conditions(),
    _conditionIndex(),
    _iconCache()
{
}

Conditions::~Conditions()
{
    if(_activeConditions == this)
        _activeConditions = nullptr;
}

Conditions* Conditions::activeConditions()
{
    return _activeConditions;
}

void Conditions::setActiveConditions(Conditions* conditions)
{
    _activeConditions = conditions;
}

bool Conditions::loadFromFile(const QString& filename)
{
    if(filename.isEmpty())
        return false;

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[Conditions] ERROR: Unable to open conditions file:" << filename;
        return false;
    }

    QDomDocument doc("ConditionsXML");
    QDomDocument::ParseResult result = doc.setContent(file.readAll());
    file.close();

    if(!result)
    {
        qDebug() << "[Conditions] ERROR: Unable to parse conditions file:" << filename
                 << "at line" << result.errorLine << ":" << result.errorMessage;
        return false;
    }

    QDomElement root = doc.documentElement();
    if(root.isNull() || root.tagName() != QStringLiteral("conditions"))
    {
        qDebug() << "[Conditions] ERROR: Invalid root element in conditions file:" << filename;
        return false;
    }

    QDir fileDir = QFileInfo(filename).absoluteDir();

    _conditions.clear();
    _conditionIndex.clear();
    clearIconCache();

    QDomElement condElement = root.firstChildElement(QStringLiteral("condition"));
    while(!condElement.isNull())
    {
        ConditionDefinition def;
        def.id = condElement.attribute(QStringLiteral("id"));
        if(!def.id.isEmpty())
        {
            def.title = condElement.attribute(QStringLiteral("title"));
            def.description = condElement.attribute(QStringLiteral("description"));
            def.iconName = condElement.attribute(QStringLiteral("icon"));
            def.group = condElement.attribute(QStringLiteral("group"));

            QString customPath = condElement.attribute(QStringLiteral("customIconPath"));
            if(!customPath.isEmpty())
                def.customIconPath = fileDir.absoluteFilePath(customPath);

            _conditionIndex.insert(def.id, _conditions.size());
            _conditions.append(def);
        }

        condElement = condElement.nextSiblingElement(QStringLiteral("condition"));
    }

    qDebug() << "[Conditions] Loaded" << _conditions.size() << "conditions from" << filename;
    return true;
}

void Conditions::applyDeltas(const QDomElement& deltasElement, const QDir& baseDir)
{
    if(deltasElement.isNull())
        return;

    QDomElement condElement = deltasElement.firstChildElement(QStringLiteral("condition"));
    while(!condElement.isNull())
    {
        QString id = condElement.attribute(QStringLiteral("id"));
        QString action = condElement.attribute(QStringLiteral("action"));

        if(!id.isEmpty())
        {
            if(action == QStringLiteral("remove"))
            {
                removeCondition(id);
            }
            else if(action == QStringLiteral("add"))
            {
                ConditionDefinition def;
                def.id = id;
                def.title = condElement.attribute(QStringLiteral("title"));
                def.description = condElement.attribute(QStringLiteral("description"));
                def.iconName = condElement.attribute(QStringLiteral("icon"));
                def.group = condElement.attribute(QStringLiteral("group"));
                QString customPath = condElement.attribute(QStringLiteral("customIconPath"));
                if(!customPath.isEmpty())
                    def.customIconPath = baseDir.absoluteFilePath(customPath);
                addCondition(def);
            }
            else if(action == QStringLiteral("modify"))
            {
                if(hasConditionDef(id))
                {
                    ConditionDefinition def = getCondition(id);
                    if(condElement.hasAttribute(QStringLiteral("title")))
                        def.title = condElement.attribute(QStringLiteral("title"));
                    if(condElement.hasAttribute(QStringLiteral("description")))
                        def.description = condElement.attribute(QStringLiteral("description"));
                    if(condElement.hasAttribute(QStringLiteral("icon")))
                        def.iconName = condElement.attribute(QStringLiteral("icon"));
                    if(condElement.hasAttribute(QStringLiteral("group")))
                        def.group = condElement.attribute(QStringLiteral("group"));
                    if(condElement.hasAttribute(QStringLiteral("customIconPath")))
                    {
                        QString customPath = condElement.attribute(QStringLiteral("customIconPath"));
                        def.customIconPath = customPath.isEmpty() ? QString() : baseDir.absoluteFilePath(customPath);
                    }
                    modifyCondition(def);
                }
            }
        }

        condElement = condElement.nextSiblingElement(QStringLiteral("condition"));
    }
}

void Conditions::outputDeltas(QDomDocument& doc, QDomElement& parent, const Conditions& rulesetDefaults, const QDir& targetDirectory) const
{
    QDomElement deltasElement = doc.createElement(QStringLiteral("conditionDeltas"));
    bool hasDelta = false;

    // Check for removed conditions (present in defaults, absent here)
    for(const ConditionDefinition& defaultDef : rulesetDefaults._conditions)
    {
        if(!hasConditionDef(defaultDef.id))
        {
            QDomElement condElement = doc.createElement(QStringLiteral("condition"));
            condElement.setAttribute(QStringLiteral("id"), defaultDef.id);
            condElement.setAttribute(QStringLiteral("action"), QStringLiteral("remove"));
            deltasElement.appendChild(condElement);
            hasDelta = true;
        }
    }

    // Check for added or modified conditions
    for(const ConditionDefinition& def : _conditions)
    {
        if(!rulesetDefaults.hasConditionDef(def.id))
        {
            // Added condition
            QDomElement condElement = doc.createElement(QStringLiteral("condition"));
            condElement.setAttribute(QStringLiteral("id"), def.id);
            condElement.setAttribute(QStringLiteral("action"), QStringLiteral("add"));
            condElement.setAttribute(QStringLiteral("title"), def.title);
            if(!def.description.isEmpty())
                condElement.setAttribute(QStringLiteral("description"), def.description);
            if(!def.iconName.isEmpty())
                condElement.setAttribute(QStringLiteral("icon"), def.iconName);
            if(!def.group.isEmpty())
                condElement.setAttribute(QStringLiteral("group"), def.group);
            if(!def.customIconPath.isEmpty())
                condElement.setAttribute(QStringLiteral("customIconPath"), targetDirectory.relativeFilePath(def.customIconPath));
            deltasElement.appendChild(condElement);
            hasDelta = true;
        }
        else
        {
            ConditionDefinition defaultDef = rulesetDefaults.getCondition(def.id);
            if(def != defaultDef)
            {
                // Modified condition
                QDomElement condElement = doc.createElement(QStringLiteral("condition"));
                condElement.setAttribute(QStringLiteral("id"), def.id);
                condElement.setAttribute(QStringLiteral("action"), QStringLiteral("modify"));
                if(def.title != defaultDef.title)
                    condElement.setAttribute(QStringLiteral("title"), def.title);
                if(def.description != defaultDef.description)
                    condElement.setAttribute(QStringLiteral("description"), def.description);
                if(def.iconName != defaultDef.iconName)
                    condElement.setAttribute(QStringLiteral("icon"), def.iconName);
                if(def.group != defaultDef.group)
                    condElement.setAttribute(QStringLiteral("group"), def.group);
                if(def.customIconPath != defaultDef.customIconPath)
                {
                    if(def.customIconPath.isEmpty())
                        condElement.setAttribute(QStringLiteral("customIconPath"), QString());
                    else
                        condElement.setAttribute(QStringLiteral("customIconPath"), targetDirectory.relativeFilePath(def.customIconPath));
                }
                deltasElement.appendChild(condElement);
                hasDelta = true;
            }
        }
    }

    if(hasDelta)
        parent.appendChild(deltasElement);
}

bool Conditions::hasDeltasFrom(const Conditions& rulesetDefaults) const
{
    if(_conditions.size() != rulesetDefaults._conditions.size())
        return true;

    for(const ConditionDefinition& def : _conditions)
    {
        if(!rulesetDefaults.hasConditionDef(def.id))
            return true;
        if(def != rulesetDefaults.getCondition(def.id))
            return true;
    }

    for(const ConditionDefinition& defaultDef : rulesetDefaults._conditions)
    {
        if(!hasConditionDef(defaultDef.id))
            return true;
    }

    return false;
}

int Conditions::getConditionCount() const
{
    return _conditions.size();
}

QList<ConditionDefinition> Conditions::getConditions() const
{
    return _conditions;
}

ConditionDefinition Conditions::getCondition(const QString& id) const
{
    if(_conditionIndex.contains(id))
        return _conditions.at(_conditionIndex.value(id));
    return ConditionDefinition();
}

bool Conditions::hasConditionDef(const QString& id) const
{
    return _conditionIndex.contains(id);
}

QString Conditions::getConditionIconPath(const QString& id) const
{
    if(!_conditionIndex.contains(id))
        return QString();

    const ConditionDefinition& def = _conditions.at(_conditionIndex.value(id));
    return resolveIconPath(def);
}

QString Conditions::getConditionTitle(const QString& id) const
{
    if(!_conditionIndex.contains(id))
        return QString();
    return _conditions.at(_conditionIndex.value(id)).title;
}

QString Conditions::getConditionDescription(const QString& id) const
{
    if(!_conditionIndex.contains(id))
        return QString();
    return _conditions.at(_conditionIndex.value(id)).description;
}

QStringList Conditions::getConditionIds() const
{
    QStringList result;
    for(const ConditionDefinition& def : _conditions)
        result.append(def.id);
    return result;
}

QPixmap Conditions::getConditionPixmap(const QString& id, int size)
{
    QString cacheKey = id + QStringLiteral("_") + QString::number(size);
    if(_iconCache.contains(cacheKey))
        return _iconCache.value(cacheKey);

    QString path = getConditionIconPath(id);
    if(path.isEmpty())
        return QPixmap();

    QPixmap pix(path);
    if(pix.isNull())
        return QPixmap();

    QPixmap scaled = pix.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    _iconCache.insert(cacheKey, scaled);
    return scaled;
}

void Conditions::invalidateIconCache(const QString& id)
{
    QMutableHashIterator<QString, QPixmap> it(_iconCache);
    while(it.hasNext())
    {
        it.next();
        if(it.key().startsWith(id + QStringLiteral("_")))
            it.remove();
    }
}

void Conditions::clearIconCache()
{
    _iconCache.clear();
}

void Conditions::addCondition(const ConditionDefinition& condition)
{
    if(condition.id.isEmpty() || _conditionIndex.contains(condition.id))
        return;

    _conditionIndex.insert(condition.id, _conditions.size());
    _conditions.append(condition);
}

void Conditions::modifyCondition(const ConditionDefinition& condition)
{
    if(!_conditionIndex.contains(condition.id))
        return;

    int index = _conditionIndex.value(condition.id);
    _conditions[index] = condition;
    invalidateIconCache(condition.id);
}

void Conditions::removeCondition(const QString& id)
{
    if(!_conditionIndex.contains(id))
        return;

    int index = _conditionIndex.value(id);
    _conditions.removeAt(index);
    invalidateIconCache(id);

    // Rebuild the index
    _conditionIndex.clear();
    for(int i = 0; i < _conditions.size(); ++i)
        _conditionIndex.insert(_conditions.at(i).id, i);
}

void Conditions::clear()
{
    _conditions.clear();
    _conditionIndex.clear();
    clearIconCache();
}

void Conditions::drawConditions(QPaintDevice* target, const QStringList& activeConditions)
{
    if(!target || activeConditions.isEmpty())
        return;

    Conditions* conds = Conditions::activeConditions();
    if(!conds)
        return;

    int spacing = target->width() / 20;
    int iconSize = (target->width() / 3) - spacing;
    if((spacing <= 0) || (iconSize <= 5) || (iconSize + spacing >= target->height()))
    {
        qDebug() << "[Conditions] spacing or icon size are not ok to draw conditions. Spacing:" << spacing
                 << ", icon size:" << iconSize << ", target:" << target->width() << "x" << target->height();
        return;
    }

    QPainter painter(target);
    int cx = spacing;
    int cy = spacing;

    for(const QString& condId : activeConditions)
    {
        if(cy > target->height() - iconSize)
            break;

        QPixmap pix = conds->getConditionPixmap(condId, iconSize);
        if(pix.isNull())
            continue;

        painter.drawPixmap(cx, cy, pix);
        cx += iconSize + spacing;
        if(cx > target->width() - iconSize)
        {
            cx = spacing;
            cy += iconSize;
        }
    }
}

QStringList Conditions::getConditionStrings(const QStringList& conditionIds)
{
    QStringList result;
    Conditions* conds = activeConditions();
    for(const QString& id : conditionIds)
    {
        if(conds)
        {
            QString desc = conds->getConditionDescription(id);
            result.append(desc.isEmpty() ? id : desc);
        }
        else
        {
            result.append(id);
        }
    }
    return result;
}

QStringList Conditions::migrateFromBitmask(int bitmask)
{
    QStringList result;
    if(bitmask == 0)
        return result;

    // Map old enum bit positions to condition IDs
    if(bitmask & 0x00000001) result.append(LEGACY_ID_BLINDED);
    if(bitmask & 0x00000002) result.append(LEGACY_ID_CHARMED);
    if(bitmask & 0x00000004) result.append(LEGACY_ID_DEAFENED);
    if(bitmask & 0x00000008) result.append(LEGACY_ID_EXHAUSTION_1);
    if(bitmask & 0x00000010) result.append(LEGACY_ID_EXHAUSTION_2);
    if(bitmask & 0x00000020) result.append(LEGACY_ID_EXHAUSTION_3);
    if(bitmask & 0x00000040) result.append(LEGACY_ID_EXHAUSTION_4);
    if(bitmask & 0x00000080) result.append(LEGACY_ID_EXHAUSTION_5);
    if(bitmask & 0x00000100) result.append(LEGACY_ID_FRIGHTENED);
    if(bitmask & 0x00000200) result.append(LEGACY_ID_GRAPPLED);
    if(bitmask & 0x00000400) result.append(LEGACY_ID_INCAPACITATED);
    if(bitmask & 0x00000800) result.append(LEGACY_ID_INVISIBLE);
    if(bitmask & 0x00001000) result.append(LEGACY_ID_PARALYZED);
    if(bitmask & 0x00002000) result.append(LEGACY_ID_PETRIFIED);
    if(bitmask & 0x00004000) result.append(LEGACY_ID_POISONED);
    if(bitmask & 0x00008000) result.append(LEGACY_ID_PRONE);
    if(bitmask & 0x00010000) result.append(LEGACY_ID_RESTRAINED);
    if(bitmask & 0x00020000) result.append(LEGACY_ID_STUNNED);
    if(bitmask & 0x00040000) result.append(LEGACY_ID_UNCONSCIOUS);

    return result;
}

QString Conditions::resolveIconPath(const ConditionDefinition& def)
{
    // Check custom icon first
    if(!def.customIconPath.isEmpty() && QFile::exists(def.customIconPath))
        return def.customIconPath;

    // Fall back to resource icon
    if(!def.iconName.isEmpty())
        return QStringLiteral(":/img/data/img/") + def.iconName + QStringLiteral(".png");

    return QString();
}
