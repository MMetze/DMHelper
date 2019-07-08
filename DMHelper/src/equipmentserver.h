#ifndef EQUIPMENTSERVER_H
#define EQUIPMENTSERVER_H

#include <QString>
#include <QList>
#include <QDomElement>

class EquipmentServer;

class EquipmentServer
{
public:
    enum ItemProbability
    {
        Probability_Always = 0,
        Probability_Common,
        Probability_Uncommon,
        Probability_Rare,
        Probability_Very_Rare,
        Probability_Legendary,
        Probability_None
    };

    enum ItemCategory
    {
        Category_Mundane = 0,
        Category_Goods,
        Category_Magic_Armor,
        Category_Magic_Potion,
        Category_Magic_Ring,
        Category_Magic_Rod,
        Category_Magic_Scroll,
        Category_Magic_Staff,
        Category_Magic_Wand,
        Category_Magic_Weapon,
        Category_Magic_Wondrousitem,
        Category_Vehicles,
        Category_Weapon,
        Category_Armor
    };

    EquipmentServer();

    static EquipmentServer* Instance();
    static void Initialize();
    static void Shutdown();

    class MagicItem;
    class MagicSubItem;
    class GearItem;
    class GearSubItem;
    class WeaponItem;
    class ArmorItem;
    class GoodsItem;
    class ToolItem;
    class ToolSubItem;

    QList<MagicItem> getMagicItems() const;
    QList<GearItem> getGearItems() const;
    QList<WeaponItem> getWeaponItems() const;
    QList<ArmorItem> getArmorItems() const;
    QList<GoodsItem> getGoodsItems() const;
    QList<ToolItem> getToolItems() const;

private:
    static EquipmentServer* _instance;

    QList<MagicItem> _magicItems;
    QList<GearItem> _gearItems;
    QList<WeaponItem> _weaponItems;
    QList<ArmorItem> _armorItems;
    QList<GoodsItem> _goodsItems;
    QList<ToolItem> _toolItems;

    void readEquipment();
    ItemProbability probabilityFromString(QString probability);
    ItemCategory categoryFromString(QString category, bool isMagic);

    void readWeaponSubSection(QDomElement weaponSection, QString subSectionName);
    void readArmorSubSection(QDomElement armorSection, QString subSectionName);

public:

    class MagicItem
    {
    public:
        MagicItem() :
            _name(),
            _categoryText(),
            _category(),
            _probability(Probability_Common),
            _attunement(false),
            _subitems()
        {}

        QString _name;
        QString _categoryText;
        ItemCategory _category;
        ItemProbability _probability;
        bool _attunement;
        QList<MagicSubItem> _subitems;
    };

    class MagicSubItem
    {
    public:
        MagicSubItem() :
            _name(),
            _categoryText(),
            _probability(Probability_Common),
            _attunement(false)
        {}

        QString _name;
        QString _categoryText;
        ItemProbability _probability;
        bool _attunement;
    };

    class GearItem
    {
    public:
        GearItem() :
            _name(),
            _cost(),
            _weight(),
            _probability(Probability_Common),
            _subitems()
        {}

        QString _name;
        QString _cost;
        QString _weight;
        ItemProbability _probability;
        QList<GearSubItem> _subitems;
    };

    class GearSubItem
    {
    public:
        GearSubItem() :
            _name(),
            _cost(),
            _weight(),
            _probability(Probability_Common)
        {}

        QString _name;
        QString _cost;
        QString _weight;
        ItemProbability _probability;
    };

    class WeaponItem
    {
    public:
        WeaponItem() :
            _name(),
            _cost(),
            _weight(),
            _probability(Probability_Common)
        {}

        QString _name;
        QString _cost;
        QString _weight;
        ItemProbability _probability;
    };

    class ArmorItem
    {
    public:
        ArmorItem() :
            _name(),
            _cost(),
            _weight(),
            _probability(Probability_Common)
        {}

        QString _name;
        QString _cost;
        QString _weight;
        ItemProbability _probability;
    };

    class GoodsItem
    {
    public:
        GoodsItem() :
            _name(),
            _cost(),
            _probability(Probability_Common)
        {}

        QString _name;
        QString _cost;
        ItemProbability _probability;
    };

    class ToolItem
    {
    public:
        ToolItem() :
            _name(),
            _cost(),
            _weight(),
            _probability(Probability_Common),
            _subitems()
        {}

        QString _name;
        QString _cost;
        QString _weight;
        ItemProbability _probability;
        QList<ToolSubItem> _subitems;
    };

    class ToolSubItem
    {
    public:
        ToolSubItem() :
            _name(),
            _cost(),
            _weight(),
            _probability(Probability_Common)
        {}

        QString _name;
        QString _cost;
        QString _weight;
        ItemProbability _probability;
    };

};

#endif // EQUIPMENTSERVER_H
