#include "texttranslatedialog.h"
#include "ui_texttranslatedialog.h"
#include "dice.h"
#include <QKeyEvent>
#include <QPainter>
#include <QIntValidator>
#include <QRandomGenerator>
#include <QFontDatabase>

TextTranslateDialog::TextTranslateDialog(const QString& originalText, const QString& translatedText, QImage backgroundImage, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextTranslateDialog),
    _backgroundImage(backgroundImage),
    _backgroundImageScaled()
{
    ui->setupUi(this);

    ui->edtTarget->setValidator(new QIntValidator(-999999, 999999, this));
    ui->edtRoll->setValidator(new QIntValidator(-999999, 999999, this));

    ui->txtOriginal->installEventFilter(this);
    ui->txtTranslated->installEventFilter(this);

    connect(ui->btnRolld20, &QAbstractButton::clicked, this, &TextTranslateDialog::rolld20);
    connect(ui->btnTranslate, &QAbstractButton::clicked, this, &TextTranslateDialog::translateText);

    ui->txtOriginal->setHtml(originalText);
    ui->txtTranslated->setHtml(translatedText);

    ui->btnColorOriginal->setRotationVisible(false);
    ui->btnColorTranslated->setRotationVisible(false);

    QFontDatabase fontDB;
    ui->cmbFontOriginal->addItem("Dwarvish");
    ui->cmbFontOriginal->addItem("Elvish");
    ui->cmbFontOriginal->addItem("Draconic");
    ui->cmbFontOriginal->insertSeparator(4);
    ui->cmbFontOriginal->addItems(fontDB.families());
    ui->cmbFontTranslated->addItem("Dwarvish");
    ui->cmbFontTranslated->addItem("Elvish");
    ui->cmbFontTranslated->addItem("Draconic");
    ui->cmbFontTranslated->insertSeparator(4);
    ui->cmbFontTranslated->addItems(fontDB.families());

    QTextDocument document;
    document.setHtml(originalText);
    QTextCursor cursor(&document);
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    QTextCharFormat format = cursor.charFormat();
    QFont sourceFont = format.font();
    ui->btnColorOriginal->setColor(format.foreground().color());
    ui->btnColorTranslated->setColor(format.foreground().color());
    ui->cmbFontOriginal->setCurrentText(sourceFont.family());
    ui->cmbFontTranslated->setCurrentText(sourceFont.family());
    ui->edtSizeOriginal->setText(QString::number(sourceFont.pointSize()));
    ui->edtSizeTranslated->setText(QString::number(sourceFont.pointSize()));
    ui->btnBoldOriginal->setChecked(sourceFont.weight() == QFont::Bold);
    ui->btnBoldTranslated->setChecked(sourceFont.weight() == QFont::Bold);
    ui->btnItalicsOriginal->setChecked(sourceFont.italic());
    ui->btnItalicsTranslated->setChecked(sourceFont.italic());
    ui->btnUnderlineOriginal->setChecked(format.fontUnderline());
    ui->btnUnderlineTranslated->setChecked(format.fontUnderline());
}

TextTranslateDialog::~TextTranslateDialog()
{
    delete ui;
}

bool TextTranslateDialog::eventFilter(QObject *watched, QEvent *event)
{
    if(((watched != ui->txtOriginal) && (watched != ui->txtTranslated)) ||
        (event->type() != QEvent::Paint) ||
        (_backgroundImageScaled.isNull()))
        return false;

    QPainter paint;
    if(watched == ui->txtOriginal)
        paint.begin(ui->txtOriginal);
    else
        paint.begin(ui->txtTranslated);

    paint.drawImage(0, 0, _backgroundImageScaled);
    paint.end();

    return false;
}

void TextTranslateDialog::getOriginalText(QString& originalText) const
{
    originalText = ui->txtOriginal->toHtml();
}

void TextTranslateDialog::getTranslatedText(QString& translatedText) const
{
    translatedText = ui->txtTranslated->toHtml();
}

void TextTranslateDialog::translateText()
{
    QString originalText = ui->txtOriginal->toPlainText();
    qreal difficultyValue = ui->edtTarget->text().toDouble();
    qreal rollValue = ui->edtRoll->text().toDouble();
    qreal probability = difficultyValue > 0 ? rollValue / difficultyValue : 1.0;

    /*

        QTextDocument document(_text);
        QTextCursor cursor(&document);

        cursor.select(QTextCursor::Document);

        QString fontFamily = element.attribute("fontFamily");
        QString fontSize = element.attribute("fontSize");
        QString fontBold = element.attribute("fontBold",QString::number(0));
        QString fontItalics = element.attribute("fontItalics",QString::number(0));

        QTextCharFormat format = cursor.charFormat();
        QFont formatFont(fontFamily,
                         fontSize.toInt(),
                         fontBold.toInt(),
                         static_cast<bool>(fontItalics.toInt()));
        format.setFont(formatFont);
        QString colorName = element.attribute("fontColor");
        if(QColor::isValidColor(colorName))
            format.setForeground(QBrush(QColor(colorName)));
        cursor.mergeCharFormat(format);

        QTextBlockFormat blockFormat = cursor.blockFormat();
        blockFormat.setAlignment(static_cast<Qt::Alignment>(element.attribute("alignment", "4").toInt()));
        cursor.mergeBlockFormat(blockFormat);

        cursor.clearSelection();
        setText(document.toHtml());

        QTextCharFormat format = ui->textBrowser->currentCharFormat();
        format.setFontUnderline(!format.fontUnderline());
        ui->textBrowser->setCurrentCharFormat(format);

      */

    QFont originalFont = getSelectedFont(ui->cmbFontOriginal->currentText(),
                                         ui->edtSizeOriginal->text().toInt(),
                                         ui->btnBoldOriginal->isChecked() ? QFont::Bold : QFont::Normal,
                                         ui->btnItalicsOriginal->isChecked());

    QFont translatedFont = getSelectedFont(ui->cmbFontTranslated->currentText(),
                                           ui->edtSizeTranslated->text().toInt(),
                                           ui->btnBoldTranslated->isChecked() ? QFont::Bold : QFont::Normal,
                                           ui->btnItalicsTranslated->isChecked());

    QTextDocument document;
    document.setHtml(originalText);
    QTextCursor cursor(&document);

    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QTextCharFormat format = cursor.charFormat();
    format.setFont(originalFont);
    cursor.mergeCharFormat(format);

    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    while(!cursor.atEnd())
    {
        qreal randomValue = QRandomGenerator::global()->generateDouble();
        if(randomValue > probability)
        {
            QString selectedText = cursor.selectedText();
            for(int i = 0; i < selectedText.length(); ++i)
            {
                if(selectedText.at(i).isLetter())
                {
                    QChar newChar('a' + static_cast<char>(QRandomGenerator::global()->bounded(26)));
                    selectedText[i] = selectedText.at(i).isLower() ? newChar : newChar.toUpper();
                }
            }
            if(ui->grpTranslated->isChecked())
            {
                QTextCharFormat format = cursor.charFormat();
                format.setFont(translatedFont);
                format.setFontUnderline(ui->btnUnderlineTranslated->isChecked());
                format.setForeground(QBrush(ui->btnColorTranslated->getColor()));
                cursor.insertText(selectedText, format);
            }
            else
            {
                cursor.insertText(selectedText);
            }
        }
        else if(ui->grpOriginal->isChecked())
        {
            QTextCharFormat format = cursor.charFormat();
            format.setFont(originalFont);
            format.setFontUnderline(ui->btnUnderlineOriginal->isChecked());
            format.setForeground(QBrush(ui->btnColorOriginal->getColor()));
            cursor.mergeCharFormat(format);
        }

        cursor.movePosition(QTextCursor::NextWord, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    }

    ui->txtTranslated->setHtml(document.toHtml());

    /*
    QStringList originalWords = originalText.split(QChar(' '));
    for(int w = 0; w < originalWords.count(); ++w)
    {
        cursor->movePosition(QTextCursor::NextWord, QTextCursor::MoveAnchor);
        cursor->movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        QString word = originalWords.at(w);
        qreal randomValue = QRandomGenerator::global()->generateDouble();
        if(randomValue > probability)
        {
            for(int i = 0; i < word.length(); ++i)
            {
                if(word.at(i).isLetter())
                {
                    QChar newChar('a' + static_cast<char>(QRandomGenerator::global()->bounded(26)));
                    word[i] = word.at(i).isLower() ? newChar : newChar.toUpper();
                }
            }
            QTextDocument document(word);
            QTextCursor cursor(&document);
            cursor.select(QTextCursor::Document);

            QTextCharFormat format = cursor.charFormat();
            QFont formatFont(ui->cmbFontTranslated->currentText(),
                             ui->edtSizeTranslated->text().toInt(),
                             ui->btnBoldTranslated->isChecked() ? QFont::Bold : QFont::Normal,
                             ui->btnItalicsTranslated->isChecked());
            format.setFont(formatFont);
            format.setFontUnderline(ui->btnUnderlineTranslated);
            format.setForeground(QBrush(ui->btnColorTranslated->getColor()));
            cursor.mergeCharFormat(format);

            word = document.toHtml();
        }
        else
        {
            QTextDocument document(word);
            QTextCursor cursor(&document);
            cursor.select(QTextCursor::Document);

            QTextCharFormat format = cursor.charFormat();
            QFont formatFont(ui->cmbFontOriginal->currentText(),
                             ui->edtSizeOriginal->text().toInt(),
                             ui->btnBoldOriginal->isChecked() ? QFont::Bold : QFont::Normal,
                             ui->btnItalicsOriginal->isChecked());
            format.setFont(formatFont);
            format.setFontUnderline(ui->btnUnderlineOriginal);
            format.setForeground(QBrush(ui->btnColorOriginal->getColor()));
            cursor.mergeCharFormat(format);

            word = document.toHtml();
        }
        originalWords[w] = word;
    }

    ui->txtTranslated->setHtml(originalWords.join(QChar(' ')));
    */
}

/*
void TextTranslateDialog::publishTextImage()
{
    if(!ui->txtTranslated->document())
        return;

    QImage pub(ui->txtTranslated->document()->size().toSize(), QImage::Format_ARGB32);
    pub.fill(Qt::white);
    QPainter painter(&pub);
    ui->txtTranslated->document()->drawContents(&painter);

    emit publishImage(pub, Qt::white);
}
*/

void TextTranslateDialog::keyPressEvent(QKeyEvent * event)
{
    if(event->key() == Qt::Key_Escape)
        hide();
    else
        QDialog::keyPressEvent(event);
}

void TextTranslateDialog::hideEvent(QHideEvent * event)
{
    Q_UNUSED(event);
    deleteLater();
}

void TextTranslateDialog::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    //ui->grpOriginal->setMinimumHeight(ui->btnColorOriginal->height() * 3);
    //ui->grpTranslated->setMinimumHeight(ui->btnColorOriginal->height() * 3);

    if(!_backgroundImage.isNull())
        _backgroundImageScaled = _backgroundImage.scaledToWidth(ui->txtOriginal->width(), Qt::SmoothTransformation);
}

void TextTranslateDialog::rolld20()
{
    ui->edtRoll->setText(QString::number(Dice::d20()));
}

QFont TextTranslateDialog::getSelectedFont(const QString& fontName, int pointSize, int weight, bool italic)
{
    QString fontFamily = fontName;
    if(fontFamily == "Dwarvish")
    {
        int id = QFontDatabase::addApplicationFont(":/img/data/fonts/Davek-vGXA.ttf");
        fontFamily = QFontDatabase::applicationFontFamilies(id).at(0);
    }
    else if(fontFamily == "Draconic")
    {
        int id = QFontDatabase::addApplicationFont(":/img/data/fonts/Iokharic-dqvK.ttf");
        fontFamily = QFontDatabase::applicationFontFamilies(id).at(0);
    }
    else if(fontFamily == "Elvish")
    {
        int id = QFontDatabase::addApplicationFont(":/img/data/fonts/Rellanic-Agx7.ttf");
        fontFamily = QFontDatabase::applicationFontFamilies(id).at(0);
    }

    return QFont(fontFamily, pointSize, weight, italic);
}
