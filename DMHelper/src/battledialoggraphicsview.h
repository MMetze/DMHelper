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
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

};

#endif // BATTLEDIALOGGRAPHICSVIEW_H
