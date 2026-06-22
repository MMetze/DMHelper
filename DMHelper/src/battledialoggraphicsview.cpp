#include "battledialoggraphicsview.h"
#include <QKeyEvent>
#include <QPainter>
#include <QPixmap>

BattleDialogGraphicsView::BattleDialogGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
}

BattleDialogGraphicsView::~BattleDialogGraphicsView()
{
}

void BattleDialogGraphicsView::drawBackground(QPainter *painter, const QRectF &rect)
{
    static const QPixmap parchment(QString(":/img/data/parchment.jpg"));
    if(parchment.isNull())
    {
        QGraphicsView::drawBackground(painter, rect);
        return;
    }
    painter->drawTiledPixmap(rect, parchment);
}

void BattleDialogGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::AltModifier)
        event->ignore();
    else
        QGraphicsView::keyPressEvent(event);
}

