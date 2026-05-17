#include "battledialoggraphicsview.h"
#include <QKeyEvent>

BattleDialogGraphicsView::BattleDialogGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
}

BattleDialogGraphicsView::~BattleDialogGraphicsView()
{
}

void BattleDialogGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::AltModifier)
        event->ignore();
    else
        QGraphicsView::keyPressEvent(event);
}

