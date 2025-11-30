#include "mapmanagerdialog.h"
#include "ui_mapmanagerdialog.h"
#include "optionscontainer.h"
#include "dmhcache.h"
#include "dmconstants.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <QDir>
#include <QImageReader>
#include <QStandardPaths>
#include <QTimer>
#include <QScrollArea>
#include <QDomDocument>
#include <QDomElement>

const int MAPMANAGERDIALOG_CACHE_IMAGE_SIZE = 256;
const int MAPMANAGERDIALOG_METADATA = Qt::UserRole + 1;

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
    _currentPath(),
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
}

MapManagerDialog::~MapManagerDialog()
{
    _searchList.clear();
    delete ui;
}

void MapManagerDialog::showEvent(QShowEvent *event)
{
    if(_currentPath.isEmpty())
        setCurrentPath(_options.getLastMapDirectory());

    QDialog::showEvent(event);
}

void MapManagerDialog::selectItem(const QItemSelection &current, const QItemSelection &previous)
{
    Q_UNUSED(previous);
    QStandardItem* currentItem = _model->itemFromIndex(_proxy->mapToSource(current.indexes().first()));
    if(currentItem)
    {
        QVariant variantData = currentItem->data(MAPMANAGERDIALOG_METADATA);
        if(variantData.isValid())
        {
            MapFileMetadata metaData = variantData.value<MapFileMetadata>();
            if(metaData._type != DMHelper::FileType_Directory)
            {
                QPixmap itemPixmap;
                QString cacheFile = DMHCache().getCacheFilePath(metaData._filePath, QString("png"), QString("maps"));
                if((!cacheFile.isEmpty()) && (QFile::exists(cacheFile)))
                    itemPixmap.load(cacheFile);

                if(itemPixmap.isNull())
                {
                    QImageReader reader(metaData._filePath);
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

                    if(metaData._type == DMHelper::FileType_Unknown)
                    {
                        metaData._type = DMHelper::FileType_Image;
                        currentItem->setData(QVariant::fromValue(metaData), MAPMANAGERDIALOG_METADATA);
                    }

                    return;
                }
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

    QVariant variantData = item->data(MAPMANAGERDIALOG_METADATA);
    if(!variantData.isValid())
        return;

    MapFileMetadata metaData = variantData.value<MapFileMetadata>();
    if(metaData._filePath.isEmpty())
        return;

    QPixmap itemPixmap(metaData._filePath);
    if(itemPixmap.isNull())
        return;

    QLabel* imageLabel = new QLabel;
    QScrollArea* scrollArea = new QScrollArea;
    QDialog* imageDialog = new QDialog(this);

    imageLabel->setScaledContents(false);
    imageLabel->setPixmap(itemPixmap);
    scrollArea->setWidgetResizable(false);
    scrollArea->setWidget(imageLabel);
    imageDialog->setWindowTitle(tr("Map Preview - %1").arg(metaData._filePath));
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

    setCurrentPath(mapPath);
}

void MapManagerDialog::findMaps()
{
    _options.setLastMapDirectory(ui->edtMapPath->text());

    QDir mapDir(ui->edtMapPath->text());
    if(!mapDir.exists())
        return;

    _currentPath = ui->edtMapPath->text();

    _model->clear();
    _searchList.clear();
    _searchList.append(QPair<QStandardItem*, const QString&>(_model->invisibleRootItem(), ui->edtMapPath->text()));
    QTimer::singleShot(0, this, &MapManagerDialog::scanNextDirectory);
}

void MapManagerDialog::scanNextDirectory()
{
    if(_searchList.isEmpty())
        return;

    QPair<QStandardItem*, QString> currentPair = _searchList.takeFirst();
    scanDirectory(currentPair.first, currentPair.second);

    if(!_searchList.isEmpty())
        QTimer::singleShot(0, this, &MapManagerDialog::scanNextDirectory);
    else
        QTimer::singleShot(0, this, &MapManagerDialog::writeModel);
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
        MapFileMetadata metaData{ DMHelper::FileType_Unknown, entry.absoluteFilePath(), QStringList() };
        if(entry.isDir())
        {
            metaData._type = DMHelper::FileType_Directory;
            _searchList.append(QPair<QStandardItem*, QString>(newItem, entry.absoluteFilePath()));
        }
        else if(entry.isFile())
        {
            //if(QImageReader(entry.absoluteFilePath()).canRead())
        }
        newItem->setData(QVariant::fromValue(metaData), MAPMANAGERDIALOG_METADATA);
        parent->appendRow(newItem);
    }    
}

void MapManagerDialog::readModel()
{
    if((!_model) || (!_model->invisibleRootItem()) || (_currentPath.isEmpty()))
        return;

    _searchList.clear();
    _model->clear();

    QDomDocument doc("DMHelperXML");
    QString modelFile = DMHCache().getCacheFilePath(_currentPath, QString("xml"));
    QFile file(modelFile);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[MapManagerDialog] Loading Failed: Unable to open map manager cache file: " << _currentPath << ", file: " << modelFile;
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString contentError;
    int contentErrorLine = 0;
    int contentErrorColumn = 0;
    bool contentResult = doc.setContent(in.readAll(), &contentError, &contentErrorLine, &contentErrorColumn);

    file.close();

    if(contentResult == false)
    {
        qDebug() << "[MapManagerDialog] Loading Failed: Error reading XML " << _currentPath << ", file: " << modelFile << ", (line " << contentErrorLine << ", column " << contentErrorColumn << "): " << contentError;
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "root"))
    {
        qDebug() << "[MapManagerDialog] Loading Failed: Error reading XML - unable to find root entry: " << _currentPath << ", file: " << modelFile;
        return;
    }

    QDomElement childEntry = root.firstChildElement("entry");
    while(!childEntry.isNull())
    {
        inputItemXML(childEntry, *_model->invisibleRootItem());
        childEntry = childEntry.nextSiblingElement("entry");
    }
}

void MapManagerDialog::writeModel()
{
    if((!_model) || (_model->rowCount() == 0) || (_currentPath.isEmpty()))
        return;

    QDomDocument doc("DMHelperXML");

    QDomElement rootElement = doc.createElement("root");
    for(int i = 0; i < _model->rowCount(); ++i)
    {
        if(_model->item(i))
            outputItemXML(doc, rootElement, *_model->item(i));
    }
    outputItemXML(doc, rootElement, *_model->invisibleRootItem());
    doc.appendChild(rootElement);

    QString modelFile = DMHCache().getCacheFilePath(_currentPath, QString("xml"));

    QFile file(modelFile);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "[MapManagerDialog] Unable to open map manager cache file for writing: " << modelFile;
        qDebug() << "       Error " << file.error() << ": " << file.errorString();
        return;
    }

    QTextStream ts(&file);
    ts.setEncoding(QStringConverter::Utf8);
    ts << doc.toString();

    file.close();

    qDebug() << "[MapManagerDialog] Map manager cache file written: " << modelFile;
}

void MapManagerDialog::setCurrentPath(const QString& path)
{
    if(_currentPath == path)
        return;

    _currentPath = path;
    ui->edtMapPath->setText(_currentPath);

    QTimer::singleShot(0, this, &MapManagerDialog::readModel);
}

void MapManagerDialog::inputItemXML(QDomElement &element, QStandardItem& parent)
{
    QString path = element.attribute("path");
    if(path.isEmpty())
        return;

    QFileInfo entry(path);
    QStandardItem* newItem = new QStandardItem(entry.fileName());
    newItem->setEditable(false);

    MapFileMetadata metaData{ DMHelper::FileType_Unknown, path, QStringList() };
    if(element.hasAttribute("type"))
        metaData._type = static_cast<DMHelper::FileType>(element.attribute("type").toInt());
    if(element.hasAttribute("tags"))
        metaData._tags = element.attribute("tags").split(";", Qt::SkipEmptyParts);
    newItem->setData(QVariant::fromValue(metaData), MAPMANAGERDIALOG_METADATA);

    parent.appendRow(newItem);

    QDomElement childEntry = element.firstChildElement("entry");
    while(!childEntry.isNull())
    {
        inputItemXML(childEntry, *newItem);
        childEntry = childEntry.nextSiblingElement("entry");
    }
}

void MapManagerDialog::outputItemXML(QDomDocument &doc, QDomElement &parent, QStandardItem& item)
{
    QVariant variantData = item.data(MAPMANAGERDIALOG_METADATA);
    if(!variantData.isValid())
        return;

    MapFileMetadata metaData = variantData.value<MapFileMetadata>();
    if(metaData._filePath.isEmpty())
        return;

    QDomElement element = doc.createElement("entry");
    element.setAttribute("path", metaData._filePath);
    if(metaData._type != DMHelper::FileType_Unknown)
        element.setAttribute("type", metaData._type);
    if(!metaData._tags.isEmpty())
        element.setAttribute("tags", metaData._tags.join(";"));

    if(item.hasChildren())
    {
        for(int i = 0; i < item.rowCount(); ++i)
        {
            if(item.child(i))
                outputItemXML(doc, element, *item.child(i));
        }
    }

    parent.appendChild(element);
}
