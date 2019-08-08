#include "randommarketdialog.h"
#include "ui_randommarketdialog.h"
#include "equipmentserver.h"
#include <QDomDocument>
#include <QFile>
#include <QRandomGenerator>
#include <QTextCharFormat>
#include <QScrollBar>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

RandomMarketDialog::RandomMarketDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RandomMarketDialog),
    _locations()
{
    ui->setupUi(this);

    connect(ui->btnRandomize, SIGNAL(clicked()), this, SLOT(randomizeMarket()));
    connect(ui->cmbLocation, SIGNAL(currentIndexChanged(int)), this, SLOT(locationSelected(int)));

    loadMarkets();

    randomizeMarket();
}

RandomMarketDialog::~RandomMarketDialog()
{
    delete ui;
}

void RandomMarketDialog::loadMarkets()
{
    qDebug() << "[RandomMarketDialog] Reading markets...";

#ifdef Q_OS_MAC
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    fileDirPath.cdUp();
    fileDirPath.cdUp();
    fileDirPath.cdUp();
    QString shopFileName = fileDirPath.path() + QString("/shops.xml");
#else
    QString shopFileName("shops.xml");
#endif

    QDomDocument doc("DMHelperDataXML");
    QFile file(shopFileName);
    qDebug() << "[RandomMarketDialog] Market file: " << QFileInfo(file).filePath();
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[RandomMarketDialog] Unable to read market file: " << shopFileName;
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
        qDebug() << "[RandomMarketDialog] Unable to parse the market data file.";
        qDebug() << errMsg << errRow << errColumn;
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "root"))
    {
        qDebug() << "[RandomMarketDialog] Unable to find the root element in the market data file.";
        return;
    }

    QDomElement locationSection = root.firstChildElement(QString("locations"));
    if(locationSection.isNull())
    {
        qDebug() << "[RandomMarketDialog] Unable to find the locations element in the market data file.";
    }
    else
    {
        ui->cmbLocation->clear();

        QDomElement locationElement = locationSection.firstChildElement(QString("location"));
        while(!locationElement.isNull())
        {
            Location newLocation;
            newLocation._name = locationElement.attribute(QString("name"));
            newLocation._probability = locationElement.attribute(QString("probability")).toDouble();

            QDomElement shopElement = locationElement.firstChildElement(QString("shop"));
            while(!shopElement.isNull())
            {
                Shop newShop;
                newShop._name = shopElement.attribute(QString("name"));
                newShop._mundane = shopElement.attribute(QString("mundane")).toDouble();
                newShop._goods = shopElement.attribute(QString("goods")).toDouble();
                newShop._magic_armor = shopElement.attribute(QString("magic_armor")).toDouble();
                newShop._magic_potion = shopElement.attribute(QString("magic_potion")).toDouble();
                newShop._magic_ring = shopElement.attribute(QString("magic_ring")).toDouble();
                newShop._magic_rod = shopElement.attribute(QString("magic_rod")).toDouble();
                newShop._magic_scroll = shopElement.attribute(QString("magic_scroll")).toDouble();
                newShop._magic_staff = shopElement.attribute(QString("magic_staff")).toDouble();
                newShop._magic_wand = shopElement.attribute(QString("magic_wand")).toDouble();
                newShop._magic_weapon = shopElement.attribute(QString("magic_weapon")).toDouble();
                newShop._magic_wondrousitem = shopElement.attribute(QString("magic_wondrousitem")).toDouble();
                newShop._vehicles = shopElement.attribute(QString("vehicles")).toDouble();
                newShop._weapon = shopElement.attribute(QString("weapon")).toDouble();
                newShop._armor = shopElement.attribute(QString("armor")).toDouble();

                newLocation._shops.append(newShop);

                shopElement = shopElement.nextSiblingElement(QString("shop"));
            }

            _locations.append(newLocation);
            ui->cmbLocation->addItem(newLocation._name);

            locationElement = locationElement.nextSiblingElement(QString("location"));
        }
    }

    if(ui->cmbLocation->count() > 0)
        ui->cmbLocation->setCurrentIndex(0);

    qDebug() << "[RandomMarketDialog] Completed reading equipment.";

}

void RandomMarketDialog::randomizeMarket()
{
    if(!EquipmentServer::Instance())
        return;

    if((ui->cmbLocation->currentIndex() < 0) || (ui->cmbLocation->currentIndex() >= _locations.count()) ||
       (ui->cmbShop->currentIndex() < 0) || (ui->cmbShop->currentIndex() >= _locations.at(ui->cmbLocation->currentIndex())._shops.count()))
        return;

    ui->textEdit->clear();

    Location location = _locations.at(ui->cmbLocation->currentIndex());
    Shop shop = location._shops.at(ui->cmbShop->currentIndex());



    addTitle(QString("Weapons"));
    QList<EquipmentServer::WeaponItem> weaponItems = EquipmentServer::Instance()->getWeaponItems();
    for(EquipmentServer::WeaponItem weaponItem : weaponItems)
    {
        qreal randomValue = QRandomGenerator::global()->generateDouble();
        if(randomValue < getProbability(location, shop, EquipmentServer::Category_Weapon, weaponItem._probability))
        {
            QString itemText(weaponItem._name + " (" + weaponItem._weight + ", " + weaponItem._cost + ")");
            ui->textEdit->append(itemText);
        }
    }
    ui->textEdit->append("");



    addTitle(QString("Armor"));
    QList<EquipmentServer::ArmorItem> armorItems = EquipmentServer::Instance()->getArmorItems();
    for(EquipmentServer::ArmorItem armorItem : armorItems)
    {
        qreal randomValue = QRandomGenerator::global()->generateDouble();
        if(randomValue < getProbability(location, shop, EquipmentServer::Category_Armor, armorItem._probability))
        {
            QString itemText(armorItem._name + " (" + armorItem._weight + ", " + armorItem._cost + ")");
            ui->textEdit->append(itemText);
        }
    }
    ui->textEdit->append("");



    addTitle(QString("Adventuring Gear"));
    QList<EquipmentServer::GearItem> gearItems = EquipmentServer::Instance()->getGearItems();
    for(EquipmentServer::GearItem gearItem : gearItems)
    {
        if(gearItem._subitems.count() == 0)
        {
            qreal randomValue = QRandomGenerator::global()->generateDouble();
            if(randomValue < getProbability(location, shop, EquipmentServer::Category_Mundane, gearItem._probability))
            {
                QString itemText(gearItem._name + " (" + gearItem._weight + ", " + gearItem._cost + ")");
                ui->textEdit->append(itemText);
            }
        }
        else
        {
            for(EquipmentServer::GearSubItem subItem : gearItem._subitems)
            {
                qreal randomValue = QRandomGenerator::global()->generateDouble();
                if(randomValue < getProbability(location, shop, EquipmentServer::Category_Mundane, gearItem._probability != EquipmentServer::Probability_None ? gearItem._probability : subItem._probability ))
                {
                    QString itemText(gearItem._name + ", " + subItem._name + " (" + subItem._weight + ", " + subItem._cost + ")");
                    ui->textEdit->append(itemText);
                }
            }
        }
    }
    ui->textEdit->append("");



    addTitle(QString("Tools"));
    QList<EquipmentServer::ToolItem> toolItems = EquipmentServer::Instance()->getToolItems();
    for(EquipmentServer::ToolItem toolItem : toolItems)
    {
        if(toolItem._subitems.count() == 0)
        {
            qreal randomValue = QRandomGenerator::global()->generateDouble();
            if(randomValue < getProbability(location, shop, EquipmentServer::Category_Mundane, toolItem._probability))
            {
                QString itemText(toolItem._name + " (" + toolItem._weight + ", " + toolItem._cost + ")");
                ui->textEdit->append(itemText);
            }
        }
        else
        {
            for(EquipmentServer::ToolSubItem subitem : toolItem._subitems)
            {
                qreal randomValue = QRandomGenerator::global()->generateDouble();
                if(randomValue < getProbability(location, shop, EquipmentServer::Category_Mundane, toolItem._probability != EquipmentServer::Probability_None ? toolItem._probability : subitem._probability ))
                {
                    QString itemText(toolItem._name + ", " + subitem._name + " (" + subitem._weight + ", " + subitem._cost + ")");
                    ui->textEdit->append(itemText);
                }
            }
        }
    }
    ui->textEdit->append("");



    addTitle(QString("Trading Goods"));
    QList<EquipmentServer::GoodsItem> goodsItems = EquipmentServer::Instance()->getGoodsItems();
    for(EquipmentServer::GoodsItem goodsItem : goodsItems)
    {
        qreal randomValue = QRandomGenerator::global()->generateDouble();
        if(randomValue < getProbability(location, shop, EquipmentServer::Category_Goods, goodsItem._probability))
        {
            QString itemText(goodsItem._name + " (" + goodsItem._cost + ")");
            ui->textEdit->append(itemText);
        }
    }
    ui->textEdit->append("");




    addTitle(QString("Magic Items"));
    QList<EquipmentServer::MagicItem> magicItems = EquipmentServer::Instance()->getMagicItems();
    for(EquipmentServer::MagicItem magicItem : magicItems)
    {
        if(magicItem._subitems.count() == 0)
        {
            qreal randomValue = QRandomGenerator::global()->generateDouble();
            if(randomValue < getProbability(location, shop, magicItem._category, magicItem._probability))
            {
                QString itemText(magicItem._name + " (" + magicItem._categoryText + ")");
                if(magicItem._attunement)
                    itemText.append(QString(", requires attunement"));
                ui->textEdit->append(itemText);
            }
        }
        else
        {
            for(EquipmentServer::MagicSubItem subitem : magicItem._subitems)
            {
                qreal randomValue = QRandomGenerator::global()->generateDouble();
                if(randomValue < getProbability(location, shop, magicItem._category, magicItem._probability != EquipmentServer::Probability_None ? magicItem._probability : subitem._probability ))
                {
                    QString itemText(magicItem._name + ", " + subitem._name + " (" + magicItem._categoryText + ")");
                    if(magicItem._attunement)
                        itemText.append(QString(", requires attunement"));
                    ui->textEdit->append(itemText);
                }
            }
        }
    }
    ui->textEdit->append("");

    ui->textEdit->moveCursor(QTextCursor::Start);
    ui->textEdit->ensureCursorVisible();

}

void RandomMarketDialog::locationSelected(int index)
{
    ui->cmbShop->clear();

    if((index < 0) || (index >= ui->cmbLocation->count()))
        return;

    for(int i = 0; i < _locations.at(index)._shops.count(); ++i)
    {
        ui->cmbShop->addItem(_locations.at(index)._shops.at(i)._name);
    }

    if(ui->cmbShop->count() > 0)
        ui->cmbShop->setCurrentIndex(0);
}

qreal RandomMarketDialog::getProbability(const Location& location, const Shop& shop, EquipmentServer::ItemCategory category, EquipmentServer::ItemProbability itemProbability)
{
    qreal result = location._probability;

    switch(category)
    {
        case EquipmentServer::Category_Mundane:
            result *= shop._mundane; break;
        case EquipmentServer::Category_Goods:
            result *= shop._goods; break;
        case EquipmentServer::Category_Magic_Armor:
            result *= shop._magic_armor; break;
        case EquipmentServer::Category_Magic_Potion:
            result *= shop._magic_potion; break;
        case EquipmentServer::Category_Magic_Ring:
            result *= shop._magic_ring; break;
        case EquipmentServer::Category_Magic_Rod:
            result *= shop._magic_rod; break;
        case EquipmentServer::Category_Magic_Scroll:
            result *= shop._magic_scroll; break;
        case EquipmentServer::Category_Magic_Staff:
            result *= shop._magic_staff; break;
        case EquipmentServer::Category_Magic_Wand:
            result *= shop._magic_wand; break;
        case EquipmentServer::Category_Magic_Weapon:
            result *= shop._magic_weapon; break;
        case EquipmentServer::Category_Magic_Wondrousitem:
            result *= shop._magic_wondrousitem; break;
        case EquipmentServer::Category_Vehicles:
            result *= shop._vehicles; break;
        case EquipmentServer::Category_Weapon:
            result *= shop._weapon; break;
        case EquipmentServer::Category_Armor:
            result *= shop._armor; break;
    }

    switch(itemProbability)
    {
        case EquipmentServer::Probability_Always:
            result *= 1.0; break;
        case EquipmentServer::Probability_Common:
            result *= 0.9; break;
        case EquipmentServer::Probability_Uncommon:
            result *= 0.6; break;
        case EquipmentServer::Probability_Rare:
            result *= 0.4; break;
        case EquipmentServer::Probability_Very_Rare:
            result *= 0.2; break;
        case EquipmentServer::Probability_Legendary:
            result *= 0.05; break;
        case EquipmentServer::Probability_None:
            result *= 0.0; break;
    }

    return result;
}

void RandomMarketDialog::addTitle(const QString& titleName)
{
    QTextCharFormat format = ui->textEdit->currentCharFormat();
    format.setFontWeight(QFont::Bold);
    ui->textEdit->setCurrentCharFormat(format);
    ui->textEdit->append(titleName);
    format.setFontWeight(QFont::Normal);
    ui->textEdit->setCurrentCharFormat(format);
}
