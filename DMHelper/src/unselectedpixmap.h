#ifndef UNSELECTEDPIXMAP_H
#define UNSELECTEDPIXMAP_H

#include <QGraphicsPixmapItem>

class BattleDialogModelObject;

const int BATTLE_CONTENT_CHILD_INDEX = 0;
const int BATTLE_CONTENT_CHILD_ID = 1;

enum BattleDialogItemChild
{
    BattleDialogItemChild_Base = 0,
    BattleDialogItemChild_Selection,
    BattleDialogItemChild_AreaEffect,
    BattleDialogItemChild_Area,
    BattleDialogItemChild_PersonalEffect
};

class UnselectedPixmap : public QGraphicsPixmapItem
{
public:
    explicit UnselectedPixmap(BattleDialogModelObject* object = nullptr, QGraphicsItem *parent = nullptr);
    UnselectedPixmap(const QPixmap &pixmap, BattleDialogModelObject* object, QGraphicsItem *parent = nullptr);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

    void setDraw(bool draw);

    void setHover(bool hover);
    BattleDialogModelObject* getObject();

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void selectionChanged();

private:
    BattleDialogModelObject* _object;
    bool _draw;
};

#endif // UNSELECTEDPIXMAP_H
