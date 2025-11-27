#include "mapmanagerdialog.h"
#include "ui_mapmanagerdialog.h"
#include "optionscontainer.h"
#include "dmhcache.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <QDir>
#include <QImageReader>
#include <QStandardPaths>
#include <QTimer>
#include <QScrollArea>

const int MAPMANAGERDIALOG_CACHE_IMAGE_SIZE = 256;
const int MAPMANAGERDIALOG_PATH = Qt::UserRole + 1;

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
    _model(new QStandardItemModel()),
    _proxy(new QSortFilterProxyModel()),
    _options(options),
    _searchList()
{
    ui->setupUi(this);

    DMHCache().ensureCacheExists(QString("maps"));

    _proxy->setSourceModel(_model);
    ui->treeView->setModel(_proxy);

    connect(ui->btnBrowse, &QPushButton::clicked, this, &MapManagerDialog::browsePath);
    connect(ui->btnRefresh, &QPushButton::clicked, this, &MapManagerDialog::findMaps);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MapManagerDialog::selectItem);
    connect(ui->treeView, &QTreeView::doubleClicked, this, &MapManagerDialog::openPreviewDialog);

    ui->edtMapPath->setText(_options.getLastMapDirectory());
}

MapManagerDialog::~MapManagerDialog()
{
    _searchList.clear();
    delete ui;
}

void MapManagerDialog::selectItem(const QItemSelection &current, const QItemSelection &previous)
{
    Q_UNUSED(previous);
    QStandardItem* currentItem = _model->itemFromIndex(_proxy->mapToSource(current.indexes().first()));
    if(currentItem)
    {
        QString itemPath = currentItem->data(MAPMANAGERDIALOG_PATH).toString();
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

void MapManagerDialog::openPreviewDialog(const QModelIndex &current)
{
    QStandardItem* item = _model->itemFromIndex(_proxy->mapToSource(current));

    if(!item)
        return;

    QString itemPath = item->data(MAPMANAGERDIALOG_PATH).toString();
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
    _searchList.append(QPair<QStandardItem*, const QString&>(_model->invisibleRootItem(), ui->edtMapPath->text()));
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

    QPair<QStandardItem*, QString> currentPair = _searchList.takeFirst();
    scanDirectory(currentPair.first, currentPair.second);

    if(!_searchList.isEmpty())
        QTimer::singleShot(0, this, &MapManagerDialog::scanNextDirectory);
}

void MapManagerDialog::scanDirectory(QStandardItem* parent, const QString& absolutePath)
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
        QStandardItem* newItem = new QStandardItem(entry.fileName());
        newItem->setEditable(false);
        if(entry.isDir())
        {
            //scanDirectory(newItem, entry.absoluteFilePath());
            _searchList.append(QPair<QStandardItem*, QString>(newItem, entry.absoluteFilePath()));
        }
        else if(entry.isFile())
        {
            //if(QImageReader(entry.absoluteFilePath()).canRead())
                newItem->setData(entry.absoluteFilePath(), MAPMANAGERDIALOG_PATH);
        }
        parent->appendRow(newItem);
    }    
}
