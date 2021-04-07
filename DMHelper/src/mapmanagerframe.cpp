#include "mapmanagerframe.h"
#include "ui_mapmanagerframe.h"
#include "mapmanagereditdirectoriesdialog.h"
#include <QScreen>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QMouseEvent>
#include <QtGlobal>
#include <QDebug>

const int MAP_FRAME_SPACING = 5;
const int MAP_FRAME_SIZE = 50;

MapManagerFrame::MapManagerFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MapManagerFrame),
    _scene(new QGraphicsScene(this)),
    _directories(),
    _categories(),
    _layoutColumns(-1),
    _labelSize(MAP_FRAME_SIZE),
    _labelGap(MAP_FRAME_SPACING),
    _previewPixmap(),
    _items(),
    _worker(new MapReaderWorker),
    _workerThread(new QThread)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(_scene);
    ui->graphicsView->installEventFilter(this);

    _worker->moveToThread(_workerThread);
    connect(_worker, &MapReaderWorker::processingStopped, this, &MapManagerFrame::populateTable);
    connect(_worker, &MapReaderWorker::imageReady, this, &MapManagerFrame::imageAvailable);
    connect(_workerThread, &QThread::finished, _worker, &QObject::deleteLater);
    _workerThread->start();

    connect(ui->splitter, &QSplitter::splitterMoved, this, &MapManagerFrame::layoutItems);
    connect(ui->splitter, &QSplitter::splitterMoved, this, &MapManagerFrame::layoutPreview);
    QList<int> sizes;
    sizes.append(2 * width() / 3);
    sizes.append(width() / 3);
    ui->splitter->setSizes(sizes);

    connect(ui->btnEditDirectories, &QAbstractButton::clicked, this, &MapManagerFrame::editDirectories);
    connect(ui->btnEditCategories, &QAbstractButton::clicked, this, &MapManagerFrame::editCategories);

    // TODO: DEBUG ONLY
    _directories.append(QDir("D:/DnD/Assets/Maps/Island Fort"));
    _directories.append(QDir("D:/DnD/Assets/Maps/James RPG"));
}

MapManagerFrame::~MapManagerFrame()
{
    _workerThread->quit();
    _workerThread->wait();

    delete ui;
}

bool MapManagerFrame::eventFilter(QObject *watched, QEvent *event)
{
    if((watched == ui->graphicsView) &&
       (event->type() == QEvent::MouseButtonPress))
    {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if(mouseEvent)
        {
            QGraphicsPixmapItem* item = dynamic_cast<QGraphicsPixmapItem*>(ui->graphicsView->itemAt(mouseEvent->pos()));
            if(item)
            {
                if(_previewPixmap.load(item->toolTip()))
                    layoutPreview();
            }

            return true;
        }
    }

    // standard event processing
    return QObject::eventFilter(watched, event);
}

void MapManagerFrame::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    layoutItems();
}

void MapManagerFrame::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    QScreen* primary = QGuiApplication::primaryScreen();
    if(primary)
    {
        _labelSize = primary->availableSize().height() / 10;
        _labelGap = _labelSize / 10;
    }
    else
    {
        _labelSize = MAP_FRAME_SIZE;
        _labelGap = MAP_FRAME_SPACING;
    }

    _worker->setPixmapSize(_labelSize);

    // Stop processing will lead to a signal that should populate the screen
    _worker->stopProcessing();
}

void MapManagerFrame::imageAvailable(const QString& filename, const QString& md5, QPixmap pixmap)
{
    Q_UNUSED(md5);

    QGraphicsPixmapItem* item = _items.value(filename, nullptr);
    if(item)
        item->setPixmap(pixmap);
}

void MapManagerFrame::layoutItems()
{
    int columnCount = getColumnCount();
    if(columnCount < 1)
        columnCount = 1;

    QList<QGraphicsItem *> itemList = _scene->items();

    int rowCount = (itemList.count() + columnCount) / columnCount;
    int labelSizeGap = _labelSize + _labelGap;

    for(int y = 0; y < rowCount; ++y)
    {
        for(int x = 0; x < columnCount; ++x)
        {
            int index = (y * columnCount) + x;
            if(index < itemList.count())
                itemList[index]->setPos(x * labelSizeGap, y * labelSizeGap);
        }
    }

    int viewHeight = qMax(rowCount * labelSizeGap, ui->graphicsView->height());
    ui->graphicsView->setSceneRect(-5.0, -5.0, ui->graphicsView->width(), viewHeight - 5.0);
}

void MapManagerFrame::layoutPreview()
{
    if(!_previewPixmap.isNull())
        ui->lblPreview->setPixmap(_previewPixmap.scaled(ui->lblPreview->size(), Qt::KeepAspectRatio));
}

void MapManagerFrame::editDirectories()
{
    QStringList directories;
    for(int i = 0; i < _directories.count(); ++i)
        directories.append(_directories.at(i).absolutePath());

    MapManagerEditDirectoriesDialog directoryDialog(directories, false, QString("Edit Map Directories"), QString(), this);
    directoryDialog.exec();

    _directories.clear();
    directories = directoryDialog.getDirectories();
    for(QString directory : directories)
        _directories.append(QDir(directory));

    populateTable();
}

void MapManagerFrame::editCategories()
{
    MapManagerEditDirectoriesDialog categoryDialog(_categories, true, QString("Edit Map Categories"), QString("Category"), this);
    categoryDialog.exec();
    _categories = categoryDialog.getDirectories();

    populateCategories();
}

void MapManagerFrame::populateTable()
{
    _items.clear();
    _scene->clear();

    for(QDir sourceDir : _directories)
    {
        scanDirectory(sourceDir);
    }

    layoutItems();
}

void MapManagerFrame::populateCategories()
{
    int columnCount = 6;

    ui->tblCategories->clear();
    ui->tblCategories->setColumnCount(columnCount);
    ui->tblCategories->setRowCount((_categories.count() / columnCount) + 1);

    for(int i = 0; i < _categories.count(); ++i)
    {
        QTableWidgetItem *newItem = new QTableWidgetItem(_categories.at(i));
        newItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        newItem->setCheckState(Qt::Unchecked);
        ui->tblCategories->setItem(i / columnCount, i % columnCount, newItem);
    }

    ui->listWidget->clear();
}

void MapManagerFrame::scanDirectory(QDir directory)
{
    qDebug() << "[MapManagerFrame] Scanning directory: " << directory.absolutePath();

    QStringList dirEntries = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for(QString dirEntry : dirEntries)
    {
        scanDirectory(QDir(directory.absolutePath() + QString("/") + dirEntry));
    }

    QStringList fileEntries = directory.entryList(QDir::Files);
    for(QString fileEntry : fileEntries)
    {
        if(directory.exists(fileEntry))
        {
            QString labelFile = QDir::cleanPath(directory.absoluteFilePath(fileEntry));
            QPixmap labelPixmap;
            //if(!labelPixmap.load(labelFile))
                labelPixmap.load(":/img/data/icon_contentmap.png");
            QGraphicsPixmapItem* newItem = _scene->addPixmap(labelPixmap.scaled(_labelSize, _labelSize, Qt::KeepAspectRatio));
            newItem->setToolTip(labelFile);
            qDebug() << "[MapManagerFrame] Adding image:" << labelFile;

            _items.insert(labelFile, newItem);
            _worker->addFilename(labelFile);
        }
    }
}

int MapManagerFrame::getColumnCount()
{
    if(_scene->items().count() == 0)
        return -1;

    //qDebug() << "[PartyFrame] character list width: " << ui->characterList->width() << ", viewport: " << ui->characterList->viewport()->width();
    //qDebug() << "[PartyFrame] single grid count: " << _characterFrames.at(0)->width() << ", hint: " << _characterFrames.at(0)->sizeHint();
    //qDebug() << "[PartyFrame] calculated columns: " << (ui->characterList->width() - PARTY_FRAME_SPACING) / (_characterFrames.at(0)->width() + PARTY_FRAME_SPACING);

    // (Width of the scroll area minus the left margin) / (width of a frame plus spacing between frames)
    //return (ui->characterList->width() - PARTY_FRAME_SPACING) / (_characterFrames.at(0)->width() + PARTY_FRAME_SPACING);
    return (ui->graphicsView->width() - _labelGap) / (_labelSize + _labelGap);
}

MapReaderWorker::MapReaderWorker() :
    QObject(),
    _filenames(),
    _processing(false),
    _stopRequested(false),
    _pixmapSize(MAP_FRAME_SIZE)
{
    connect(this, &MapReaderWorker::processNext, this, &MapReaderWorker::processFilename);
}

void MapReaderWorker::addFilename(const QString& filename)
{
    if(!_filenames.contains(filename))
    {
        _stopRequested = false;
        _filenames.append(filename);
        if(!_processing)
            QTimer::singleShot(0, this, SLOT(processFilename()));
    }
}

void MapReaderWorker::stopProcessing()
{
    _stopRequested = true;
    if(!_processing)
        processingComplete();
}

void MapReaderWorker::setPixmapSize(int pixmapSize)
{
    _pixmapSize = pixmapSize;
}

void MapReaderWorker::processFilename()
{
    if((_filenames.isEmpty()) || (_stopRequested))
    {
        processingComplete();
        return;
    }

    _processing = true;
    QString filename = _filenames.takeLast();
    QPixmap pixmap;
    qDebug() << "[MapReaderWorkerThread] Loading image for: " << filename;
    if(pixmap.load(filename))
    {
        pixmap = pixmap.scaled(_pixmapSize, _pixmapSize, Qt::KeepAspectRatio);
        emit imageReady(filename, QString(), pixmap);
    }

    QTimer::singleShot(0, this, SLOT(processFilename()));
}

void MapReaderWorker::processingComplete()
{
    _processing = false;
    _filenames.clear();
    if(_stopRequested)
        emit processingStopped();
}

