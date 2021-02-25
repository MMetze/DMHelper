#include "ribbonframetext.h"
#include "ui_ribbonframetext.h"
#include <QFontDatabase>
#include <QAbstractItemView>

RibbonFrameText::RibbonFrameText(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonFrameText)
{
    ui->setupUi(this);

    ui->btnColor->setRotationVisible(false);
    connect(ui->btnColor, SIGNAL(colorChanged(QColor)), this, SIGNAL(colorChanged(QColor)));

    QFontDatabase fontDB;
    ui->cmbFont->addItems(fontDB.families());
    ui->cmbFont->view()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    connect(ui->cmbFont, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(fontFamilyChanged(QString)));

    ui->edtSize->setValidator(new QIntValidator(1,999,this));
    connect(ui->edtSize, SIGNAL(editingFinished()), this, SLOT(handleFontSizeChanged()));

    connect(ui->btnBold, SIGNAL(toggled(bool)), this, SIGNAL(fontBoldChanged(bool)));
    connect(ui->btnItalics, SIGNAL(toggled(bool)), this, SIGNAL(fontItalicsChanged(bool)));
    connect(ui->btnUnderline, SIGNAL(toggled(bool)), this, SIGNAL(fontUnderlineChanged(bool)));

    ui->btnGrpAlignment->setId(ui->btnLeftText, Qt::AlignLeft);
    ui->btnGrpAlignment->setId(ui->btnCenterText, Qt::AlignHCenter);
    ui->btnGrpAlignment->setId(ui->btnRightText, Qt::AlignRight);
    connect(ui->btnGrpAlignment, SIGNAL(buttonClicked(int)), this, SLOT(handleAlignmentChanged(int)));
}

RibbonFrameText::~RibbonFrameText()
{
    delete ui;
}

PublishButtonRibbon* RibbonFrameText::getPublishRibbon()
{
    return nullptr;
}

void RibbonFrameText::setColor(QColor color)
{
    ui->btnColor->setColor(color); // color button takes care of the signal and duplicates
}

void RibbonFrameText::setFontFamily(const QString& fontFamily)
{
    if(fontFamily != ui->cmbFont->currentText())
    {
        int familyIndex = ui->cmbFont->findText(fontFamily);
        if(familyIndex != -1)
            ui->cmbFont->setCurrentIndex(familyIndex); // triggers the signal
    }
}

void RibbonFrameText::setFontSize(int fontSize)
{
    if(fontSize != ui->edtSize->text().toInt())
        ui->edtSize->setText(QString::number(fontSize)); // triggers the signal
}

void RibbonFrameText::setFontBold(bool fontBold)
{
    if(fontBold != ui->btnBold->isChecked())
        ui->btnBold->setChecked(fontBold); // triggers the toggled signal
}

void RibbonFrameText::setFontItalics(bool fontItalics)
{
    if(fontItalics != ui->btnItalics->isChecked())
        ui->btnItalics->setChecked(fontItalics); // triggers the toggled signal
}

void RibbonFrameText::setFontUnderline(bool fontUnderline)
{
    if(fontUnderline != ui->btnUnderline->isChecked())
        ui->btnUnderline->setChecked(fontUnderline);
}

void RibbonFrameText::setAlignment(Qt::Alignment alignment)
{
    switch(alignment)
    {
        case Qt::AlignLeft:
            if(ui->btnLeftText->isChecked())
                return;
            ui->btnLeftText->setChecked(true);
            break;
        case Qt::AlignHCenter:
            if(ui->btnCenterText->isChecked())
                return;
            ui->btnCenterText->setChecked(true);
            break;
        case Qt::AlignRight:
            if(ui->btnRightText->isChecked())
                return;
            ui->btnRightText->setChecked(true);
            break;
        default:
            return;
    }

    emit alignmentChanged(alignment); // Set checked calls don't trigger a signal
}

void RibbonFrameText::handleFontSizeChanged()
{
    bool ok = false;
    int intSize = ui->edtSize->text().toInt(&ok);
    if(ok)
        emit fontSizeChanged(intSize);
}

void RibbonFrameText::handleAlignmentChanged(int id)
{
    Qt::Alignment alignment = Qt::AlignHCenter;

    switch(id)
    {
        case Qt::AlignLeft:
        case Qt::AlignHCenter:
        case Qt::AlignRight:
            alignment = static_cast<Qt::Alignment>(id);
            break;
        default:
            break;
    }

    emit alignmentChanged(alignment);
}

void RibbonFrameText::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    QFontMetrics metrics = ui->lblFont->fontMetrics();
    int labelHeight = getLabelHeight(metrics, height());
    int iconSize = height() - labelHeight;
    int halfHeight = (iconSize - ui->layoutFont->spacing()) / 2;

    setWidgetSize(*ui->edtSize, metrics.horizontalAdvance(QChar('0')) * 4, halfHeight);
    setButtonSize(*ui->btnBold, halfHeight, halfHeight);
    setButtonSize(*ui->btnItalics, halfHeight, halfHeight);

    setLineHeight(*ui->line_5, halfHeight);

    setButtonSize(*ui->btnColor, halfHeight, halfHeight);

    setLineHeight(*ui->line_4, halfHeight);

    setButtonSize(*ui->btnLeftText, halfHeight, halfHeight);
    setButtonSize(*ui->btnCenterText, halfHeight, halfHeight);
    setButtonSize(*ui->btnRightText, halfHeight, halfHeight);

    setWidgetSize(*ui->cmbFont,
                  ui->btnBold->width() + ui->btnItalics->width() + ui->line_5->width() + ui->btnColor->width() + ui->line_4->width() +
                      ui->btnLeftText->width() + ui->btnCenterText->width() + ui->btnRightText->width() - ui->edtSize->width(),
                  halfHeight);

    setWidgetSize(*ui->lblFont, ui->cmbFont->width() + ui->edtSize->width(), labelHeight);
}
