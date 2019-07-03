#include "randommarketdialog.h"
#include "ui_randommarketdialog.h"

RandomMarketDialog::RandomMarketDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RandomMarketDialog),
    _locations()
{
    ui->setupUi(this);

    connect(ui->btnRandomize, SIGNAL(clicked()), this, SLOT(randomizeMarket()));

    loadMarkets();

    randomizeMarket();
}

RandomMarketDialog::~RandomMarketDialog()
{
    delete ui;
}

void RandomMarketDialog::loadMarkets()
{

}

void RandomMarketDialog::randomizeMarket()
{
    ui->textEdit->clear();
}
