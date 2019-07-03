#ifndef RANDOMMARKETDIALOG_H
#define RANDOMMARKETDIALOG_H

#include <QDialog>

namespace Ui {
class RandomMarketDialog;
}

class RandomMarketDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RandomMarketDialog(QWidget *parent = nullptr);
    ~RandomMarketDialog();

private slots:
    void loadMarkets();
    void randomizeMarket();

private:
    Ui::RandomMarketDialog *ui;

    class Shop
    {
    public:
        Shop() :
            _name(),
            _mundane(1.0),
            _magic_armor(1.0),
            _magic_potion(1.0),
            _magic_ring(1.0),
            _magic_rod(1.0),
            _magic_scroll(1.0),
            _magic_staff(1.0),
            _magic_wand(1.0),
            _magic_weapon(1.0),
            _magic_wondrousitem(1.0),
            _vehicles(1.0),
            _weapon(1.0),
            _armor(1.0)
        {}

        QString _name;
        qreal _mundane;
        qreal _magic_armor;
        qreal _magic_potion;
        qreal _magic_ring;
        qreal _magic_rod;
        qreal _magic_scroll;
        qreal _magic_staff;
        qreal _magic_wand;
        qreal _magic_weapon;
        qreal _magic_wondrousitem;
        qreal _vehicles;
        qreal _weapon;
        qreal _armor;
    };

    class Location
    {
    public:
        Location() :
            _name(),
            _probability(1.0),
            _shops()
        {}

        QString _name;
        qreal _probability;
        QList<Shop> _shops;
    };

    QList<Location> _locations;

};

#endif // RANDOMMARKETDIALOG_H
