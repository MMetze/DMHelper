#include "customtableframe.h"
#include "dice.h"
#include "ui_customtableframe.h"
#include <QDir>
#include <QStringList>
#include <QDomDocument>
#include <QDebug>

CustomTableFrame::CustomTableFrame(const QString& tableDirectory, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CustomTableFrame),
    _tableDirectory(tableDirectory),
    _timerId(0),
    _index(-1),
    _readTriggered(false),
    _directoryList(),
    _tableList()
{
    ui->setupUi(this);

    ui->listWidget->setUniformItemSizes(true);
    ui->listEntries->setUniformItemSizes(true);

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

    //QDir tableDir(QString("./tables"));
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

    QStringList tableEntries = _tableList.value(tableName);
    ui->listEntries->clear();
    ui->listEntries->addItems(tableEntries);

    selectItem();
}

void CustomTableFrame::selectItem()
{
    int rollIndex = Dice::dX(ui->listEntries->count()) - 1;

    QListWidgetItem* rolledValue = ui->listEntries->item(rollIndex);
    if(rolledValue)
        ui->edtResult->setPlainText(rolledValue->text());
}

void CustomTableFrame::readXMLFile(const QString& fileName)
{
    // QString fullFileName = QString("./tables/") + fileName;
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

    QString tableName = root.attribute(QString("name"));
    if(tableName.isEmpty())
    {
        qDebug() << "[CustomTableFrame] Custom table file dmhelperlist item missing name attribute";
        return;
    }

    QStringList entryList;
    QDomElement element = root.firstChildElement(QString("entry"));
    while(!element.isNull())
    {
        entryList.append(element.text());
        element = element.nextSiblingElement(QString("entry"));
    }

    _tableList.insert(tableName, entryList);
    ui->listWidget->addItem(tableName);
    ui->listWidget->sortItems();
}
