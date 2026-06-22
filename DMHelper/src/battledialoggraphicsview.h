#ifndef BATTLEDIALOGGRAPHICSVIEW_H
#define BATTLEDIALOGGRAPHICSVIEW_H

#include <QGraphicsView>

class BattleDialogGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit BattleDialogGraphicsView(QWidget *parent = nullptr);
    virtual ~BattleDialogGraphicsView();

protected:
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

};

#endif // BATTLEDIALOGGRAPHICSVIEW_H
