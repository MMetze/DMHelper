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
    _currentPath(),
    _searchList(),
    _tagList()
{
    ui->setupUi(this);

    DMHCache().ensureCacheExists(QString("maps"));

    _proxy->setSourceModel(_model);
    ui->treeView->setModel(_proxy);

    connect(ui->btnBrowse, &QPushButton::clicked, this, &MapManagerDialog::browsePath);
    connect(ui->btnRefresh, &QPushButton::clicked, this, &MapManagerDialog::findMaps);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MapManagerDialog::selectItem);
    connect(ui->treeView, &QTreeView::doubleClicked, this, &MapManagerDialog::openPreviewDialog);
    connect(ui->btnAutoTag, &QPushButton::clicked, this, &MapManagerDialog::addTags);
    connect(ui->btnBrowseTags, &QPushButton::clicked, this, &MapManagerDialog::browseTags);
    connect(ui->edtSearch, &QLineEdit::editingFinished, this, &MapManagerDialog::handleSearchTagsEdited);

    ui->edtTags->installEventFilter(this);
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

    if((!_model) || (!_proxy) || (current.indexes().count() == 0))
        return;

    if(current.indexes().count() == 1)
        selectSingleEntry(_model->itemFromIndex(_proxy->mapToSource(current.indexes().first())));
    else
        selectMultipleEntries(current);
}

void MapManagerDialog::openPreviewDialog(const QModelIndex &current)
{
    MapFileMetadata metaData = getMetadataFromIndex(current);
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
        QString absoluteFilePath = entry.absoluteFilePath();
        QStandardItem* entryItem = containsEntry(*parent, absoluteFilePath);
        if(!entryItem)
        {
            QStandardItem* newItem = new QStandardItem(entry.fileName());
            newItem->setEditable(false);
            MapFileMetadata metaData{ DMHelper::FileType_Unknown, absoluteFilePath, QStringList() };
            if(entry.isDir())
                metaData._type = DMHelper::FileType_Directory;
            metaData._tags = proposeTags(absoluteFilePath);

            newItem->setData(QVariant::fromValue(metaData), MAPMANAGERDIALOG_METADATA);
            parent->appendRow(newItem);
        }

        if(entry.isDir())
            _searchList.append(QPair<QStandardItem*, QString>(entryItem, absoluteFilePath));
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
        QStringList newTags = proposeTags(fileInfo.fileName());
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

    for(int i = 0; i < _model->rowCount(); ++i)
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
    if((!_proxy) || (ui->edtSearch->text().isEmpty()))
        return;

    QStringList editTags = ui->edtSearch->text().split(QChar::Space, Qt::SkipEmptyParts);
    _proxy->setRequiredTags(editTags);
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
    if((!_model) || (!_model->invisibleRootItem()) || (_currentPath.isEmpty()))
        return;

    clearModel();

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

bool MapManagerDialog::eventFilter(QObject* object, QEvent* event)
{
    if((object) && (event) && (object == ui->edtTags))
    {
        if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if((keyEvent->key() == Qt::Key_Return) || (keyEvent->key() != Qt::Key_Enter))
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
    {
        metaData._tags = element.attribute("tags").split(";", Qt::SkipEmptyParts);
        registerTags(metaData._tags);
    }
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
    _tagList.insert(tag);
}

void MapManagerDialog::registerTags(const QStringList& tags)
{
    for(const QString& tagString : tags)
    {
        registerTag(tagString);
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
    for(const auto &pair : creators)
    {
        if (filename.startsWith(pair.first, Qt::CaseInsensitive))
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
    _requiredTags = tags;
    invalidateFilter();
}

void MapManagerDialog::TagFilterProxyModel::clearRequiredTags()
{
    _requiredTags.clear();
    invalidateFilter();
}

bool MapManagerDialog::TagFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if(_requiredTags.isEmpty())
        return true;

    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    QVariant variantData = index.data(MAPMANAGERDIALOG_METADATA);
    if(!variantData.isValid())
        return true;

    MapFileMetadata metaData = variantData.value<MapFileMetadata>();

    // If any required tag matches → accept row
    if(listContainsAllTags(metaData._tags))
        return true;

    // Check the children
    int childCount = sourceModel()->rowCount(index);
    for(int i = 0; i < childCount; ++i)
        if(filterAcceptsRow(i, index))
            return true;

    // Reject
    return false;
}

bool MapManagerDialog::TagFilterProxyModel::listContainsAllTags(const QStringList& list) const
{
    for(const QString& r : _requiredTags)
        if(!list.contains(r))
            return false;
    return true;
}
