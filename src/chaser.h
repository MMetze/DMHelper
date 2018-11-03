#ifndef CHASER_H
#define CHASER_H

#include "complicationdata.h"
#include <QVariant>

class Combatant;
class QGraphicsTextItem;
class QGraphicsPixmapItem;
class Monster;
class ChaserWidget;

class Chaser
{
public:
    Chaser(int position = 0);
    explicit Chaser(const Chaser &other);  // copy constructor
    virtual ~Chaser();

    // Getters
    virtual int getPosition() const;
    virtual void setPosition(int position);

    virtual QString getName() const = 0;
    virtual int getSpeed() const = 0;
    virtual int getInitiative() const = 0;
    virtual int getX() const = 0;
    virtual int getWidth() const = 0;

    virtual void update();

protected:
    int _position;
};

class ChaserCombatant : public Chaser
{
public:
    ChaserCombatant(QGraphicsTextItem* textItem = 0, QGraphicsPixmapItem* pixmapItem = 0, Combatant* combatant = 0, int position = 0, int pursuer = true);
    explicit ChaserCombatant(const ChaserCombatant &other);  // copy constructor
    virtual ~ChaserCombatant();

    virtual void setPosition(int position);
    virtual QString getName() const;
    virtual int getSpeed() const;
    virtual void setSpeed(int speed);
    virtual int getInitiative() const;
    virtual void setInitiative(int initiative);
    virtual int getX() const;
    virtual int getWidth() const;

    virtual int getDashCount() const;
    virtual void useDash();
    virtual void setDashCount(int dashCount);

    virtual int getExhaustion() const;
    virtual void addExhaustion();
    virtual void setExhaustion(int exhaustion);

    virtual int getDeficit() const;
    virtual void reduceDeficit(int delta);
    virtual void setDeficit(int deficit);

    virtual bool isPursuer() const;
    virtual void setPursuer(bool pursuer);

    QGraphicsTextItem* getTextItem() const;
    void setTextItem(QGraphicsTextItem* textItem);
    Combatant* getCombatant() const;
    void setCombatant(Combatant* combatant);
    ChaserWidget* getWidget() const;
    void setWidget(ChaserWidget* widget);

    virtual void update();

protected:
    void refreshWidget();

    int _dashCount;
    int _speed;
    int _exhaustion;
    int _deficit;
    bool _pursuer;
    QGraphicsTextItem* _textItem;
    QGraphicsPixmapItem* _pixmapItem;
    Combatant* _combatant;
    ChaserWidget* _widget;
};

Q_DECLARE_METATYPE(ChaserCombatant*)

class ChaserLocalCombatant: public ChaserCombatant
{
public:
    ChaserLocalCombatant(QGraphicsTextItem* textItem, Combatant* combatant, int position = 0, int pursuer = true);
    virtual ~ChaserLocalCombatant();
};

class ChaserComplication : public Chaser
{
public:
    ChaserComplication();
    ChaserComplication(QGraphicsTextItem* textItem, const ComplicationData& data, bool active, int position);
    explicit ChaserComplication(const ChaserComplication &other);  // copy constructor
    virtual ~ChaserComplication();

    virtual void setPosition(int position);
    virtual QString getName() const;
    virtual int getSpeed() const;
    virtual int getInitiative() const;
    virtual int getX() const;
    virtual int getWidth() const;

    QGraphicsTextItem* getTextItem() const;
    void setTextItem(QGraphicsTextItem* textItem);
    const ComplicationData& getData() const;
    void setComplicationData(const ComplicationData& data);
    bool isActive() const;
    void setActive(bool active);

protected:
    QGraphicsTextItem* _textItem;
    ComplicationData _data;
    bool _active;
};

Q_DECLARE_METATYPE(ChaserComplication*)

#endif // CHASER_H
