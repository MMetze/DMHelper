#include "bestiarytemplatedialog.h"
#include "ui_bestiarytemplatedialog.h"

BestiaryTemplateDialog::BestiaryTemplateDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BestiaryTemplateDialog)
{
    ui->setupUi(this);
}

BestiaryTemplateDialog::~BestiaryTemplateDialog()
{
    delete ui;
}

void BestiaryTemplateDialog::loadMonsterUITemplate(const QString& templateFile)
{
    do this next
}
