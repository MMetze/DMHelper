#include "bestiaryexportdialog.h"
#include "ui_bestiaryexportdialog.h"
#include "bestiary.h"
#include "monsterclassv2.h"
#include <QFileDialog>
#include <QDomDocument>
#include <QDebug>

BestiaryExportDialog::BestiaryExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BestiaryExportDialog)
{
    ui->setupUi(this);

    connect(ui->btnSelectAll, SIGNAL(clicked()), this, SLOT(selectAll()));
    connect(ui->btnSelectNone, SIGNAL(clicked()), this, SLOT(selectNone()));
    connect(ui->btnExport, SIGNAL(clicked()), this, SLOT(exportSelected()));

    if(Bestiary::Instance())
    {
        QList<QString> monsters = Bestiary::Instance()->getMonsterList();
        for(QString monster : monsters)
        {
            QListWidgetItem* newItem = new QListWidgetItem(monster);
            newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            newItem->setCheckState(Qt::Checked);
            ui->listMonsters->addItem(newItem);
        }
    }
}

BestiaryExportDialog::~BestiaryExportDialog()
{
    delete ui;
}

void BestiaryExportDialog::selectAll()
{
    massSelect(true);
}

void BestiaryExportDialog::selectNone()
{
    massSelect(false);
}

void BestiaryExportDialog::massSelect(bool select)
{
    for(int i = 0; i < ui->listMonsters->count(); ++i)
    {
        if(ui->listMonsters->item(i))
            ui->listMonsters->item(i)->setCheckState(select ? Qt::Checked : Qt::Unchecked);
    }
}

void BestiaryExportDialog::exportSelected()
{
    if(!Bestiary::Instance())
        return;

    qDebug() << "[BestiaryExportDialog] Exporting selected monsters...";

    int checkedCount = 0;
    int i;
    for(i = 0; i < ui->listMonsters->count(); ++i)
    {
        if((ui->listMonsters->item(i)) && (ui->listMonsters->item(i)->checkState() == Qt::Checked))
            ++checkedCount;
    }

    if(checkedCount == 0)
    {
         qDebug() << "[BestiaryExportDialog] No monsters selected for export.";
         return;
    }

    QString bestiaryFileName = QFileDialog::getSaveFileName(this, QString("Save Bestiary"), QString(), QString("XML files (*.xml)"));
    if(bestiaryFileName.isEmpty())
    {
        qDebug() << "[BestiaryExportDialog] No valid file selected for.";
        return;
    }

    QDomDocument doc("DMHelperBestiaryXML");

    QDomElement root = doc.createElement("root");
    doc.appendChild(root);

    QFileInfo fileInfo(bestiaryFileName);
    QDir targetDirectory(fileInfo.absoluteDir());

    QDomElement bestiaryElement = doc.createElement("bestiary");
    bestiaryElement.setAttribute("majorversion", Bestiary::Instance()->getMajorVersion());
    bestiaryElement.setAttribute("minorversion", Bestiary::Instance()->getMinorVersion());

    int monsterCount = 0;
    for(i = 0; i < ui->listMonsters->count(); ++i)
    {
        if((ui->listMonsters->item(i)) && (ui->listMonsters->item(i)->checkState() == Qt::Checked))
        {
            MonsterClassv2* monsterClass = Bestiary::Instance()->getMonsterClass(ui->listMonsters->item(i)->text());
            if(monsterClass)
            {
                QDomElement monsterElement = doc.createElement("monster");
                monsterClass->outputXML(doc, monsterElement, targetDirectory, true);
                bestiaryElement.appendChild(monsterElement);
                ++monsterCount;
            }
        }
    }

    root.appendChild(bestiaryElement);
    qDebug() << "[BestiaryExportDialog] Exporting bestiary completed: " << monsterCount << " monsters written to XML";

    QString xmlString = doc.toString();
    QFile file(bestiaryFileName);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "[BestiaryExportDialog] Unable to open Bestiary export file for writing: " << bestiaryFileName;
        return;
    }

    QTextStream ts(&file);
    ts.setEncoding(QStringConverter::Utf8);
    ts << xmlString;

    file.close();

    qDebug() << "[BestiaryExportDialog] Bestiary export file writing complete: " << bestiaryFileName;

    close();
}
