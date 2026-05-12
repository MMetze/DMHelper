#include "mapmanagerdialog.h"
#include "ui_mapmanagerdialog.h"
#include "optionscontainer.h"
#include "dmhcache.h"
#include "dmconstants.h"
#include <QStandardItemModel>
#include <QFileDialog>
#include <QDir>
#include <QImageReader>
#include <QStandardPaths>
#include <QTimer>
#include <QScrollArea>
#include <QDomDocument>
#include <QDomElement>
#include <QListWidget>
#include <QKeyEvent>
#include <QShortcut>
#include <QMimeDatabase>
#include <QStyle>
#include <QApplication>
#include "videoplayerscreenshot.h"

const int MAPMANAGERDIALOG_CACHE_IMAGE_SIZE = 256;
const int MAPMANAGERDIALOG_METADATA = Qt::UserRole + 1;

/*
 * Next Steps
 *
 * Include a generic search that matches the filename
 *
 * store the directory list in the cache and read it at startup
 * create a mapping of image md5s to tags and read/store that as well
 * Enable tagging of each image or groups of images
 * Enable filtering based on tag, type, title, etc
 */

MapManagerDialog::MapManagerDialog(OptionsContainer& options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapManagerDialog),
    _model(new QStandardItemModel()),
    _proxy(new TagFilterProxyModel()),
    _options(options),
    _directories(),
    _searchList(),
    _tagList(),
    _scanning(false),
    _campaignOpen(false),
    _layerSceneAvailable(false)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);

    DMHCache().ensureCacheExists(QString("maps"));

    _proxy->setSourceModel(_model);
    ui->treeView->setModel(_proxy);

    connect(ui->btnBrowse, &QPushButton::clicked, this, &MapManagerDialog::addDirectory);
    connect(ui->btnRefresh, &QPushButton::clicked, this, &MapManagerDialog::refreshDirectories);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MapManagerDialog::selectItem);
    connect(ui->treeView, &QTreeView::doubleClicked, this, &MapManagerDialog::openPreviewDialog);
    connect(ui->btnAutoTag, &QPushButton::clicked, this, &MapManagerDialog::addTags);
    connect(ui->btnBrowseTags, &QPushButton::clicked, this, &MapManagerDialog::browseTags);
    connect(ui->edtSearch, &QLineEdit::editingFinished, this, &MapManagerDialog::handleSearchTagsEdited);
    connect(ui->btnClearFilter, &QPushButton::clicked, this, [this]() {
        ui->edtSearch->clear();
        _proxy->clearRequiredTags();
        _proxy->clearSearch();
        // Ensure the current selection is visible after filter is cleared
        QModelIndex currentIndex = ui->treeView->selectionModel()->currentIndex();
        if(currentIndex.isValid())
            ui->treeView->scrollTo(currentIndex, QAbstractItemView::PositionAtCenter);
    });
    connect(ui->btnCreateEntry, &QPushButton::clicked, this, &MapManagerDialog::handleCreateEntry);
    connect(ui->btnAddLayer, &QPushButton::clicked, this, &MapManagerDialog::handleAddLayer);
    connect(ui->btnRemoveDir, &QPushButton::clicked, this, &MapManagerDialog::removeDirectory);

    QPushButton* btnPreview = new QPushButton(QIcon(":/img/data/icon_zoomin.png"), QString(), ui->lblPreview);
    btnPreview->setMinimumSize(40, 40);
    btnPreview->setMaximumSize(40, 40);
    btnPreview->setIconSize(QSize(35, 35));
    btnPreview->setToolTip(tr("Preview Map"));
    btnPreview->move(5, 5);
    connect(btnPreview, &QPushButton::clicked, this, &MapManagerDialog::previewCurrentItem);

    ui->edtTags->installEventFilter(this);
    ui->lblScanStatus->hide();
    updateActionButtons();
}

MapManagerDialog::~MapManagerDialog()
{
    _searchList.clear();
    delete ui;
}

void MapManagerDialog::setCampaignOpen(bool open)
{
    _campaignOpen = open;
    updateActionButtons();
}

void MapManagerDialog::setLayerSceneAvailable(bool available)
{
    _layerSceneAvailable = available;
    updateActionButtons();
}

void MapManagerDialog::showEvent(QShowEvent *event)
{
    if(_directories.isEmpty())
        loadDirectories();

    QDialog::showEvent(event);
}

void MapManagerDialog::selectItem(const QItemSelection &current, const QItemSelection &previous)
{
    Q_UNUSED(previous);

    if((!_model) || (!_proxy) || (current.indexes().count() == 0))
        return;

    if(current.indexes().count() == 1)
        selectSingleEntry(_model->itemFromIndex(_proxy->mapToSource(current.indexes().first())));
    else
        selectMultipleEntries(current);

    updateActionButtons();
}

void MapManagerDialog::openPreviewDialog(const QModelIndex &current)
{
    if(!current.isValid())
        return;

    MapFileMetadata metaData = getMetadataFromIndex(current);
    if(metaData._filePath.isEmpty())
        return;

    QPixmap itemPixmap(metaData._filePath);
    if(itemPixmap.isNull())
        return;

    QLabel* imageLabel = new QLabel;
    QScrollArea* scrollArea = new QScrollArea;
    QMainWindow* imageWindow = new QMainWindow(this);

    imageLabel->setScaledContents(false);
    imageLabel->setPixmap(itemPixmap);
    scrollArea->setWidgetResizable(false);
    scrollArea->setWidget(imageLabel);
    imageWindow->setWindowTitle(tr("DMHelper Map Preview - %1").arg(metaData._filePath));
    imageWindow->resize(qMin(itemPixmap.width() + 20, width()), qMin(itemPixmap.height() + 20, height()));
    imageWindow->setCentralWidget(scrollArea);
    imageWindow->setAttribute(Qt::WA_DeleteOnClose);
    QShortcut* escapeShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), imageWindow, SLOT(close()));
    QShortcut* returnShortcut = new QShortcut(QKeySequence(Qt::Key_Return), imageWindow, SLOT(close()));
    imageWindow->show();
}

void MapManagerDialog::previewCurrentItem()
{
    if(!ui->treeView->selectionModel())
        return;

    openPreviewDialog(ui->treeView->selectionModel()->currentIndex());
}

void MapManagerDialog::addDirectory()
{
    QString mapPath = QFileDialog::getExistingDirectory(this, tr("Select Map Directory"),
        _directories.isEmpty() ? QString() : _directories.last());
    if(mapPath.isEmpty())
        return;

    if(_directories.contains(mapPath))
        return;

    _directories.append(mapPath);
    _options.addMapDirectory(mapPath);
    _options.writeSettings();

    // Create root item for the new directory and scan it
    QDir mapDir(mapPath);
    QStandardItem* rootItem = new QStandardItem(mapDir.dirName());
    rootItem->setEditable(false);
    MapFileMetadata rootMeta{ DMHelper::FileType_Directory, mapPath, QStringList() };
    rootItem->setData(QVariant::fromValue(rootMeta), MAPMANAGERDIALOG_METADATA);
    applyFileTypeIcon(rootItem, DMHelper::FileType_Directory);
    _model->appendRow(rootItem);

    // Read cached data first
    readModelForDirectory(mapPath, rootItem);

    // Then scan for new files
    _searchList.clear();
    _searchList.append(QPair<QStandardItem*, QString>(rootItem, mapPath));
    _scanning = true;
    QTimer::singleShot(0, this, &MapManagerDialog::scanNextDirectory);
}

void MapManagerDialog::removeDirectory()
{
    if(!ui->treeView->selectionModel())
        return;

    QModelIndex currentIndex = ui->treeView->selectionModel()->currentIndex();
    if(!currentIndex.isValid())
        return;

    // Only allow removing top-level items (root directories)
    QModelIndex sourceIndex = _proxy->mapToSource(currentIndex);
    if(sourceIndex.parent().isValid())
        return; // not a top-level item

    QStandardItem* item = _model->itemFromIndex(sourceIndex);
    if(!item)
        return;

    MapFileMetadata metaData = getMetadataFromItem(item);
    _directories.removeAll(metaData._filePath);
    _options.removeMapDirectory(metaData._filePath);
    _options.writeSettings();

    _model->removeRow(sourceIndex.row());
}

void MapManagerDialog::refreshDirectories()
{
    if(_scanning)
        return;

    _searchList.clear();

    for(int i = 0; i < _model->rowCount(); ++i)
    {
        QStandardItem* rootItem = _model->item(i);
        if(rootItem)
        {
            MapFileMetadata metaData = getMetadataFromItem(rootItem);
            if(!metaData._filePath.isEmpty())
                _searchList.append(QPair<QStandardItem*, QString>(rootItem, metaData._filePath));
        }
    }

    if(!_searchList.isEmpty())
    {
        _scanning = true;
        QTimer::singleShot(0, this, &MapManagerDialog::scanNextDirectory);
    }
}

void MapManagerDialog::findMaps()
{
    refreshDirectories();
}

void MapManagerDialog::scanNextDirectory()
{
    if(_searchList.isEmpty())
    {
        _scanning = false;
        ui->lblScanStatus->hide();
        QTimer::singleShot(0, this, &MapManagerDialog::writeModel);
        return;
    }

    QPair<QStandardItem*, QString> currentPair = _searchList.takeFirst();

    ui->lblScanStatus->setText(tr("Scanning: %1 (%2 remaining)").arg(QDir(currentPair.second).dirName()).arg(_searchList.count()));
    ui->lblScanStatus->show();

    scanDirectory(currentPair.first, currentPair.second);

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

    // Build a set of known child paths for O(1) duplicate checking
    QSet<QString> existingPaths;
    for(int i = 0; i < parent->rowCount(); ++i)
    {
        MapFileMetadata meta = getMetadataFromItem(parent->child(i));
        if(!meta._filePath.isEmpty())
            existingPaths.insert(meta._filePath);
    }

    for(const QFileInfo &entry : dirEntries)
    {
        QString absoluteFilePath = entry.absoluteFilePath();

        // Only process directories and supported media files
        if(!entry.isDir())
        {
            int fileType = classifyFileType(absoluteFilePath);
            if(fileType != DMHelper::FileType_Image && fileType != DMHelper::FileType_Video)
                continue;
        }

        if(!existingPaths.contains(absoluteFilePath))
        {
            QStandardItem* entryItem = new QStandardItem(entry.fileName());
            entryItem->setEditable(false);
            MapFileMetadata metaData{ DMHelper::FileType_Unknown, absoluteFilePath, QStringList() };
            if(entry.isDir())
                metaData._type = DMHelper::FileType_Directory;
            else
                metaData._type = classifyFileType(absoluteFilePath);
            metaData._tags = proposeTags(absoluteFilePath);

            entryItem->setData(QVariant::fromValue(metaData), MAPMANAGERDIALOG_METADATA);
            applyFileTypeIcon(entryItem, metaData._type);
            parent->appendRow(entryItem);

            if(entry.isDir())
                _searchList.append(QPair<QStandardItem*, QString>(entryItem, absoluteFilePath));
        }
        else if(entry.isDir())
        {
            // Already exists — still need to queue subdirectory for scanning
            QStandardItem* existingItem = containsEntry(*parent, absoluteFilePath);
            if(existingItem)
                _searchList.append(QPair<QStandardItem*, QString>(existingItem, absoluteFilePath));
        }
    }
}

void MapManagerDialog::addTags()
{
    if(!_model)
        return;

    for(int i = 0; i < _model->rowCount(); ++i)
    {
        if(_model->item(i))
            addTagsToItem(*_model->item(i));
    }

    // Update the tags text for the currently selected item
    if(ui->treeView->selectionModel()->selection().indexes().count() == 1)
    {
        MapFileMetadata metaData = getMetadataFromIndex(ui->treeView->selectionModel()->selection().indexes().first());
        ui->edtTags->setText(metaData._tags.join(QChar::Space));
    }
    else if(ui->treeView->selectionModel()->selection().indexes().count() > 1)
    {
        selectMultipleEntries(ui->treeView->selectionModel()->selection());
    }

    QTimer::singleShot(0, this, &MapManagerDialog::writeModel);
}

void MapManagerDialog::addTagsToItem(QStandardItem& item)
{    
    MapFileMetadata metaData = getMetadataFromItem(&item);

    if((metaData._type != DMHelper::FileType_Directory) && (!metaData._filePath.isEmpty()))
    {
        QFileInfo fileInfo(metaData._filePath);
        QStringList newTags = proposeTags(fileInfo.absoluteFilePath());
        if(metaData._tags.isEmpty())
        {
            metaData._tags = newTags;
            registerTags(newTags);
        }
        else
        {
            for(QString& tagString : newTags)
            {
                if(!metaData._tags.contains(tagString))
                {
                    metaData._tags.append(tagString);
                    registerTag(tagString);
                }
            }
        }

        if(!metaData._tags.isEmpty())
            item.setData(QVariant::fromValue(metaData), MAPMANAGERDIALOG_METADATA);
    }

    for(int i = 0; i < item.rowCount(); ++i)
    {
        if(item.child(i))
            addTagsToItem(*item.child(i));
    }
}

void MapManagerDialog::browseTags()
{
    if((_tagList.isEmpty()) || (!_proxy))
        return;

    QDialog* dlgTags = new QDialog(this);

    QVBoxLayout* vLayout = new QVBoxLayout();

    QListWidget* listWidget = new QListWidget();
    listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    listWidget->setStyleSheet(QString("QListWidget { background: transparent; }"));
    QStringList tags = _tagList.values();
    tags.sort(Qt::CaseInsensitive);
    for(const QString& tag : tags)
    {
        QListWidgetItem* newItem = new QListWidgetItem(tag);
        listWidget->addItem(newItem);
        if(_proxy->getRequiredTags().contains(tag))
            newItem->setSelected(true);
    }
    vLayout->addWidget(listWidget);

    QHBoxLayout* hLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton(QString("OK"));
    hLayout->addWidget(okButton);
    connect(okButton, &QPushButton::clicked, dlgTags, &QDialog::accept);
    QPushButton* cancelButton = new QPushButton(QString("Cancel"));
    hLayout->addWidget(cancelButton);
    connect(cancelButton, &QPushButton::clicked, dlgTags, &QDialog::reject);
    hLayout->insertStretch(0,10);
    QPushButton* clearButton = new QPushButton(QString("Clear"));
    hLayout->insertWidget(0, clearButton);
    connect(clearButton, &QPushButton::clicked, listWidget, &QListWidget::clearSelection);
    vLayout->addLayout(hLayout);

    dlgTags->setLayout(vLayout);
    if(dlgTags->exec() == QDialog::Accepted)
    {
        QStringList selectedTags;
        QList<QListWidgetItem*> selectedItems = listWidget->selectedItems();
        for(QListWidgetItem* item : selectedItems)
        {
            selectedTags.append(item->text());
        }
        _proxy->setRequiredTags(selectedTags);
        ui->edtSearch->setText(selectedTags.join(QChar::Space));
    }

    dlgTags->deleteLater();
}

void MapManagerDialog::handleSearchTagsEdited()
{
    if(!_proxy)
        return;

    QString searchText = ui->edtSearch->text().trimmed();
    if(searchText.isEmpty())
    {
        _proxy->clearRequiredTags();
        _proxy->clearSearch();
        return;
    }

    // Split into tokens: words starting with '#' are explicit tag filters,
    // everything else is used for both tag matching AND filename search
    QStringList tokens = searchText.split(QChar::Space, Qt::SkipEmptyParts);
    QStringList tags;
    QStringList searchParts;

    for(const QString& token : tokens)
    {
        if(token.startsWith('#'))
        {
            QString tag = token.mid(1);
            if(!tag.isEmpty())
                tags.append(tag);
        }
        else
        {
            searchParts.append(token);
            // Also add as a tag filter if it matches a known tag
            if(_tagList.contains(token.toLower()))
                tags.append(token.toLower());
        }
    }

    _proxy->setRequiredTags(tags);
    _proxy->setSearchText(searchParts.join(QChar::Space));
}

void MapManagerDialog::handleTagsEdited()
{
    if((!_model) || (!_proxy) || (ui->treeView->selectionModel()->selection().indexes().count() == 0))
        return;

    if(ui->treeView->selectionModel()->selection().indexes().count() == 1)
        changeSingleEntry(_model->itemFromIndex(_proxy->mapToSource(ui->treeView->selectionModel()->selection().indexes().first())));
    else
        changeMultipleEntries(ui->treeView->selectionModel()->selection());
}

void MapManagerDialog::readModel()
{
    clearModel();

    ui->lblScanStatus->setText(tr("Loading cached data..."));
    ui->lblScanStatus->show();

    for(const QString& dirPath : _directories)
    {
        QDir dir(dirPath);
        QStandardItem* rootItem = new QStandardItem(dir.dirName());
        rootItem->setEditable(false);
        MapFileMetadata rootMeta{ DMHelper::FileType_Directory, dirPath, QStringList() };
        rootItem->setData(QVariant::fromValue(rootMeta), MAPMANAGERDIALOG_METADATA);
        applyFileTypeIcon(rootItem, DMHelper::FileType_Directory);
        _model->appendRow(rootItem);

        readModelForDirectory(dirPath, rootItem);
    }

    ui->lblScanStatus->hide();
}

void MapManagerDialog::readModelForDirectory(const QString& dirPath, QStandardItem* rootItem)
{
    if((!rootItem) || (dirPath.isEmpty()))
        return;

    QDomDocument doc("DMHelperXML");
    QString modelFile = DMHCache().getCacheFilePath(dirPath, QString("xml"));
    QFile file(modelFile);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[MapManagerDialog] Loading Failed: Unable to open map manager cache file: " << dirPath << ", file: " << modelFile;
        return;
    }

    // Safety: reject unreasonably large cache files (likely corrupted by old double-write bug)
    qint64 fileSize = file.size();
    if(fileSize > 50 * 1024 * 1024) // 50 MB
    {
        qDebug() << "[MapManagerDialog] Cache file too large (" << fileSize << " bytes), deleting corrupt cache: " << modelFile;
        file.close();
        QFile::remove(modelFile);
        return;
    }

    qDebug() << "[MapManagerDialog] Reading map manager cache file: " << dirPath << ", file: " << modelFile;

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QDomDocument::ParseResult contentResult = doc.setContent(in.readAll());

    file.close();

    if(!contentResult)
    {
        qDebug() << "[MapManagerDialog] Loading Failed: Error reading XML " << dirPath << ", file: " << modelFile << ", (line " << contentResult.errorLine << ", column " << contentResult.errorColumn << "): " << contentResult.errorMessage;
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "root"))
    {
        qDebug() << "[MapManagerDialog] Loading Failed: Error reading XML - unable to find root entry: " << dirPath << ", file: " << modelFile;
        return;
    }

    QDomElement childEntry = root.firstChildElement("entry");
    while(!childEntry.isNull())
    {
        inputItemXML(childEntry, *rootItem);
        childEntry = childEntry.nextSiblingElement("entry");
    }
}

void MapManagerDialog::writeModel()
{
    if(!_model || _model->rowCount() == 0)
        return;

    // Write each top-level root directory separately
    for(int i = 0; i < _model->rowCount(); ++i)
    {
        QStandardItem* rootItem = _model->item(i);
        if(!rootItem)
            continue;

        MapFileMetadata rootMeta = getMetadataFromItem(rootItem);
        if(!rootMeta._filePath.isEmpty())
            writeModelForDirectory(rootMeta._filePath, rootItem);
    }
}

void MapManagerDialog::writeModelForDirectory(const QString& dirPath, QStandardItem* rootItem)
{
    if((!rootItem) || (dirPath.isEmpty()) || (rootItem->rowCount() == 0))
        return;

    QDomDocument doc("DMHelperXML");

    QDomElement rootElement = doc.createElement("root");
    for(int i = 0; i < rootItem->rowCount(); ++i)
    {
        if(rootItem->child(i))
            outputItemXML(doc, rootElement, *rootItem->child(i));
    }
    doc.appendChild(rootElement);

    QString modelFile = DMHCache().getCacheFilePath(dirPath, QString("xml"));

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

void MapManagerDialog::handleCreateEntry()
{
    if(!ui->treeView->selectionModel())
        return;

    // Inform the application to add a new entry
    if(ui->treeView->selectionModel()->selection().indexes().count() != 1)
        return;

    MapFileMetadata metaData = getMetadataFromIndex(ui->treeView->selectionModel()->selection().indexes().first());
    if(metaData._type == DMHelper::FileType_Directory)
        return;

    if(metaData._filePath.isEmpty())
        return;

    emit createEntryImage(metaData._filePath);
}

void MapManagerDialog::handleAddLayer()
{
    if(!ui->treeView->selectionModel())
        return;

    if(ui->treeView->selectionModel()->selection().indexes().count() != 1)
        return;

    MapFileMetadata metaData = getMetadataFromIndex(ui->treeView->selectionModel()->selection().indexes().first());
    if(metaData._type == DMHelper::FileType_Directory)
        return;

    if(metaData._filePath.isEmpty())
        return;

    emit addLayerImage(metaData._filePath);
}

bool MapManagerDialog::eventFilter(QObject* object, QEvent* event)
{
    if((object) && (event) && (object == ui->edtTags))
    {
        if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if((keyEvent->key() == Qt::Key_Return) || (keyEvent->key() == Qt::Key_Enter))
            {
                handleTagsEdited();
                return true;
            }
        }
        else if(event->type() == QEvent::FocusOut)
        {
            handleTagsEdited();
        }
    }

    return QDialog::eventFilter(object, event);
}

void MapManagerDialog::updateActionButtons()
{
    bool isMediaSelected = false;

    if(ui->treeView->selectionModel() && ui->treeView->selectionModel()->currentIndex().isValid())
    {
        MapFileMetadata metaData = getMetadataFromIndex(ui->treeView->selectionModel()->currentIndex());
        isMediaSelected = (metaData._type == DMHelper::FileType_Image || metaData._type == DMHelper::FileType_Video);
    }

    ui->btnCreateEntry->setEnabled(_campaignOpen && isMediaSelected);
    ui->btnAddLayer->setEnabled(_layerSceneAvailable && isMediaSelected);
}

void MapManagerDialog::clearModel()
{
    _searchList.clear();
    _model->clear();
    _tagList.clear();
}

QStandardItem* MapManagerDialog::containsEntry(QStandardItem& item, const QString& fullPath)
{
    if(fullPath.isEmpty())
        return nullptr;

    if(item.hasChildren())
    {
        for(int i = 0; i < item.rowCount(); ++i)
        {
            MapFileMetadata metaData = getMetadataFromItem(item.child(i));
            if(metaData._filePath == fullPath)
                return item.child(i);
        }
    }

    return nullptr;
}

void MapManagerDialog::loadDirectories()
{
    _directories = _options.getMapDirectories();

    // Migration: if no directories but a last map directory exists, use it
    if(_directories.isEmpty())
    {
        QString lastDir = _options.getLastMapDirectory();
        if(!lastDir.isEmpty())
        {
            _directories.append(lastDir);
            _options.addMapDirectory(lastDir);
        }
    }

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
    {
        metaData._tags = element.attribute("tags").split(";", Qt::SkipEmptyParts);
        registerTags(metaData._tags);
    }
    newItem->setData(QVariant::fromValue(metaData), MAPMANAGERDIALOG_METADATA);
    applyFileTypeIcon(newItem, metaData._type);

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
    MapFileMetadata metaData = getMetadataFromItem(&item);
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

QStandardItem* MapManagerDialog::getItemFromIndex(const QModelIndex &index)
{
    if((!_model) || (!_proxy) || (!index.isValid()))
        return nullptr;
    else
        return _model->itemFromIndex(_proxy->mapToSource(index));
}

MapManagerDialog::MapFileMetadata MapManagerDialog::getMetadataFromItem(QStandardItem* item)
{
    MapFileMetadata metaData;

    if(item)
    {
        QVariant variantData = item->data(MAPMANAGERDIALOG_METADATA);
        if(variantData.isValid())
            metaData = variantData.value<MapFileMetadata>();
    }

    return metaData;
}

MapManagerDialog::MapFileMetadata MapManagerDialog::getMetadataFromIndex(const QModelIndex &index)
{
    MapFileMetadata metaData;

    if(index.isValid())
    {
        QVariant variantData = index.data(MAPMANAGERDIALOG_METADATA);
        if(variantData.isValid())
            metaData = variantData.value<MapFileMetadata>();
    }

    return metaData;
}

void MapManagerDialog::selectSingleEntry(QStandardItem* item)
{
    if(!item)
        return;

    MapFileMetadata metaData = getMetadataFromItem(item);
    ui->edtTags->setText(metaData._tags.join(QChar::Space));
    ui->edtMapPath->setText(metaData._filePath);

    if(metaData._type != DMHelper::FileType_Directory)
    {
        QPixmap itemPixmap;
        QString cacheFile = DMHCache().getCacheFilePath(metaData._filePath, QString("png"), QString("maps"));
        if((!cacheFile.isEmpty()) && (QFile::exists(cacheFile)))
            itemPixmap.load(cacheFile);

        if(itemPixmap.isNull() && metaData._type != DMHelper::FileType_Video)
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

        if(itemPixmap.isNull() && metaData._type == DMHelper::FileType_Video)
        {
            VideoPlayerScreenshot* screenshot = new VideoPlayerScreenshot(metaData._filePath, this);
            connect(screenshot, &VideoPlayerScreenshot::screenshotReady, this, [this](const QImage& image) {
                if(!image.isNull())
                {
                    QImage thumb = image.scaled(MAPMANAGERDIALOG_CACHE_IMAGE_SIZE, MAPMANAGERDIALOG_CACHE_IMAGE_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    QPixmap pix = QPixmap::fromImage(thumb);
                    ui->lblPreview->setText(QString());
                    ui->lblPreview->setPixmap(pix.scaled(ui->lblPreview->contentsRect().size(), Qt::KeepAspectRatio));
                }
            });
            screenshot->retrieveScreenshot();
            ui->lblPreview->setText(tr("Generating video thumbnail..."));
            ui->lblPreview->setEnabled(true);
            return;
        }

        if(!itemPixmap.isNull())
        {
            ui->lblPreview->setText(QString());
            ui->lblPreview->setPixmap(itemPixmap.scaled(ui->lblPreview->contentsRect().size(), Qt::KeepAspectRatio));

            if(metaData._type == DMHelper::FileType_Unknown)
            {
                metaData._type = DMHelper::FileType_Image;
                item->setData(QVariant::fromValue(metaData), MAPMANAGERDIALOG_METADATA);
            }

            return;
        }
    }

    ui->lblPreview->setText(tr("No Preview Available"));
    ui->lblPreview->setEnabled(true);
}

void MapManagerDialog::selectMultipleEntries(const QItemSelection &selection)
{
    if(selection.indexes().count() == 0)
        return;

    MapFileMetadata firstMetaData = getMetadataFromIndex(selection.indexes().first());
    QStringList combinedTags = firstMetaData._tags;

    for(const QModelIndex& index : selection.indexes())
    {
        MapFileMetadata metaData = getMetadataFromIndex(index);
        for(const QString& tagString : combinedTags)
        {
            if(!metaData._tags.contains(tagString))
                combinedTags.removeAll(tagString);
        }
    }

    ui->edtTags->setText(combinedTags.join(QChar::Space));

    ui->lblPreview->setText(tr("No Preview Available"));
    ui->lblPreview->setEnabled(false);
}

void MapManagerDialog::changeSingleEntry(QStandardItem* item)
{
    if(!item)
        return;

    MapFileMetadata metaData = getMetadataFromItem(item);
    metaData._tags = ui->edtTags->toPlainText().split(QChar::Space);
    item->setData(QVariant::fromValue(metaData), MAPMANAGERDIALOG_METADATA);
}

void MapManagerDialog::changeMultipleEntries(const QItemSelection &selection)
{
    if(selection.indexes().count() == 0)
        return;

    QStringList combinedTags = ui->edtTags->toPlainText().split(QChar::Space);

    for(const QModelIndex& index : selection.indexes())
    {
        QStandardItem* item = getItemFromIndex(index);
        if(item)
        {
            bool changed = false;
            MapFileMetadata metaData = getMetadataFromItem(item);
            for(const QString& tagString : combinedTags)
            {
                if(!metaData._tags.contains(tagString))
                {
                    metaData._tags.append(tagString);
                    changed = true;
                }
            }

            if(changed)
                item->setData(QVariant::fromValue(metaData), MAPMANAGERDIALOG_METADATA);
        }
    }
}

void MapManagerDialog::registerTag(const QString& tag)
{
    _tagList.insert(tag.toLower());
}

void MapManagerDialog::registerTags(const QStringList& tags)
{
    for(const QString& tagString : tags)
    {
        registerTag(tagString);
    }
}

int MapManagerDialog::classifyFileType(const QString& filename) const
{
    static const QSet<QString> imageExts = { "jpg", "jpeg", "png", "bmp", "gif", "tga", "tiff", "webp" };
    static const QSet<QString> videoExts = { "mp4", "m4v", "webm", "avi", "mkv", "mov" };
    static const QSet<QString> audioExts = { "mp3", "wav", "ogg", "flac", "m4a" };
    static const QSet<QString> docExts = { "pdf", "docx", "txt", "md", "html", "htm", "url" };

    // Extension-based only - no filesystem I/O. Directory detection is handled
    // by the caller (scanDirectory uses QFileInfo::isDir from the dir listing).
    QString ext = QFileInfo(filename).suffix().toLower();

    if(imageExts.contains(ext))
        return DMHelper::FileType_Image;
    if(videoExts.contains(ext))
        return DMHelper::FileType_Video;
    if(audioExts.contains(ext))
        return DMHelper::FileType_Audio;
    if(docExts.contains(ext))
        return DMHelper::FileType_Text;
    return DMHelper::FileType_Unknown;
}

QIcon MapManagerDialog::iconForFileType(int fileType) const
{
    QStyle* appStyle = QApplication::style();
    switch(fileType)
    {
        case DMHelper::FileType_Directory:
            return appStyle->standardIcon(QStyle::SP_DirIcon);
        case DMHelper::FileType_Image:
            return appStyle->standardIcon(QStyle::SP_FileIcon);
        case DMHelper::FileType_Video:
            return appStyle->standardIcon(QStyle::SP_MediaPlay);
        case DMHelper::FileType_Audio:
            return appStyle->standardIcon(QStyle::SP_MediaVolume);
        default:
            return appStyle->standardIcon(QStyle::SP_FileIcon);
    }
}

void MapManagerDialog::applyFileTypeIcon(QStandardItem* item, int fileType)
{
    if(!item)
        return;

    // Cache icons to avoid thousands of QStyle::standardIcon() lookups
    static QIcon dirIcon;
    static QIcon imageIcon;
    static QIcon videoIcon;
    static QIcon audioIcon;
    static QIcon defaultIcon;
    static bool iconsInitialized = false;

    if(!iconsInitialized)
    {
        QStyle* appStyle = QApplication::style();
        dirIcon = appStyle->standardIcon(QStyle::SP_DirIcon);
        imageIcon = appStyle->standardIcon(QStyle::SP_FileIcon);
        videoIcon = appStyle->standardIcon(QStyle::SP_MediaPlay);
        audioIcon = appStyle->standardIcon(QStyle::SP_MediaVolume);
        defaultIcon = appStyle->standardIcon(QStyle::SP_FileIcon);
        iconsInitialized = true;
    }

    switch(fileType)
    {
        case DMHelper::FileType_Directory:
            item->setIcon(dirIcon);
            break;
        case DMHelper::FileType_Image:
            item->setIcon(imageIcon);
            break;
        case DMHelper::FileType_Video:
            item->setIcon(videoIcon);
            break;
        case DMHelper::FileType_Audio:
            item->setIcon(audioIcon);
            break;
        default:
            item->setIcon(defaultIcon);
            break;
    }
}

void MapManagerDialog::groupVariants(QStandardItem* parent)
{
    if(!parent || parent->rowCount() < 3)
        return;

    // First recursively process child directories
    for(int i = 0; i < parent->rowCount(); ++i)
    {
        QStandardItem* child = parent->child(i);
        if(child)
        {
            MapFileMetadata meta = getMetadataFromItem(child);
            if(meta._type == DMHelper::FileType_Directory)
                groupVariants(child);
        }
    }

    // Collect non-directory children with their tokenized stems
    struct FileEntry {
        int row;
        QStandardItem* item;
        QStringList tokens;
        QString stem;
    };

    QList<FileEntry> fileEntries;
    for(int i = 0; i < parent->rowCount(); ++i)
    {
        QStandardItem* child = parent->child(i);
        if(!child)
            continue;

        MapFileMetadata meta = getMetadataFromItem(child);
        if(meta._type == DMHelper::FileType_Directory)
            continue;

        QFileInfo fi(meta._filePath);
        QString stem = fi.completeBaseName().toLower();
        QString cleaned = stem;
        cleaned.replace("_", " ").replace("-", " ").replace("[", " ").replace("]", " ");
        cleaned.replace("(", " ").replace(")", " ").replace(",", " ").replace(".", " ");
        QStringList tokens = cleaned.split(" ", Qt::SkipEmptyParts);

        fileEntries.append({i, child, tokens, stem});
    }

    if(fileEntries.count() < 3)
        return;

    // Safety: skip variant grouping for very large directories (O(n²) algorithm)
    if(fileEntries.count() > 500)
    {
        qDebug() << "[MapManagerDialog] Skipping variant grouping for" << parent->text() << "- too many entries:" << fileEntries.count();
        return;
    }

    // Find groups sharing a common prefix of >=2 tokens
    // Use a greedy approach: for each pair, find longest common prefix
    QMap<QString, QList<int>> prefixGroups; // prefix -> list of indices into fileEntries

    for(int i = 0; i < fileEntries.count(); ++i)
    {
        const QStringList& tokensA = fileEntries[i].tokens;
        if(tokensA.count() < 2)
            continue;

        // Build a prefix key from the first N tokens where N is at least 2
        // and matches the maximum shared prefix with any other entry
        int bestPrefixLen = 0;
        for(int j = 0; j < fileEntries.count(); ++j)
        {
            if(i == j)
                continue;
            const QStringList& tokensB = fileEntries[j].tokens;
            int commonLen = 0;
            int maxLen = qMin(tokensA.count(), tokensB.count());
            for(int k = 0; k < maxLen; ++k)
            {
                if(tokensA[k] == tokensB[k])
                    commonLen++;
                else
                    break;
            }
            if(commonLen >= 2)
                bestPrefixLen = qMax(bestPrefixLen, commonLen);
        }

        if(bestPrefixLen >= 2)
        {
            QString prefixKey = tokensA.mid(0, bestPrefixLen).join(" ");
            prefixGroups[prefixKey].append(i);
        }
    }

    // Only keep groups with 2+ members, and ensure no item appears in multiple groups
    QSet<int> assignedEntries;
    QList<QPair<QString, QList<int>>> validGroups;

    // Sort by prefix length descending to prefer longer prefixes
    QList<QString> sortedKeys = prefixGroups.keys();
    std::sort(sortedKeys.begin(), sortedKeys.end(), [&](const QString& a, const QString& b) {
        return a.count(' ') > b.count(' ');
    });

    for(const QString& key : sortedKeys)
    {
        QList<int>& indices = prefixGroups[key];
        QList<int> filtered;
        for(int idx : indices)
        {
            if(!assignedEntries.contains(idx))
                filtered.append(idx);
        }

        if(filtered.count() >= 2)
        {
            for(int idx : filtered)
                assignedEntries.insert(idx);
            validGroups.append({key, filtered});
        }
    }

    if(validGroups.isEmpty())
        return;

    // Create group items - process from highest row to lowest to preserve indices
    for(const auto& group : validGroups)
    {
        // Sort indices in descending order for safe removal
        QList<int> sortedIndices = group.second;
        std::sort(sortedIndices.begin(), sortedIndices.end(), std::greater<int>());

        // Find the original row indices
        QList<int> rowIndices;
        for(int entryIdx : sortedIndices)
            rowIndices.append(fileEntries[entryIdx].row);
        std::sort(rowIndices.begin(), rowIndices.end(), std::greater<int>());

        // Create group item
        QString groupName = group.first;
        groupName[0] = groupName[0].toUpper();
        QStandardItem* groupItem = new QStandardItem(tr("[Variants] %1 (%2)").arg(groupName).arg(group.second.count()));
        groupItem->setEditable(false);
        MapFileMetadata groupMeta{ DMHelper::FileType_Directory, QString(), QStringList() };
        groupItem->setData(QVariant::fromValue(groupMeta), MAPMANAGERDIALOG_METADATA);
        applyFileTypeIcon(groupItem, DMHelper::FileType_Directory);

        // Move items from parent into the group
        QList<QList<QStandardItem*>> takenRows;
        for(int row : rowIndices)
        {
            QList<QStandardItem*> taken = parent->takeRow(row);
            if(!taken.isEmpty())
                takenRows.prepend(taken); // prepend to maintain original order
        }

        for(const auto& taken : takenRows)
            groupItem->appendRow(taken);

        // Insert the group at the position of the first (lowest) original row
        int insertRow = rowIndices.last(); // the smallest row number
        if(insertRow > parent->rowCount())
            insertRow = parent->rowCount();
        parent->insertRow(insertRow, groupItem);
    }
}

QStringList MapManagerDialog::proposeTags(const QString &filename) const
{
    QSet<QString> tags;

    QFileInfo info(filename);
    QString base = info.completeBaseName().toLower();
    QString ext  = info.suffix().toLower();

    // --------- NORMALIZE ----------
    QString cleaned = base;
    cleaned.replace("_", " ");
    cleaned.replace("-", " ");
    cleaned.replace("[", " ");
    cleaned.replace("]", " ");
    cleaned.replace("(", " ");
    cleaned.replace(")", " ");
    cleaned.replace(",", " ");
    cleaned.replace(".", " ");

    QStringList tokens = cleaned.split(" ", Qt::SkipEmptyParts);

    // --------- CREATOR PREFIX MATCHING ----------
    // Check filename and parent directory names for creator prefixes
    QString fileNameOnly = info.fileName();
    QString parentDirName = QFileInfo(info.absolutePath()).fileName();
    for(const auto &pair : creators)
    {
        if(fileNameOnly.startsWith(pair.first, Qt::CaseInsensitive) ||
           parentDirName.startsWith(pair.first, Qt::CaseInsensitive))
            tags.insert(pair.second);
    }

    // --------- KEYWORD MATCHING ----------
    for(const QString &token : tokens)
    {
        if(keywordToTags.contains(token))
        {
            for (const QString &tag : keywordToTags[token])
                tags.insert(tag);
        }

        if(timeTags.contains(token))
            tags.insert(timeTags[token]);

        if(gridWords.contains(token))
            tags.insert(gridWords[token]);
    }

    // --------- EXTENSION MATCHING ----------
    if(extensionTags.contains(ext))
        tags.insert(extensionTags[ext]);

    return tags.values();
}

QStringList MapManagerDialog::TagFilterProxyModel::getRequiredTags()
{
    return _requiredTags;
}

void MapManagerDialog::TagFilterProxyModel::setRequiredTags(const QStringList& tags)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
    _requiredTags = tags;
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    _requiredTags = tags;
    invalidateFilter();
#endif
}

void MapManagerDialog::TagFilterProxyModel::clearRequiredTags()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
    _requiredTags.clear();
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    _requiredTags.clear();
    invalidateFilter();
#endif
}

QString MapManagerDialog::TagFilterProxyModel::getSearchText() const
{
    return _searchText;
}

void MapManagerDialog::TagFilterProxyModel::setSearchText(const QString& text)
{
    if(_searchText == text)
        return;

#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
    _searchText = text;
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    _searchText = text;
    invalidateFilter();
#endif
}

void MapManagerDialog::TagFilterProxyModel::clearSearch()
{
    if(_searchText.isEmpty())
        return;

#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
    _searchText.clear();
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    _searchText.clear();
    invalidateFilter();
#endif
}

bool MapManagerDialog::TagFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if(_requiredTags.isEmpty() && _searchText.isEmpty())
        return true;

    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    QVariant variantData = index.data(MAPMANAGERDIALOG_METADATA);
    MapFileMetadata metaData;
    if(variantData.isValid())
        metaData = variantData.value<MapFileMetadata>();

    // Check tag match (only relevant if tags are active)
    bool tagsMatch = !_requiredTags.isEmpty() && (listContainsAllTags(metaData._tags) || pathMatchesRow(index));

    // Check filename search match (only relevant if search text is active)
    bool searchMatch = !_searchText.isEmpty() && nameMatchesSearch(index);

    // Accept if either criterion is satisfied
    if(tagsMatch || searchMatch)
        return true;

    // Check the children (for directories)
    int childCount = sourceModel()->rowCount(index);
    for(int i = 0; i < childCount; ++i)
        if(filterAcceptsRow(i, index))
            return true;

    // Reject
    return false;
}

bool MapManagerDialog::TagFilterProxyModel::pathMatchesRow(const QModelIndex& rowIndex) const
{
    if(!rowIndex.isValid())
        return false;

    QString rowName = rowIndex.data().toString();
    if(rowIndex.data().toString().isEmpty())
        return false;
    
    QStringList nameList = rowIndex.data().toString().split(QChar::Space);
    foreach(const QString & namePart, nameList)
    {
        if(!_requiredTags.contains(namePart, Qt::CaseInsensitive))
            return pathMatchesRow(rowIndex.parent());
    }

    return true;
}

bool MapManagerDialog::TagFilterProxyModel::listContainsAllTags(const QStringList& list) const
{
    for(const QString& r : _requiredTags)
        if(!list.contains(r))
            return false;
    return true;
}

bool MapManagerDialog::TagFilterProxyModel::nameMatchesSearch(const QModelIndex& index) const
{
    if(!index.isValid() || _searchText.isEmpty())
        return true;

    QString displayName = index.data().toString();
    return displayName.contains(_searchText, Qt::CaseInsensitive);
}
