#include "monsteractionframe.h"
#include "ui_monsteractionframe.h"
#include "monsteractioneditdialog.h"
#include <QMouseEvent>

MonsterActionFrame::MonsterActionFrame(const MonsterAction& action, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MonsterActionFrame),
    _action(action)
{
    ui->setupUi(this);
    updateFields();
}

MonsterActionFrame::~MonsterActionFrame()
{
    delete ui;
}

const MonsterAction& MonsterActionFrame::getAction() const
{
    return _action;
}

void MonsterActionFrame::mouseDoubleClickEvent(QMouseEvent *event)
{
    MonsterActionEditDialog dlg(_action, true);
    if(dlg.exec() == QDialog::Accepted)
    {
        _action = dlg.getAction();
        updateFields();
        emit frameChanged();
    }
    else if(dlg.isDeleted())
    {
        emit deleteAction(_action);
        deleteLater();
    }
    event->accept();
}

void MonsterActionFrame::updateFields()
{
    ui->lblName->setText(_action.summaryString());
    ui->lblDescription->setText(_action.getDescription());
}
