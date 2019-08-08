#include "dmscreentabwidget.h"
#include "dicerolldialog.h"
#include "ui_dmscreentabwidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include <QDomElement>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

DMScreenTabWidget::DMScreenTabWidget(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::DMScreenTabWidget)
{
    ui->setupUi(this);

    readEquipment();
}

DMScreenTabWidget::~DMScreenTabWidget()
{
    delete ui;
}

void DMScreenTabWidget::readEquipment()
{
    qDebug() << "[DMScreen] Reading equipment for DM Screen";

#ifdef Q_OS_MAC
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    fileDirPath.cdUp();
    fileDirPath.cdUp();
    fileDirPath.cdUp();
    QString equipmentFileName = fileDirPath.path() + QString("/equipment.xml");
#else
    QString equipmentFileName("equipment.xml");
#endif

    QDomDocument doc("DMHelperDataXML");
    QFile file(equipmentFileName);
    qDebug() << "[DMScreen] Equipment file: " << QFileInfo(file).filePath();
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[DMScreen] Unable to read equipment file: " << equipmentFileName;
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
        qDebug() << "[DMScreen] Unable to parse the equipment data file.";
        qDebug() << errMsg << errRow << errColumn;
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "root"))
    {
        qDebug() << "[DMScreen] Unable to find the root element in the equipment data file.";
        return;
    }

    QDomElement weaponsElement = root.firstChildElement(QString("weapons"));
    if(weaponsElement.isNull())
    {
        qDebug() << "[DMScreen] Unable to find the weapons information element in the equipment data file.";
    }
    else
    {
        readWeaponSection(weaponsElement.firstChildElement(QString("simplemeleeweapons")),
                          *ui->layoutSimpleMeleeName,
                          *ui->layoutSimpleMeleeCost,
                          *ui->layoutSimpleMeleeDamage,
                          *ui->layoutSimpleMeleeWeight,
                          *ui->layoutSimpleMeleeProperties);

        readWeaponSection(weaponsElement.firstChildElement(QString("simplerangedweapons")),
                          *ui->layoutSimpleRangedName,
                          *ui->layoutSimpleRangedCost,
                          *ui->layoutSimpleRangedDamage,
                          *ui->layoutSimpleRangedWeight,
                          *ui->layoutSimpleRangedProperties);

        readWeaponSection(weaponsElement.firstChildElement(QString("martialmeleeweapons")),
                          *ui->layoutMartialMeleeName,
                          *ui->layoutMartialMeleeCost,
                          *ui->layoutMartialMeleeDamage,
                          *ui->layoutMartialMeleeWeight,
                          *ui->layoutMartialMeleeProperties);

        readWeaponSection(weaponsElement.firstChildElement(QString("martialrangedweapons")),
                          *ui->layoutMartialRangedName,
                          *ui->layoutMartialRangedCost,
                          *ui->layoutMartialRangedDamage,
                          *ui->layoutMartialRangedWeight,
                          *ui->layoutMartialRangedProperties);
    }

    QDomElement armorElement = root.firstChildElement(QString("armor"));
    if(armorElement.isNull())
    {
        qDebug() << "[DMScreen] Unable to find the armor information element in the equipment data file.";
    }
    else
    {
        int armorPosition = 2;
        readArmorSection(armorElement.firstChildElement(QString("lightarmor")), armorPosition);
        ++armorPosition;
        readArmorSection(armorElement.firstChildElement(QString("mediumarmor")), armorPosition);
        ++armorPosition;
        readArmorSection(armorElement.firstChildElement(QString("heavyarmor")), armorPosition);
        ++armorPosition;
        readArmorSection(armorElement.firstChildElement(QString("shield")), armorPosition);
    }

    QDomElement gearSection = root.firstChildElement(QString("adventuringgear"));
    if(gearSection.isNull())
    {
        qDebug() << "[DMScreen] Unable to find the adventuring gear information element in the equipment data file.";
    }
    else
    {
        int gearCount = countAllEquipment(gearSection, QString("gear"), QString("subgear"));
        int sectionGearCount = gearCount / 4;
        if(gearCount % 4 > 0)
            sectionGearCount++;
        QDomElement gearElement = gearSection.firstChildElement(QString("gear"));
        readEquipmentSection(gearElement, sectionGearCount, QString("gear"), QString("subgear"), *ui->layoutGearName1, QString("name"), *ui->layoutGearCost1, QString("cost"), *ui->layoutGearWeight1, QString("weight"));
        readEquipmentSection(gearElement, sectionGearCount, QString("gear"), QString("subgear"), *ui->layoutGearName2, QString("name"), *ui->layoutGearCost2, QString("cost"), *ui->layoutGearWeight2, QString("weight"));
        readEquipmentSection(gearElement, sectionGearCount, QString("gear"), QString("subgear"), *ui->layoutGearName3, QString("name"), *ui->layoutGearCost3, QString("cost"), *ui->layoutGearWeight3, QString("weight"));
        readEquipmentSection(gearElement, sectionGearCount, QString("gear"), QString("subgear"), *ui->layoutGearName4, QString("name"), *ui->layoutGearCost4, QString("cost"), *ui->layoutGearWeight4, QString("weight"));

        if(!gearElement.isNull())
            qDebug() << "[DMScreen] Not all adventuring gear elements were added to the DM Screen!";
    }

    readSimpleSection(root, QString("containers"), QString("container"), QString(), *ui->layoutContainerName, QString("name"), *ui->layoutContainerCapacity, QString("capacity"));

    QDomElement toolSection = root.firstChildElement(QString("tools"));
    if(toolSection.isNull())
    {
        qDebug() << "[DMScreen] Unable to find the tool information element in the equipment data file.";
    }
    else
    {
        int toolCount = countAllEquipment(toolSection, QString("tool"), QString("subtool"));
        int sectionToolCount = toolCount / 2;
        if(toolCount % 2 > 0)
            sectionToolCount++;
        QDomElement toolElement = toolSection.firstChildElement(QString("tool"));
        readEquipmentSection(toolElement, sectionToolCount, QString("tool"), QString("subtool"), *ui->layoutToolName1, QString("name"), *ui->layoutToolCost1, QString("cost"), *ui->layoutToolWeight1, QString("weight"));
        readEquipmentSection(toolElement, sectionToolCount, QString("tool"), QString("subtool"), *ui->layoutToolName2, QString("name"), *ui->layoutToolCost2, QString("cost"), *ui->layoutToolWeight2, QString("weight"));

        if(!toolElement.isNull())
            qDebug() << "[DMScreen] Not all tool elements were added to the DM Screen!";
    }

    readSimpleSection(root, QString("tradegoods"), QString("tradegood"), QString(), *ui->layoutGoodName, QString("name"), *ui->layoutGoodCost, QString("cost"));

    readSimpleSection(root, QString("animals"), QString("animal"), QString(), *ui->layoutAnimalsName, QString("name"), *ui->layoutAnimalsCost, QString("cost"), *ui->layoutAnimalsSpeed, QString("speed"), *ui->layoutAnimalsCapacity, QString("capacity"));

    readSimpleSection(root, QString("vehicles"), QString("vehicle"), QString(), *ui->layoutVehiclesName, QString("name"), *ui->layoutVehiclesCost, QString("cost"), *ui->layoutVehiclesSpeed, QString("speed"));

    readSimpleSection(root, QString("tacks"), QString("tack"), QString(), *ui->layoutTackName, QString("name"), *ui->layoutTackCost, QString("cost"), *ui->layoutTackWeight, QString("weight"));

    readSimpleSection(root, QString("food"), QString("fooditem"), QString("foodsubitem"), *ui->layoutFoodName, QString("name"), *ui->layoutFoodCost, QString("cost"));

    readSimpleSection(root, QString("service"), QString("serviceitem"), QString("servicesubitem"), *ui->layoutServiceName, QString("name"), *ui->layoutServicePay, QString("cost"));

    QDomElement magicSection = root.firstChildElement(QString("magicitems"));
    if(magicSection.isNull())
    {
        qDebug() << "[DMScreen] Unable to find the magic item information element in the equipment data file.";
    }
    else
    {
        int magicCount = countAllEquipment(magicSection, QString("magicitem"), QString("subcategory"));
        int sectionMagicCount = magicCount / 3;
        if(magicCount % 3 > 0)
            sectionMagicCount++;
        QDomElement magicElement = magicSection.firstChildElement(QString("magicitem"));
        readEquipmentSection(magicElement, sectionMagicCount, QString("magicitem"), QString("subcategory"), *ui->layoutMagicName1, QString("name"), *ui->layoutMagicType1, QString("category"), *ui->layoutMagicProbability1, QString("probability"), *ui->layoutMagicAttunement1, QString("attunement"));
        readEquipmentSection(magicElement, sectionMagicCount, QString("magicitem"), QString("subcategory"), *ui->layoutMagicName2, QString("name"), *ui->layoutMagicType2, QString("category"), *ui->layoutMagicProbability2, QString("probability"), *ui->layoutMagicAttunement2, QString("attunement"));
        readEquipmentSection(magicElement, sectionMagicCount, QString("magicitem"), QString("subcategory"), *ui->layoutMagicName3, QString("name"), *ui->layoutMagicType3, QString("category"), *ui->layoutMagicProbability3, QString("probability"), *ui->layoutMagicAttunement3, QString("attunement"));

        if(!magicElement.isNull())
            qDebug() << "[DMScreen] Not all magic item elements were added to the DM Screen!";
    }

    ui->scrollAreaWidgetContents->layout()->activate();
    ui->scrollAreaWidgetContents_2->layout()->activate();
    ui->scrollAreaWidgetContents_3->layout()->activate();
    ui->scrollAreaWidgetContents_4->layout()->activate();
    ui->scrollAreaWidgetContents_5->layout()->activate();
    ui->scrollAreaWidgetContents_6->layout()->activate();

    qDebug() << "[DMScreen] Completed reading equipment for DM Screen";
}

void DMScreenTabWidget::readWeaponSection(const QDomElement& section, QLayout& layoutName, QLayout& layoutCost, QLayout& layoutDamage, QLayout& layoutWeight, QLayout& layoutProperties)
{
    if(section.isNull())
    {
        qDebug() << "[DMScreen] Weapon section is unexpectedly not found.";
        return;
    }

    QDomElement weaponElement = section.firstChildElement(QString("weapon"));
    while(!weaponElement.isNull())
    {
        layoutName.addWidget(createLabel(weaponElement.attribute("name")));
        layoutCost.addWidget(createLabel(weaponElement.attribute("cost"), true));
        layoutDamage.addWidget(createLabel(weaponElement.attribute("damage"), true));
        layoutWeight.addWidget(createLabel(weaponElement.attribute("weight"), true));
        layoutProperties.addWidget(createLabel(weaponElement.attribute("properties")));
        weaponElement = weaponElement.nextSiblingElement(QString("weapon"));
    }
}

void DMScreenTabWidget::readArmorSection(const QDomElement& section, int& position)
{
    if(section.isNull())
    {
        qDebug() << "[DMScreen] Armor section is unexpectedly not found.";
        return;
    }

    QDomElement armorElement = section.firstChildElement(QString("armor"));
    while(!armorElement.isNull())
    {
        ui->layoutArmorType->insertWidget(position, createLabel(armorElement.attribute("type")));
        ui->layoutArmorCost->insertWidget(position, createLabel(armorElement.attribute("cost"), true));
        ui->layoutArmorAC->insertWidget(position, createLabel(armorElement.attribute("ac"), true));
        ui->layoutArmorStr->insertWidget(position, createLabel(armorElement.attribute("strength"), true));
        ui->layoutArmorStealth->insertWidget(position, createLabel(armorElement.attribute("stealth"), true));
        ui->layoutArmorWeight->insertWidget(position, createLabel(armorElement.attribute("weight"), true));
        armorElement = armorElement.nextSiblingElement(QString("armor"));
        ++position;
    }
}

int DMScreenTabWidget::countAllEquipment(const QDomElement& section, QString itemLabel, QString subitemLabel)
{
    int result = 0;

    if(!section.isNull())
    {
        QDomElement element = section.firstChildElement(itemLabel);
        while(!element.isNull())
        {
            ++result;
            QDomElement subElement = element.firstChildElement(subitemLabel);
            while(!subElement.isNull())
            {
                ++result;
                subElement = subElement.nextSiblingElement(subitemLabel);
            }
            element = element.nextSiblingElement(itemLabel);
        }
    }

    return result;
}

void DMScreenTabWidget::readSimpleSection(QDomElement& root, QString sectionName, QString itemLabel, QString subitemLabel, QLayout& layout1, QString name1, QLayout& layout2, QString name2)
{
    QDomElement section = root.firstChildElement(sectionName);
    if(section.isNull())
    {
        qDebug() << "[DMScreen] Unable to find the " << sectionName << " information element in the equipment data file.";
    }
    else
    {
        QDomElement element = section.firstChildElement(itemLabel);
        readEquipmentSection(element, 9999, itemLabel, subitemLabel, layout1, name1, layout2, name2);
    }
}

void DMScreenTabWidget::readSimpleSection(QDomElement& root, QString sectionName, QString itemLabel, QString subitemLabel, QLayout& layout1, QString name1, QLayout& layout2, QString name2, QLayout& layout3, QString name3)
{
    QDomElement section = root.firstChildElement(sectionName);
    if(section.isNull())
    {
        qDebug() << "[DMScreen] Unable to find the " << sectionName << " information element in the equipment data file.";
    }
    else
    {
        QDomElement element = section.firstChildElement(itemLabel);
        readEquipmentSection(element, 9999, itemLabel, subitemLabel, layout1, name1, layout2, name2, layout3, name3);
    }
}

void DMScreenTabWidget::readSimpleSection(QDomElement& root, QString sectionName, QString itemLabel, QString subitemLabel, QLayout& layout1, QString name1, QLayout& layout2, QString name2, QLayout& layout3, QString name3, QLayout& layout4, QString name4)
{
    QDomElement section = root.firstChildElement(sectionName);
    if(section.isNull())
    {
        qDebug() << "[DMScreen] Unable to find the " << sectionName << " information element in the equipment data file.";
    }
    else
    {
        QDomElement element = section.firstChildElement(itemLabel);
        readEquipmentSection(element, 9999, itemLabel, subitemLabel, layout1, name1, layout2, name2, layout3, name3, layout4, name4);
    }
}

void DMScreenTabWidget::readEquipmentSection(QDomElement& element, int itemCount, QString itemLabel, QString subitemLabel, QLayout& layout1, QString name1, QLayout& layout2, QString name2)
{
    if(element.isNull())
        return;

    while((!element.isNull()) && (itemCount > 0))
    {
        QDomElement subElement = element.firstChildElement(subitemLabel);

        layout1.addWidget(createLabel(element.attribute(name1), false, !subElement.isNull()));
        layout2.addWidget(createLabel(element.attribute(name2), true));

        while(!subElement.isNull())
        {
            layout1.addWidget(createLabel(subElement.attribute(name1), false, false, false, true));
            layout2.addWidget(createLabel(subElement.attribute(name2), true));
            --itemCount;
            subElement = subElement.nextSiblingElement(subitemLabel);
        }

        --itemCount;
        element = element.nextSiblingElement(itemLabel);
    }
}

void DMScreenTabWidget::readEquipmentSection(QDomElement& element, int itemCount, QString itemLabel, QString subitemLabel, QLayout& layout1, QString name1, QLayout& layout2, QString name2, QLayout& layout3, QString name3)
{
    if(element.isNull())
        return;

    while((!element.isNull()) && (itemCount > 0))
    {
        QDomElement subElement = element.firstChildElement(subitemLabel);

        layout1.addWidget(createLabel(element.attribute(name1), false, !subElement.isNull()));
        layout2.addWidget(createLabel(element.attribute(name2), true));
        layout3.addWidget(createLabel(element.attribute(name3), true));

        while(!subElement.isNull())
        {
            layout1.addWidget(createLabel(subElement.attribute(name1), false, false, false, true));
            layout2.addWidget(createLabel(subElement.attribute(name2), true));
            layout3.addWidget(createLabel(subElement.attribute(name3), true));
            --itemCount;
            subElement = subElement.nextSiblingElement(subitemLabel);
        }

        --itemCount;
        element = element.nextSiblingElement(itemLabel);
    }
}

void DMScreenTabWidget::readEquipmentSection(QDomElement& element, int itemCount, QString itemLabel, QString subitemLabel, QLayout& layout1, QString name1, QLayout& layout2, QString name2, QLayout& layout3, QString name3, QLayout& layout4, QString name4)
{
    if(element.isNull())
        return;

    while((!element.isNull()) && (itemCount > 0))
    {
        QDomElement subElement = element.firstChildElement(subitemLabel);

        layout1.addWidget(createLabel(element.attribute(name1), false, !subElement.isNull()));
        layout2.addWidget(createLabel(element.attribute(name2), true));
        layout3.addWidget(createLabel(element.attribute(name3), true));
        layout4.addWidget(createLabel(element.attribute(name4), true));

        while(!subElement.isNull())
        {
            layout1.addWidget(createLabel(subElement.attribute(name1), false, false, false, true));
            layout2.addWidget(createLabel(subElement.attribute(name2), true));
            layout3.addWidget(createLabel(subElement.attribute(name3), true));
            layout4.addWidget(createLabel(subElement.attribute(name4), true));
            --itemCount;
            subElement = subElement.nextSiblingElement(subitemLabel);
        }

        --itemCount;
        element = element.nextSiblingElement(itemLabel);
    }
}

QLabel* DMScreenTabWidget::createLabel(const QString& label, bool centered, bool italics, bool title, bool indented)
{
    QLabel* result;

    result = new QLabel(indented ? QString("     ") + label : label);

    if(centered)
        result->setAlignment(Qt::AlignHCenter);

    if(italics)
    {
        QFont italicFont = result->font();
        italicFont.setItalic(true);
        result->setFont(italicFont);
    }

    if(title)
    {
        QFont titleFont = result->font();
        titleFont.setBold(true);
        titleFont.setUnderline(true);
        result->setFont(titleFont);
    }

    return result;
}
