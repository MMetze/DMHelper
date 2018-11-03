#include "chaseanimationdialog.h"
#include <QResizeEvent>
#include <QGraphicsScene>
#include "ui_chaseanimationdialog.h"

ChaseAnimationDialog::ChaseAnimationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChaseAnimationDialog)
{
    ui->setupUi(this);
}

ChaseAnimationDialog::~ChaseAnimationDialog()
{
    delete ui;
}

QGraphicsView* ChaseAnimationDialog::getGraphicsView() const
{
    return ui->graphicsView;
}

void ChaseAnimationDialog::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
}
