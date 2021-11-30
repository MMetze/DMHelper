#include "campaignnotesdialog.h"
#include "ui_campaignnotesdialog.h"

CampaignNotesDialog::CampaignNotesDialog(QStringList notes, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CampaignNotesDialog)
{
    ui->setupUi(this);

    ui->plainTextEdit->setPlainText(notes.join("\n"));
}

CampaignNotesDialog::~CampaignNotesDialog()
{
    delete ui;
}

QString CampaignNotesDialog::getNotes() const
{
    return ui->plainTextEdit->toPlainText();
}
