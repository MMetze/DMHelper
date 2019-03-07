#ifndef MONSTERACTIONFRAME_H
#define MONSTERACTIONFRAME_H

#include <QFrame>
#include "monsteraction.h"

namespace Ui {
class MonsterActionFrame;
}


class MonsterActionFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MonsterActionFrame(const MonsterAction& action, QWidget *parent = nullptr);
    ~MonsterActionFrame();

    const MonsterAction& getAction() const;

signals:
    void deleteAction(const MonsterAction& action);

protected:
    // From QWidget
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

    void updateFields();

private:
    Ui::MonsterActionFrame *ui;

    MonsterAction _action;
};

#endif // MONSTERACTIONFRAME_H
