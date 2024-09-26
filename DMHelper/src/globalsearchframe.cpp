#include "globalsearchframe.h"
#include "ui_globalsearchframe.h"

GlobalSearchFrame::GlobalSearchFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::GlobalSearchFrame)
{
    ui->setupUi(this);
}

GlobalSearchFrame::~GlobalSearchFrame()
{
    delete ui;
}
