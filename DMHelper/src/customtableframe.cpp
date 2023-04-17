#include "customtableframe.h"
#include "ui_customtableframe.h"
#include <QDir>
#include <QStringList>
#include <QDomDocument>
#include <QMessageBox>
#include <QDebug>

CustomTableFrame::CustomTableFrame(const QString& tableDirectory, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CustomTableFrame),
    _tableDirectory(tableDirectory),
    _timerId(0),
    _index(-1),
    _readTriggered(false),
    _directoryList(),
    _tableList(),
    _usedTables()
{
    ui->setupUi(this);
    ui->vSplitter->setStretchFactor(0, 1);
    ui->vSplitter->setStretchFactor(1, 5);
    ui->hSplitter->setStretchFactor(0, 1);
    ui->hSplitter->setStretchFactor(1, 3);

    ui->listWidget->setUniformItemSizes(true);
    //ui->listEntries->setUniformItemSizes(true);
    ui->listEntries->setWordWrap(true);
    ui->listEntries->setTextElideMode(Qt::ElideNone);

    connect(ui->listWidget, &QListWidget::currentItemChanged, this, &CustomTableFrame::tableItemChanged);
    connect(ui->btnReroll, &QAbstractButton::clicked, this, &CustomTableFrame::selectItem);
}

CustomTableFrame::~CustomTableFrame()
{
    if(_timerId)
        killTimer(_timerId);

    delete ui;
}

QSize CustomTableFrame::sizeHint() const
{
    return QSize(800, 600);
}

void CustomTableFrame::setTableDirectory(const QString& tableDir)
{
    if(tableDir == _tableDirectory)
        return;

    if(_timerId)
    {
        killTimer(_timerId);
        _timerId = 0;
    }

    ui->listWidget->clear();
    ui->listEntries->clear();
    ui->edtResult->clear();
    _directoryList.clear();
    _tableList.clear();

    _tableDirectory = tableDir;

    _index = -1;
    _readTriggered = false;
    if(isVisible())
        showEvent(nullptr);
}

void CustomTableFrame::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    if(_readTriggered)
        return;

    QDir tableDir(_tableDirectory);
    QStringList filters("*.xml");
    _directoryList = tableDir.entryList(QStringList("*.xml"));

    _readTriggered = true;

    if(_directoryList.count() > 0)
    {
        _index = 0;
        _timerId = startTimer(0);
    }
}

void CustomTableFrame::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if(_index >= _directoryList.count())
    {
        killTimer(_timerId);
        _timerId = 0;
    }
    else
    {
        readXMLFile(_directoryList.at(_index));
        ++_index;
    }
}

void CustomTableFrame::tableItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);

    if(!current)
        return;

    QString tableName = current->text();
    qDebug() << "[CustomTableFrame] Opening custom table name: " << tableName;

    if(!_tableList.contains(tableName))
    {
        qDebug() << "[CustomTableFrame] ERROR: Table or weight counting missing for table name: " << tableName;
        return;
    }

    CustomTable table = _tableList.value(tableName);
    ui->listEntries->clear();
    int weightCount = 0;
    int digitCount = QString::number(table.getTotalWeights()).size();
    QList<CustomTableEntry> tableEntries = table.getEntryList();
    for(int i = 0; i < tableEntries.count(); ++i)
    {
        int startingWeight = weightCount + 1;
        int endingWeight = weightCount + tableEntries.at(i).getWeight();

        QString entryText;
        if(startingWeight == endingWeight)
            entryText = QString("%1: ").arg(startingWeight, (2*digitCount) + 1, 10);
        else
            entryText = QString("%1-%2: ").arg(startingWeight, digitCount, 10).arg(endingWeight, digitCount, 10);

        entryText += tableEntries.at(i).getText();

        ui->listEntries->addItem(entryText);

        weightCount = endingWeight;
    }

    selectItem();
}

void CustomTableFrame::selectItem()
{
    if(!ui->listWidget->currentItem())
        return;

    _usedTables.clear();
    ui->edtResult->setPlainText(getEntryText(ui->listWidget->currentItem()->text()));
}

void CustomTableFrame::readXMLFile(const QString& fileName)
{
    QString fullFileName = _tableDirectory + QString("/") + fileName;
    qDebug() << "[CustomTableFrame] Reading custom table file name: " << fullFileName;

    QDomDocument doc("DMHelperDataXML");
    QFile file(fullFileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[CustomTableFrame] Opening custom table file failed.";
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString errMsg;
    int errRow;
    int errColumn;
    bool contentResult = doc.setContent(in.readAll(), &errMsg, &errRow, &errColumn);

    file.close();

    if( contentResult == false )
    {
        qDebug() << "[CustomTableFrame] Error reading custom table file XML content.";
        qDebug() << errMsg << errRow << errColumn;
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "dmhelperlist"))
    {
        qDebug() << "[CustomTableFrame] Custom table file missing dmhelperlist item";
        return;
    }

    CustomTable newTable(root, fullFileName);

    _tableList.insert(newTable.getName(), newTable);
    if(newTable.isVisible())
    {
        ui->listWidget->addItem(newTable.getName());
        ui->listWidget->sortItems();
    }
}

QString CustomTableFrame::getEntryText(const QString& tableName)
{
    QString result;

    if(_usedTables.contains(tableName))
    {
        QMessageBox::information(nullptr, QString("Looped subtables"), QString("Subtables were found to refer to each other, which would create an infinite loop."));
        return result;
    }

    if(!_tableList.contains(tableName))
    {
        qDebug() << "[CustomTableFrame] Table or weight counting missing for table name: " << tableName;
        return result;
    }

    CustomTable table = _tableList.value(tableName);
    CustomTableEntry entry = table.getRandomEntry();

    result = entry.getText();
    if(!entry.getSubtable().isEmpty())
    {
        _usedTables.append(tableName);
        result += QChar::LineFeed;
        result += getEntryText(entry.getSubtable());
    }

    return result;
}
