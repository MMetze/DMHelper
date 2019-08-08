#include "equipmentserver.h"
#include <QDebug>
#include <QDomDocument>
#include <QFileInfo>

EquipmentServer* EquipmentServer::_instance = nullptr;

EquipmentServer::EquipmentServer() :
    _magicItems(),
    _gearItems(),
    _weaponItems(),
    _armorItems(),
    _goodsItems(),
    _toolItems()
{
    readEquipment();
}

EquipmentServer* EquipmentServer::Instance()
{
    if(!_instance)
        Initialize();

    return _instance;
}

void EquipmentServer::Initialize()
{
    if(_instance)
        return;

    qDebug() << "[EquipmentServer] Initializing EquipmentServer";
    _instance = new EquipmentServer();
}

void EquipmentServer::Shutdown()
{
    delete _instance;
    _instance = nullptr;
}

QList<EquipmentServer::MagicItem> EquipmentServer::getMagicItems() const
{
    return _magicItems;
}

QList<EquipmentServer::GearItem> EquipmentServer::getGearItems() const
{
    return _gearItems;
}

QList<EquipmentServer::WeaponItem> EquipmentServer::getWeaponItems() const
{
    return _weaponItems;
}

QList<EquipmentServer::ArmorItem> EquipmentServer::getArmorItems() const
{
    return _armorItems;
}

QList<EquipmentServer::GoodsItem> EquipmentServer::getGoodsItems() const
{
    return _goodsItems;
}

QList<EquipmentServer::ToolItem> EquipmentServer::getToolItems() const
{
    return _toolItems;
}

void EquipmentServer::readEquipment()
{
    qDebug() << "[EquipmentServer] Reading equipment...";

    QString equipmentFileName("equipment.xml");

    QDomDocument doc("DMHelperDataXML");
    QFile file(equipmentFileName);
    qDebug() << "[EquipmentServer] Equipment file: " << QFileInfo(file).filePath();
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[EquipmentServer] Unable to read equipment file: " << equipmentFileName;
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString errMsg;
    int errRow;
    int errColumn;
    bool contentResult = doc.setContent(in.readAll(), &errMsg, &errRow, &errColumn);

    file.close();

    if(contentResult == false)
    {
        qDebug() << "[EquipmentServer] Unable to parse the equipment data file.";
        qDebug() << errMsg << errRow << errColumn;
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "root"))
    {
        qDebug() << "[EquipmentServer] Unable to find the root element in the equipment data file.";
        return;
    }

    QDomElement magicSection = root.firstChildElement(QString("magicitems"));
    if(magicSection.isNull())
    {
        qDebug() << "[EquipmentServer] Unable to find the magic item information element in the equipment data file.";
    }
    else
    {
        QDomElement magicElement = magicSection.firstChildElement(QString("magicitem"));
        while(!magicElement.isNull())
        {
            MagicItem newItem;
            newItem._name = magicElement.attribute(QString("name"));
            newItem._categoryText = magicElement.attribute(QString("category"));
            newItem._category = categoryFromString(newItem._categoryText, true);
            newItem._probability = probabilityFromString(magicElement.attribute(QString("probability")));
            newItem._attunement = magicElement.attribute(QString("attunement")) == QString("y");

            QDomElement subElement = magicElement.firstChildElement(QString("subcategory"));
            while(!subElement.isNull())
            {
                MagicSubItem newSubItem;
                newSubItem._name = subElement.attribute(QString("name"));
                newSubItem._categoryText = subElement.attribute(QString("category"));
                newSubItem._probability = probabilityFromString(subElement.attribute(QString("probability")));
                newSubItem._attunement = subElement.attribute(QString("attunement")) == QString("y");

                newItem._subitems.append(newSubItem);

                subElement = subElement.nextSiblingElement(QString("subcategory"));
            }

            _magicItems.append(newItem);

            magicElement = magicElement.nextSiblingElement(QString("magicitem"));
        }
    }

    QDomElement gearSection = root.firstChildElement(QString("adventuringgear"));
    if(gearSection.isNull())
    {
        qDebug() << "[EquipmentServer] Unable to find the gear information element in the equipment data file.";
    }
    else
    {
        QDomElement gearElement = gearSection.firstChildElement(QString("gear"));
        while(!gearElement.isNull())
        {
            GearItem newItem;
            newItem._name = gearElement.attribute(QString("name"));
            newItem._cost = gearElement.attribute(QString("cost"));
            newItem._weight = gearElement.attribute(QString("weight"));
            newItem._probability = probabilityFromString(gearElement.attribute(QString("probability")));

            QDomElement subElement = gearElement.firstChildElement(QString("subgear"));
            while(!subElement.isNull())
            {
                GearSubItem newSubItem;
                newSubItem._name = subElement.attribute(QString("name"));
                newSubItem._cost = subElement.attribute(QString("cost"));
                newSubItem._weight = subElement.attribute(QString("weight"));
                newSubItem._probability = probabilityFromString(subElement.attribute(QString("probability")));

                newItem._subitems.append(newSubItem);

                subElement = subElement.nextSiblingElement(QString("subgear"));
            }

            _gearItems.append(newItem);

            gearElement = gearElement.nextSiblingElement(QString("gear"));
        }
    }

    QDomElement weaponSection = root.firstChildElement(QString("weapons"));
    if(weaponSection.isNull())
    {
        qDebug() << "[EquipmentServer] Unable to find the weapon information element in the equipment data file.";
    }
    else
    {
        readWeaponSubSection(weaponSection, QString("simplemeleeweapons"));
        readWeaponSubSection(weaponSection, QString("simplerangedweapons"));
        readWeaponSubSection(weaponSection, QString("martialmeleeweapons"));
        readWeaponSubSection(weaponSection, QString("martialrangedweapons"));
    }

    QDomElement armorSection = root.firstChildElement(QString("armor"));
    if(armorSection.isNull())
    {
        qDebug() << "[EquipmentServer] Unable to find the armor information element in the equipment data file.";
    }
    else
    {
        readArmorSubSection(armorSection, QString("lightarmor"));
        readArmorSubSection(armorSection, QString("mediumarmor"));
        readArmorSubSection(armorSection, QString("heavyarmor"));
        readArmorSubSection(armorSection, QString("shield"));
    }

    QDomElement goodsSection = root.firstChildElement(QString("tradegoods"));
    if(goodsSection.isNull())
    {
        qDebug() << "[EquipmentServer] Unable to find the trade goods information element in the equipment data file.";
    }
    else
    {
        QDomElement goodsElement = goodsSection.firstChildElement(QString("tradegood"));
        while(!goodsElement.isNull())
        {
            GoodsItem newItem;
            newItem._name = goodsElement.attribute(QString("name"));
            newItem._cost = goodsElement.attribute(QString("cost"));
            newItem._probability = probabilityFromString(goodsElement.attribute(QString("probability")));

            _goodsItems.append(newItem);

            goodsElement = goodsElement.nextSiblingElement(QString("tradegood"));
        }
    }

    QDomElement toolSection = root.firstChildElement(QString("tools"));
    if(toolSection.isNull())
    {
        qDebug() << "[EquipmentServer] Unable to find the tool information element in the equipment data file.";
    }
    else
    {
        QDomElement toolElement = toolSection.firstChildElement(QString("tool"));
        while(!toolElement.isNull())
        {
            ToolItem newItem;
            newItem._name = toolElement.attribute(QString("name"));
            newItem._cost = toolElement.attribute(QString("cost"));
            newItem._weight = toolElement.attribute(QString("weight"));
            newItem._probability = probabilityFromString(toolElement.attribute(QString("probability")));

            QDomElement subElement = toolElement.firstChildElement(QString("subtool"));
            while(!subElement.isNull())
            {
                ToolSubItem newSubItem;
                newSubItem._name = subElement.attribute(QString("name"));
                newSubItem._cost = subElement.attribute(QString("cost"));
                newSubItem._weight = subElement.attribute(QString("weight"));
                newSubItem._probability = probabilityFromString(subElement.attribute(QString("probability")));

                newItem._subitems.append(newSubItem);

                subElement = subElement.nextSiblingElement(QString("subtool"));
            }

            _toolItems.append(newItem);

            toolElement = toolElement.nextSiblingElement(QString("tool"));
        }
    }

    qDebug() << "[EquipmentServer] Completed reading equipment.";
}

EquipmentServer::ItemProbability EquipmentServer::probabilityFromString(QString probability)
{
    if(probability == QString("always"))
        return EquipmentServer::Probability_Always;

    if(probability == QString("common"))
        return EquipmentServer::Probability_Common;

    if(probability == QString("uncommon"))
        return EquipmentServer::Probability_Uncommon;

    if(probability == QString("rare"))
        return EquipmentServer::Probability_Rare;

    if(probability == QString("very rare"))
        return EquipmentServer::Probability_Very_Rare;

    if(probability == QString("legendary"))
        return EquipmentServer::Probability_Legendary;

    if(probability == QString(""))
        return EquipmentServer::Probability_None;

    qDebug() << "[EquipmentServer] Unknown probability seen: " << probability;

    return EquipmentServer::Probability_Common;
}

EquipmentServer::ItemCategory EquipmentServer::categoryFromString(QString category, bool isMagic)
{
    if(category == QString("Potion"))
        return Category_Magic_Potion;

    if(category == QString("Ring"))
        return Category_Magic_Ring;

    if(category == QString("Rod"))
        return Category_Magic_Rod;

    if(category == QString("Scroll"))
        return Category_Magic_Scroll;

    if(category == QString("Staff"))
        return Category_Magic_Staff;

    if(category == QString("Wand"))
        return Category_Magic_Wand;

    if(category == QString("Wondrous item"))
        return Category_Magic_Wondrousitem;

    if(category == QString("Vehicle"))
        return Category_Vehicles;

    if(category.left(6) == QString("Weapon"))
        return isMagic ? Category_Magic_Weapon : Category_Weapon;

    if(category.left(5) == QString("Armor"))
        return isMagic ? Category_Magic_Armor : Category_Armor;

    qDebug() << "[EquipmentServer] Unknown category seen: " << category;

    return EquipmentServer::Category_Mundane;
}

void EquipmentServer::readWeaponSubSection(QDomElement weaponSection, QString subSectionName)
{
    QDomElement weaponSubSection = weaponSection.firstChildElement(subSectionName);

    QDomElement weaponElement = weaponSubSection.firstChildElement(QString("weapon"));
    while(!weaponElement.isNull())
    {
        WeaponItem newItem;
        newItem._name = weaponElement.attribute(QString("name"));
        newItem._cost = weaponElement.attribute(QString("cost"));
        newItem._weight = weaponElement.attribute(QString("weight"));
        newItem._probability = probabilityFromString(weaponElement.attribute(QString("probability")));
        _weaponItems.append(newItem);

        weaponElement = weaponElement.nextSiblingElement(QString("weapon"));
    }
}

void EquipmentServer::readArmorSubSection(QDomElement armorSection, QString subSectionName)
{
    QDomElement armorSubSection = armorSection.firstChildElement(subSectionName);

    QDomElement armorElement = armorSubSection.firstChildElement(QString("armor"));
    while(!armorElement.isNull())
    {
        ArmorItem newItem;
        newItem._name = armorElement.attribute(QString("type"));
        newItem._cost = armorElement.attribute(QString("cost"));
        newItem._weight = armorElement.attribute(QString("weight"));
        newItem._probability = probabilityFromString(armorElement.attribute(QString("probability")));
        _armorItems.append(newItem);

        armorElement = armorElement.nextSiblingElement(QString("armor"));
    }
}
