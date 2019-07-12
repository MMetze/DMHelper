#ifndef BATTLEDIALOGGRAPHICSVIEW_H
#define BATTLEDIALOGGRAPHICSVIEW_H

#include <QGraphicsView>

class BattleDialogGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit BattleDialogGraphicsView(QWidget *parent = nullptr);
    virtual ~BattleDialogGraphicsView();

};

#endif // BATTLEDIALOGGRAPHICSVIEW_H
