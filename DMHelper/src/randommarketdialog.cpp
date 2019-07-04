#include "randommarketdialog.h"
#include "ui_randommarketdialog.h"
#include <QDomDocument>
#include <QFile>
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

    QString shopFileName("shops.xml");

    QDomDocument doc("DMHelperDataXML");
    QFile file(shopFileName);
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
    ui->textEdit->clear();
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
