#include "encounterscrollingtextedit.h"
#include "ui_encounterscrollingtextedit.h"
#include "encounterscrollingtext.h"
#include "scrollingtextwindow.h"
#include <QFontDatabase>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QTextCursor>
#include <QTextBlockFormat>
#include <QMessageBox>
#include <QDebug>

EncounterScrollingTextEdit::EncounterScrollingTextEdit(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::EncounterScrollingTextEdit),
    _scrollingText(0),
    _textScene(0),
    _textItem(0),
    _backgroundWidth(0)
{
    ui->setupUi(this);

    connect(ui->btnBrowse,SIGNAL(clicked(bool)),this,SLOT(browseImageFile()));

    QFontDatabase fontDB;
    ui->cmbFont->addItems(fontDB.families());

    connect(ui->spinSpeed,SIGNAL(valueChanged(double)),this,SIGNAL(scrollSpeedChanged(double)));
    connect(ui->sliderWidth,SIGNAL(valueChanged(int)),this,SIGNAL(imageWidthChanged(int)));
    connect(ui->sliderWidth,SIGNAL(valueChanged(int)),this,SLOT(updatePreviewFont()));
    connect(ui->edtImageFile,SIGNAL(textChanged(QString)),this,SIGNAL(imgFileChanged(QString)));
    connect(ui->edtText,SIGNAL(textChanged()),this,SLOT(setPlainText()));
    connect(ui->cmbFont,SIGNAL(currentIndexChanged(QString)),this,SIGNAL(fontFamilyChanged(QString)));
    connect(ui->cmbFont,SIGNAL(currentIndexChanged(QString)),this,SLOT(setTextFont()));

    ui->edtSize->setValidator(new QIntValidator(1,999,this));
    connect(ui->edtSize,SIGNAL(textChanged(QString)),this,SLOT(setFontSize()));

    connect(ui->chkBold,SIGNAL(clicked(bool)),this,SIGNAL(fontBoldChanged(bool)));
    connect(ui->chkBold,SIGNAL(clicked(bool)),this,SLOT(setTextFont()));
    connect(ui->chkItalics,SIGNAL(clicked(bool)),this,SIGNAL(fontItalicsChanged(bool)));
    connect(ui->chkItalics,SIGNAL(clicked(bool)),this,SLOT(setTextFont()));

    ui->buttonGroup->setId(ui->btnLeftText,Qt::AlignLeft);
    ui->buttonGroup->setId(ui->btnCenterText,Qt::AlignHCenter);
    ui->buttonGroup->setId(ui->btnRightText,Qt::AlignRight);
    connect(ui->buttonGroup,SIGNAL(buttonClicked(int)),this,SLOT(setAlignment()));

    ui->edtR->setValidator(new QIntValidator(0,255,this));
    ui->edtG->setValidator(new QIntValidator(0,255,this));
    ui->edtB->setValidator(new QIntValidator(0,255,this));

    connect(ui->edtR,SIGNAL(textChanged(QString)),this,SLOT(setColor()));
    connect(ui->edtG,SIGNAL(textChanged(QString)),this,SLOT(setColor()));
    connect(ui->edtB,SIGNAL(textChanged(QString)),this,SLOT(setColor()));

    connect(this,SIGNAL(textChanged(const QString&)),this,SLOT(updatePreviewText(QString)));
    connect(this,SIGNAL(imgFileChanged(const QString&)),this,SLOT(createScene()));

    connect(ui->btnAnimate,SIGNAL(clicked(bool)),this,SLOT(runAnimation()));
}

EncounterScrollingTextEdit::~EncounterScrollingTextEdit()
{
    delete ui;
}

EncounterScrollingText* EncounterScrollingTextEdit::getScrollingText() const
{
    return _scrollingText;
}

void EncounterScrollingTextEdit::setScrollingText(EncounterScrollingText* scrollingText)
{
    if(!scrollingText)
        return;

    _scrollingText = scrollingText;
    ui->spinSpeed->setValue(_scrollingText->getScrollSpeed()); // TODO: what if invalid?
    ui->sliderWidth->setValue(_scrollingText->getImageWidth());
    ui->edtImageFile->setText(_scrollingText->getImgFile());
    ui->edtText->setPlainText(_scrollingText->getText());
    ui->cmbFont->setCurrentIndex(ui->cmbFont->findText(_scrollingText->getFontFamily()));
    ui->edtSize->setText(QString::number(_scrollingText->getFontSize()));
    ui->chkBold->setChecked(_scrollingText->getFontBold());
    ui->chkItalics->setChecked(_scrollingText->getFontItalics());
    setTextFont();
    ui->btnLeftText->setChecked(scrollingText->getAlignment() == Qt::AlignLeft);
    ui->btnCenterText->setChecked(scrollingText->getAlignment() == Qt::AlignHCenter);
    ui->btnRightText->setChecked(scrollingText->getAlignment() == Qt::AlignRight);

    setAlignment();

    ui->edtR->setText(QString::number(_scrollingText->getFontColor().red()));
    ui->edtG->setText(QString::number(_scrollingText->getFontColor().green()));
    ui->edtB->setText(QString::number(_scrollingText->getFontColor().blue()));
    setColor();
}

void EncounterScrollingTextEdit::unsetScrollingText(EncounterScrollingText* scrollingText)
{
    Q_UNUSED(scrollingText);
    _scrollingText = 0;
}

void EncounterScrollingTextEdit::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if(_textScene)
    {
        ui->graphicsView->fitInView(_textScene->sceneRect(), Qt::KeepAspectRatio);
    }
}

void EncounterScrollingTextEdit::setPlainText()
{
    emit textChanged(ui->edtText->toPlainText());
}

void EncounterScrollingTextEdit::setFontSize()
{
    emit fontSizeChanged(ui->edtSize->text().toInt());
    setTextFont();
}

void EncounterScrollingTextEdit::setAlignment()
{
    Qt::AlignmentFlag newAlignment = getAlignment();

    ui->edtText->selectAll();
    ui->edtText->setAlignment(newAlignment);
    QTextCursor cursor(ui->edtText->textCursor());
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    ui->edtText->setTextCursor(cursor);

    updatePreviewFont();
    emit alignmentChanged(newAlignment);
}

void EncounterScrollingTextEdit::setColor()
{
    QString colorString = "background-image: url(); background-color: rgb(";
    colorString.append(ui->edtR->text());
    colorString.append(",");
    colorString.append(ui->edtG->text());
    colorString.append(",");
    colorString.append(ui->edtB->text());
    colorString.append(");");
    ui->frameColor->setStyleSheet(colorString);

    QColor newColor(ui->edtR->text().toInt(), ui->edtG->text().toInt(), ui->edtB->text().toInt());
    emit colorChanged(newColor);

    // Set the text color for all text then move the cursor to the end
    ui->edtText->selectAll();
    ui->edtText->setTextColor(newColor);
    QTextCursor cursor(ui->edtText->textCursor());
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    ui->edtText->setTextCursor(cursor);

    updatePreviewFont();
}

void EncounterScrollingTextEdit::setTextFont()
{
    ui->edtText->selectAll();
    ui->edtText->setFontFamily(ui->cmbFont->currentText());
    ui->edtText->setFontItalic(ui->chkItalics->isChecked());
    ui->edtText->setFontWeight(ui->chkBold->isChecked() ? QFont::Bold : QFont::Normal);
    ui->edtText->setFontPointSize(ui->edtSize->text().toInt());

    // Move the cursor to the end of the text
    QTextCursor cursor(ui->edtText->textCursor());
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    ui->edtText->setTextCursor(cursor);

    updatePreviewFont();
}

void EncounterScrollingTextEdit::browseImageFile()
{
    QString imageFileName = QFileDialog::getOpenFileName(this,QString("Select Image File"));

    if((imageFileName.isEmpty()) || (!QFile::exists(imageFileName)))
    {
        qDebug() << "[ScrollingText] Invalid image file selected for animation";
        return;
    }

    ui->edtImageFile->setText(imageFileName);
    emit imgFileChanged(imageFileName);
}

void EncounterScrollingTextEdit::createScene()
{
    if(_textScene)
    {
        delete _textScene;
        _textScene = 0;
        _textItem = 0;
    }
    _textScene = new QGraphicsScene(this);
    ui->graphicsView->setScene(_textScene);

    QString imageFileName = ui->edtImageFile->text();
    if(!imageFileName.isEmpty())
    {
        QImage backgroundImg;
        if(!backgroundImg.load(ui->edtImageFile->text()))
        {
            QMessageBox::critical(this, QString("Invalid image file"), QString("[ScrollingText] Selected image file could not be loaded"), QMessageBox::Ok);
            qDebug() << "[ScrollingText] Selected image file could not be loaded";
        }
        else
        {
            QGraphicsPixmapItem* background = _textScene->addPixmap(QPixmap::fromImage(backgroundImg));
            background->setEnabled(false);
            background->setZValue(-2);
            _backgroundWidth = background->boundingRect().width();
            ui->graphicsView->fitInView(background);
        }
    }

    _textItem = _textScene->addText(ui->edtText->toPlainText(), ui->edtText->currentFont());
    int itemWidth = _backgroundWidth * ui->sliderWidth->value() / 100;
    _textItem->setTextWidth(itemWidth);
    _textItem->setPos((_backgroundWidth - itemWidth) / 2, 0);
}

void EncounterScrollingTextEdit::updatePreviewText(const QString& newText)
{
    if(!_textScene)
    {
        createScene();
    }
    else if(_textItem)
    {
        _textItem->setPlainText(newText);
        _textItem->update();
        ui->graphicsView->update();
    }
}

void EncounterScrollingTextEdit::updatePreviewFont()
{
    if(!_textScene)
    {
        createScene();
    }
    else if(_textItem)
    {
        // Set the preview font
        _textItem->setFont(ui->edtText->currentFont());

        // Set the preview text color
        _textItem->setDefaultTextColor(ui->edtText->textColor());

        // Set the preview alignment
        QTextBlockFormat format;
        format.setAlignment(getAlignment());
        QTextCursor cursor = _textItem->textCursor();
        cursor.select(QTextCursor::Document);
        cursor.mergeBlockFormat(format);
        cursor.clearSelection();
        _textItem->setTextCursor(cursor);

        // Set the preview position and width
        int itemWidth = _backgroundWidth * ui->sliderWidth->value() / 100;
        _textItem->setTextWidth(itemWidth);
        _textItem->setPos((_backgroundWidth - itemWidth) / 2, 0);

        // Update the scene
        _textItem->update();
        _textScene->update();
        ui->graphicsView->update();
    }
}

void EncounterScrollingTextEdit::runAnimation()
{
    if(_scrollingText)
    {
        ScrollingTextWindow *wnd = new ScrollingTextWindow(*_scrollingText, this);
        wnd->show();
    }
}

Qt::AlignmentFlag EncounterScrollingTextEdit::getAlignment()
{
    switch(ui->buttonGroup->checkedId())
    {
    case Qt::AlignLeft:
    case Qt::AlignHCenter:
    case Qt::AlignRight:
        return (Qt::AlignmentFlag)(ui->buttonGroup->checkedId());
    default:
        return Qt::AlignHCenter;
    }
}
