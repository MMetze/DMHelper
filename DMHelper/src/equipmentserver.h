#ifndef EQUIPMENTSERVER_H
#define EQUIPMENTSERVER_H

#include <QString>
#include <QList>

class EquipmentServer;

class EquipmentServer
{
public:
    enum ItemProbability
    {
        Probability_Common = 0,
        Probability_Uncommon,
        Probability_Rare,
        Probability_Very_Rare,
        Probability_Legendary
    };

    EquipmentServer();

    static EquipmentServer* Instance();
    static void Initialize();
    static void Shutdown();

    class MagicItem;
    class MagicSubItem;

private:
    static EquipmentServer* _instance;

    QList<MagicItem> _magicItems;

    void readEquipment();
    ItemProbability probabilityFromString(QString probability);

public:

    class MagicItem
    {
    public:
        MagicItem() :
            _name(),
            _category(),
            _probability(Probability_Common),
            _attunement(),
            _subitems()
        {}

        QString _name;
        QString _category;
        ItemProbability _probability;
        QString _attunement;
        QList<MagicSubItem> _subitems;
    };

    class MagicSubItem
    {
    public:
        MagicSubItem() :
            _name(),
            _category(),
            _probability(Probability_Common),
            _attunement()
        {}

        QString _name;
        QString _category;
        ItemProbability _probability;
        QString _attunement;
    };

};

#endif // EQUIPMENTSERVER_H
