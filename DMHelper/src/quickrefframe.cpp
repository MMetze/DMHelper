#include "quickrefframe.h"
#include "ui_quickrefframe.h"
#include "quickref.h"
#include "quickrefdatawidget.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

QuickRefFrame::QuickRefFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::QuickRefFrame),
    _quickRef(),
    _quickRefLayout(nullptr)

{
    ui->setupUi(this);

    // Load the quick-reference guide in the main window
    _quickRefLayout = new QVBoxLayout;
    _quickRefLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    //_quickRefLayout->setContentsMargins(1,1,1,1);
    ui->scrollQuickRefWidgetContents->setLayout(_quickRefLayout);
    //ui->scrollQuickRefWidgetContents->setContentsMargins(1,1,1,1);
    //ui->scrollQuickRef->setContentsMargins(1,1,1,1);
    connect(ui->cmbQuickRef, SIGNAL(currentIndexChanged(int)), this, SLOT(handleQuickRefChange(int)));
    readQuickRef();
}

QuickRefFrame::~QuickRefFrame()
{
    qDeleteAll(_quickRef);
    delete ui;
}

void QuickRefFrame::readQuickRef()
{
    if(!_quickRef.empty())
        return;

    ui->cmbQuickRef->clear();

    QString quickRefFileName("quickref_data.xml");

    QDomDocument doc("DMHelperDataXML");
    QFile file(quickRefFileName);
    qDebug() << "[QuickRef] Quickref data file: " << QFileInfo(file).filePath();
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[QuickRef] Unable to read quickref file: " << quickRefFileName;
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString errMsg;
    int errRow;
    int errColumn;
    bool contentResult = doc.setContent(in.readAll(), &errMsg, &errRow, &errColumn);

    file.close();

    if(contentResult == false)
    {
        qDebug() << "[QuickRef] Unable to parse the quickref data file.";
        qDebug() << errMsg << errRow << errColumn;
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "root"))
    {
        qDebug() << "[QuickRef] Unable to find the root element in the quickref data file.";
        return;
    }

    QDomElement sectionElement = root.firstChildElement(QString("section"));
    while(!sectionElement.isNull())
    {
        QuickRefSection* newSection = new QuickRefSection(sectionElement);
        _quickRef.append(newSection);
        ui->cmbQuickRef->addItem(newSection->getName());
        sectionElement = sectionElement.nextSiblingElement(QString("section"));
    }

    if(!_quickRef.empty())
    {
        ui->cmbQuickRef->setCurrentIndex(0);
    }
}

void QuickRefFrame::handleQuickRefChange(int selection)
{
    if((selection < 0) || (selection >= _quickRef.count()))
        return;

    // Delete existing widgets
    QLayoutItem *child;
    while ((child = _quickRefLayout->takeAt(0)) != nullptr)
    {
      child->widget()->deleteLater();
      delete child;
    }

    // Add a new widget for each quickref item
    QuickRefSection* section = _quickRef.at(selection);
    if(section->getLimitation().isEmpty())
    {
        ui->lblQuickRefSectionLimitation->hide();
    }
    else
    {
        ui->lblQuickRefSectionLimitation->show();
        ui->lblQuickRefSectionLimitation->setText(section->getLimitation());
    }

    //use section name and limitation
    QList<QuickRefSubsection> subsections = section->getSubsections();
    for(int i = 0; i < subsections.count(); ++i)
    {
        if(!subsections.at(i).getDescription().isEmpty())
        {
            QLabel* lblSubSection = new QLabel(subsections.at(i).getDescription(), ui->scrollQuickRefWidgetContents);
            lblSubSection->setWordWrap(true);
            lblSubSection->setMargin(6);
            _quickRefLayout->addWidget(lblSubSection);
        }

        //use subsection description
        QList<QuickRefData> data = subsections.at(i).getData();
        for(int j = 0; j < data.count(); ++j)
        {
            QuickRefDataWidget* widget = new QuickRefDataWidget(data.at(j), ui->scrollQuickRefWidgetContents);
            _quickRefLayout->addWidget(widget);
        }
    }

    updateGeometry();
}
