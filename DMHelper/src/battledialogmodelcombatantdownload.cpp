#include "battledialogmodelcombatantdownload.h"
#include <QDomElement>
#include <QDebug>

BattleDialogModelCombatantDownload::BattleDialogModelCombatantDownload(QObject *parent) :
    BattleDialogModelCombatant(QString(), parent),
    _combatantId(),
    _sizeFactor(1.0),
    _conditions(0),
    _speed(30),
    _token(),
    _md5(),
    _uuid(),
    _tokenPixmap(new ScaledPixmap())
{
}

BattleDialogModelCombatantDownload::~BattleDialogModelCombatantDownload()
{
    delete _tokenPixmap;
}

void BattleDialogModelCombatantDownload::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    // Ignored
    // element.setAttribute("type", getCombatantType());

    // element.setAttribute("combatantId", getCombatant() ? getCombatant()->getID().toString() : QUuid().toString());
    _combatantId = QUuid(element.attribute("combatantId", QUuid().toString()));
    // element.setAttribute("sizeFactor", getSizeFactor());
    _sizeFactor = element.attribute("sizeFactor", QString::number(0)).toInt();
    // element.setAttribute("conditions", getConditions());
    setConditions(element.attribute("conditions", QString::number(0)).toInt());
    // element.setAttribute("name", getName());
    setName(element.attribute("name"));

    _token = element.attribute("token");
    interpretToken();

    // element.setAttribute("initiative", _initiative);
    // element.setAttribute("positionX", _position.x());
    // element.setAttribute("positionY", _position.y());
    // element.setAttribute("isShown", _isShown);
    /*
     Covers
     _initiative = element.attribute("initiative",QString::number(0)).toInt();
     _position = QPointF(element.attribute("positionX",QString::number(0)).toDouble(),
                         element.attribute("positionY",QString::number(0)).toDouble());
     _isShown = static_cast<bool>(element.attribute("isShown",QString::number(1)).toInt());
     _isKnown = static_cast<bool>(element.attribute("isKnown",QString::number(1)).toInt());
     */
    BattleDialogModelCombatant::inputXML(element, true);

    if(!getMD5().isEmpty())
        emit requestFile(getMD5(), getUuid(), DMHelper::FileType_Image);
}

QString BattleDialogModelCombatantDownload::getMD5() const
{
    return _md5;
}

void BattleDialogModelCombatantDownload::setMD5(const QString& md5)
{
    _md5 = md5;
}

int BattleDialogModelCombatantDownload::getCombatantType() const
{
    return DMHelper::CombatantType_Download;
}

BattleDialogModelCombatant* BattleDialogModelCombatantDownload::clone() const
{
    return nullptr;
}

qreal BattleDialogModelCombatantDownload::getSizeFactor() const
{
    return _sizeFactor;
}

int BattleDialogModelCombatantDownload::getSizeCategory() const
{
    return DMHelper::CombatantSize_Unknown;
}

int BattleDialogModelCombatantDownload::getStrength() const
{
    return 10;
}

int BattleDialogModelCombatantDownload::getDexterity() const
{
    return 10;
}

int BattleDialogModelCombatantDownload::getConstitution() const
{
    return 10;
}

int BattleDialogModelCombatantDownload::getIntelligence() const
{
    return 10;
}

int BattleDialogModelCombatantDownload::getWisdom() const
{
    return 10;
}

int BattleDialogModelCombatantDownload::getCharisma() const
{
    return 10;
}

int BattleDialogModelCombatantDownload::getSkillModifier(Combatant::Skills skill) const
{
    Q_UNUSED(skill);
    return 0;
}

int BattleDialogModelCombatantDownload::getConditions() const
{
    return _conditions;
}

bool BattleDialogModelCombatantDownload::hasCondition(Combatant::Condition condition) const
{
    return ((_conditions & condition) != 0);
}

int BattleDialogModelCombatantDownload::getSpeed() const
{
    return _speed;
}

int BattleDialogModelCombatantDownload::getArmorClass() const
{
    return 10;
}

int BattleDialogModelCombatantDownload::getHitPoints() const
{
    return 1;
}

void BattleDialogModelCombatantDownload::setHitPoints(int hitPoints)
{
    Q_UNUSED(hitPoints);
}

QPixmap BattleDialogModelCombatantDownload::getIconPixmap(DMHelper::PixmapSize iconSize) const
{
    if((_tokenPixmap) && (_tokenPixmap->isValid()))
        return _tokenPixmap->getPixmap(iconSize);
    else
        return ScaledPixmap::defaultPixmap()->getPixmap(iconSize);
}

QString BattleDialogModelCombatantDownload::getUuid() const
{
    return _uuid;
}

void BattleDialogModelCombatantDownload::setUuid(const QString& uuid)
{
    _uuid = uuid;
}

bool BattleDialogModelCombatantDownload::isTokenValid()
{
    return ((_tokenPixmap) && (_tokenPixmap->isValid()));
}

void BattleDialogModelCombatantDownload::fileReceived(const QString& md5, const QString& uuid, const QByteArray& data)
{
    if((!_tokenPixmap) || (_md5.isEmpty()) || (_md5 != md5))
        return;

    QPixmap tokenPixmap;
    if(!tokenPixmap.loadFromData(data))
    {
        qDebug() << "[BattleDialogModelCombatantDownload] ERROR: invalid token file received! MD5: " << md5 << ", UUID: " << uuid;
        return;
    }
    _tokenPixmap->setBasePixmap(tokenPixmap);
    setUuid(uuid);
}

void BattleDialogModelCombatantDownload::setConditions(int conditions)
{
    if(_conditions != conditions)
        _conditions = conditions;
}

void BattleDialogModelCombatantDownload::applyConditions(int conditions)
{
    if((_conditions & conditions) != conditions)
        _conditions |= conditions;
}

void BattleDialogModelCombatantDownload::removeConditions(int conditions)
{
    if((_conditions & ~conditions) != _conditions)
        _conditions &= ~conditions;
}

void BattleDialogModelCombatantDownload::interpretToken()
{
    QStringList splits = _token.split(QString(","));
    setMD5(splits.count() > 0 ? splits.at(0) : QString());
    setUuid(splits.count() > 1 ? splits.at(1) : QString());
}
