#include "mapmanagerdialog.h"
#include "ui_mapmanagerdialog.h"
#include "optionscontainer.h"
#include "dmhcache.h"
#include <QFileDialog>
#include <QDir>
#include <QTreeWidgetItem>
#include <QImageReader>
#include <QStandardPaths>
#include <QTimer>
#include <QScrollArea>

const int MAPMANAGERDIALOG_CACHE_IMAGE_SIZE = 256;

/*
 * Next Steps
 * store the directory list in the cache and read it at startup
 * create a mapping of image md5s to tags and read/store that as well
 * Enable tagging of each image or groups of images
 * Enable filtering based on tag, type, title, etc
 */

MapManagerDialog::MapManagerDialog(OptionsContainer& options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapManagerDialog),
    _options(options),
    _searchList()
{
    ui->setupUi(this);

    DMHCache().ensureCacheExists(QString("maps"));

    connect(ui->btnBrowse, &QPushButton::clicked, this, &MapManagerDialog::browsePath);
    connect(ui->btnRefresh, &QPushButton::clicked, this, &MapManagerDialog::findMaps);
    connect(ui->treeMaps, &QTreeWidget::currentItemChanged, this, &MapManagerDialog::selectItem);
    connect(ui->treeMaps, &QTreeWidget::itemDoubleClicked, this, &MapManagerDialog::openPreviewDialog);

    ui->edtMapPath->setText(_options.getLastMapDirectory());
}

MapManagerDialog::~MapManagerDialog()
{
    _searchList.clear();
    delete ui;
}

void MapManagerDialog::selectItem(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);
    if(current)
    {
        QString itemPath = current->data(0, Qt::UserRole).toString();
        if(!itemPath.isEmpty())
        {
            QPixmap itemPixmap;
            QString cacheFile = DMHCache().getCacheFilePath(itemPath, QString("png"), QString("maps"));
            if((!cacheFile.isEmpty()) && (QFile::exists(cacheFile)))
                itemPixmap.load(cacheFile);

            if(itemPixmap.isNull())
            {
                QImageReader reader(itemPath);
                QSize imageSize = reader.size();
                imageSize.scale(QSize(MAPMANAGERDIALOG_CACHE_IMAGE_SIZE, MAPMANAGERDIALOG_CACHE_IMAGE_SIZE), Qt::KeepAspectRatio);
                reader.setScaledSize(imageSize);
                QImage image = reader.read();
                if(!image.isNull())
                {
                    itemPixmap = QPixmap::fromImage(image);
                    image.save(cacheFile);
                }
            }

            if(!itemPixmap.isNull())
            {
                ui->lblPreview->setText(QString());
                ui->lblPreview->setPixmap(itemPixmap.scaled(ui->lblPreview->contentsRect().size(), Qt::KeepAspectRatio));
                return;
            }
        }
    }

    ui->lblPreview->setText(tr("No Preview Available"));
}

void MapManagerDialog::openPreviewDialog(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if(!item)
        return;

    QString itemPath = item->data(0, Qt::UserRole).toString();
    if(itemPath.isEmpty())
        return;

    QPixmap itemPixmap(itemPath);
    if(itemPixmap.isNull())
        return;

    QLabel* imageLabel = new QLabel;
    QScrollArea* scrollArea = new QScrollArea;
    QDialog* imageDialog = new QDialog(this);

    imageLabel->setScaledContents(false);
    imageLabel->setPixmap(itemPixmap);
    scrollArea->setWidgetResizable(false);
    scrollArea->setWidget(imageLabel);
    imageDialog->setWindowTitle(tr("Map Preview - %1").arg(itemPath));
    imageDialog->resize(qMin(itemPixmap.width() + 20, width()), qMin(itemPixmap.height() + 20, height()));
    imageDialog->setLayout(new QVBoxLayout);
    imageDialog->layout()->addWidget(scrollArea);
    imageDialog->setAttribute(Qt::WA_DeleteOnClose);
    imageDialog->show();
}

void MapManagerDialog::browsePath()
{
    QString mapPath = QFileDialog::getExistingDirectory(this, tr("Select Map Directory"), ui->edtMapPath->text());
    if(mapPath.isEmpty())
        return;

    ui->edtMapPath->setText(mapPath);
}

void MapManagerDialog::findMaps()
{
    _options.setLastMapDirectory(ui->edtMapPath->text());

    QDir mapDir(ui->edtMapPath->text());
    if(!mapDir.exists())
        return;

    //scanDirectory(ui->treeMaps->invisibleRootItem(), ui->edtMapPath->text());
    _searchList.clear();
    _searchList.append(QPair<QTreeWidgetItem*, const QString&>(ui->treeMaps->invisibleRootItem(), ui->edtMapPath->text()));
    QTimer::singleShot(0, this, &MapManagerDialog::scanNextDirectory);

    /*
    QFileInfoList rootEntries = mapDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name | QDir::DirsFirst);
    if(rootEntries.isEmpty())
        return;

    for(const QFileInfo &entry : rootEntries)
    {
        QTreeWidgetItem* rootItem = new QTreeWidgetItem(static_cast<QTreeWidgetItem *>(nullptr), QStringList(entry.fileName()));
        if(entry.isDir())
            scanDirectory(rootItem, entry.absoluteFilePath());
        ui->treeMaps->addTopLevelItem(rootItem);
    }
*/
}

void MapManagerDialog::scanNextDirectory()
{
    if(_searchList.isEmpty())
        return;

    QPair<QTreeWidgetItem*, QString> currentPair = _searchList.takeFirst();
    scanDirectory(currentPair.first, currentPair.second);

    if(!_searchList.isEmpty())
        QTimer::singleShot(0, this, &MapManagerDialog::scanNextDirectory);
}

void MapManagerDialog::scanDirectory(QTreeWidgetItem* parent, const QString& absolutePath)
{
    if((!parent) || (absolutePath.isEmpty()))
        return;

    QDir mapDir(absolutePath);
    if(!mapDir.exists())
        return;

    QFileInfoList dirEntries = mapDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name | QDir::DirsFirst);
    if(dirEntries.isEmpty())
        return;

    for(const QFileInfo &entry : dirEntries)
    {
        QTreeWidgetItem* newItem = new QTreeWidgetItem(static_cast<QTreeWidgetItem *>(nullptr), QStringList(entry.fileName()));
        if(entry.isDir())
        {
            //scanDirectory(newItem, entry.absoluteFilePath());
            _searchList.append(QPair<QTreeWidgetItem*, QString>(newItem, entry.absoluteFilePath()));
        }
        else if(entry.isFile())
        {
            //if(QImageReader(entry.absoluteFilePath()).canRead())
                newItem->setData(0, Qt::UserRole, entry.absoluteFilePath());
        }
        parent->addChild(newItem);
    }    
}
