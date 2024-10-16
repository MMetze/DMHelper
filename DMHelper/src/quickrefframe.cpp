#include "quickrefframe.h"
#include "ui_quickrefframe.h"
#include "quickref.h"
#include "quickrefdatawidget.h"
#include <QTextStream>
#include <QCoreApplication>

QuickRefFrame::QuickRefFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::QuickRefFrame),
    _quickRefLayout(nullptr),
    _startSection()
{
    ui->setupUi(this);

    // Load the quick-reference guide in the main window
    _quickRefLayout = new QVBoxLayout;
    _quickRefLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ui->scrollQuickRefWidgetContents->setLayout(_quickRefLayout);

    connect(ui->cmbQuickRef, SIGNAL(currentIndexChanged(int)), this, SLOT(handleQuickRefChange(int)));
}

QuickRefFrame::~QuickRefFrame()
{
    delete ui;
}

void QuickRefFrame::refreshQuickRef()
{
    if(!QuickRef::Instance())
        return;

    ui->cmbQuickRef->clear();

    QStringList sectionNames = QuickRef::Instance()->getSectionList();
    for(const QString& section : std::as_const(sectionNames))
    {
        ui->cmbQuickRef->addItem(section);
    }

    if(QuickRef::Instance()->count() > 0)
    {
        if(_startSection.isEmpty())
        {
            ui->cmbQuickRef->setCurrentIndex(0);
        }
        else
        {
            if(ui->cmbQuickRef->findText(_startSection) != -1)
                ui->cmbQuickRef->setCurrentText(_startSection);

            _startSection = QString();
        }
    }
}

void QuickRefFrame::setQuickRefSection(const QString& sectionName)
{
    _startSection = sectionName;
}

void QuickRefFrame::handleQuickRefChange(int selection)
{
    if((!QuickRef::Instance()) || (selection < 0) || (selection >= QuickRef::Instance()->count()))
        return;

    if(ui->cmbQuickRef->currentText().isEmpty())
        return;

    // Delete existing widgets
    QLayoutItem *child;
    while ((child = _quickRefLayout->takeAt(0)) != nullptr)
    {
      child->widget()->deleteLater();
      delete child;
    }

    // Add a new widget for each quickref item
    QuickRefSection* section = QuickRef::Instance()->getSection(ui->cmbQuickRef->currentText());
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
    QList<QuickRefSubsection*> subsections = section->getSubsections();
    for(int i = 0; i < subsections.count(); ++i)
    {
        QuickRefSubsection* subsection = subsections.at(i);
        if(subsection)
        {
            if(!subsection->getDescription().isEmpty())
            {
                QLabel* lblSubSection = new QLabel(subsection->getDescription(), ui->scrollQuickRefWidgetContents);
                lblSubSection->setWordWrap(true);
                lblSubSection->setMargin(6);
                _quickRefLayout->addWidget(lblSubSection);
            }

            //use subsection description
            QStringList dataTitles = subsection->getDataTitles();
            for(const QString& dataTitle : std::as_const(dataTitles))
            {
                QuickRefData* data = subsection->getData(dataTitle);
                if(data)
                {
                    QuickRefDataWidget* widget = new QuickRefDataWidget(*data, ui->scrollQuickRefWidgetContents);
                    _quickRefLayout->addWidget(widget);
                }
            }
        }
    }

    updateGeometry();
}
