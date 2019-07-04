#include "equipmentserver.h"
#include <QDebug>
#include <QDomDocument>

EquipmentServer* EquipmentServer::_instance = nullptr;

EquipmentServer::EquipmentServer() :
    _magicItems()
{
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

void EquipmentServer::readEquipment()
{
    qDebug() << "[EquipmentServer] Reading equipment...";

    QString equipmentFileName("equipment.xml");

    QDomDocument doc("DMHelperDataXML");
    QFile file(equipmentFileName);
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
            newItem._category = magicElement.attribute(QString("category"));
            newItem._probability = probabilityFromString(magicElement.attribute(QString("probability")));
            newItem._attunement = magicElement.attribute(QString("attunement"));

            QDomElement subElement = magicElement.firstChildElement(QString("subcategory"));
            while(!subElement.isNull())
            {
                MagicSubItem newSubItem;
                newSubItem._name = subElement.attribute(QString("name"));
                newSubItem._category = subElement.attribute(QString("category"));
                newSubItem._probability = probabilityFromString(subElement.attribute(QString("probability")));
                newSubItem._attunement = subElement.attribute(QString("attunement"));

                newItem._subitems.append(newSubItem);

                subElement = subElement.nextSiblingElement(QString("subcategory"));
            }

            _magicItems.append(newItem);

            magicElement = magicElement.nextSiblingElement(QString("magicitem"));
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

    qDebug() << "[EquipmentServer] Unknown probability seen: " << probability;

    return EquipmentServer::Probability_Common;
}
