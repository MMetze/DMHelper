#include "mapmanagereditdirectoriesdialog.h"
#include "ui_mapmanagereditdirectoriesdialog.h"
#include <QFileDialog>
#include <QInputDialog>

MapManagerEditDirectoriesDialog::MapManagerEditDirectoriesDialog(QStringList directories, bool stringsOnly, const QString& dialogTitle, const QString& dataName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapManagerEditDirectoriesDialog),
    _dataName(dataName)
{
    ui->setupUi(this);
    ui->listWidget->addItems(directories);
    if(!dialogTitle.isEmpty())
        setWindowTitle(dialogTitle);

    if(stringsOnly)
        connect(ui->btnAdd, &QAbstractButton::clicked, this, &MapManagerEditDirectoriesDialog::addDirectoryString);
    else
        connect(ui->btnAdd, &QAbstractButton::clicked, this, &MapManagerEditDirectoriesDialog::addDirectory);

    connect(ui->btnRemove, &QAbstractButton::clicked, this, &MapManagerEditDirectoriesDialog::removeDirectory);
}

MapManagerEditDirectoriesDialog::~MapManagerEditDirectoriesDialog()
{
    delete ui;
}

QStringList MapManagerEditDirectoriesDialog::getDirectories()
{
    QStringList result;

    for(int i = 0; i < ui->listWidget->count(); ++i)
    {
        QListWidgetItem* item = ui->listWidget->item(i);
        if(item)
            result.append(item->text());
    }

    return result;
}

void MapManagerEditDirectoriesDialog::addDirectory()
{
    QString directory = QFileDialog::getExistingDirectory(this, QString("Select Directory"));
    if(!directory.isEmpty())
        ui->listWidget->addItem(directory);
}

void MapManagerEditDirectoriesDialog::addDirectoryString()
{
    QString directory = QInputDialog::getText(this, QString("Enter New Data"), _dataName + QString(": "));
    if(!directory.isEmpty())
        ui->listWidget->addItem(directory);
}

void MapManagerEditDirectoriesDialog::removeDirectory()
{
    QList<QListWidgetItem*> selectedItems = ui->listWidget->selectedItems();

    for(QListWidgetItem* item : selectedItems)
        delete item;
}
