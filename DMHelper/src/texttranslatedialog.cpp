#include "texttranslatedialog.h"
#include "ui_texttranslatedialog.h"
#include <QKeyEvent>
#include <QPainter>
#include <QIntValidator>
#include <QRandomGenerator>

TextTranslateDialog::TextTranslateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextTranslateDialog)
{
    ui->setupUi(this);

    ui->edtTarget->setValidator(new QIntValidator(-10,100,this));
    ui->edtRoll->setValidator(new QIntValidator(-10,100,this));

    connect(ui->btnTranslate, SIGNAL(clicked()), this, SLOT(translateText()));
    connect(ui->btnPublish, SIGNAL(clicked()), this, SLOT(publishTextImage()));
}

TextTranslateDialog::~TextTranslateDialog()
{
    delete ui;
}

void TextTranslateDialog::translateText()
{
    QString originalText = ui->txtOriginal->toPlainText();
    qreal difficultyValue = ui->edtTarget->text().toDouble();
    qreal rollValue = ui->edtRoll->text().toDouble();
    qreal probability = difficultyValue > 0 ? rollValue / difficultyValue : 1.0;

    QStringList originalWords = originalText.split(QChar(' '));
    for(int w = 0; w < originalWords.count(); ++w)
    {
        qreal randomValue = QRandomGenerator::global()->generateDouble();
        if(randomValue > probability)
        {
            QString word = originalWords.at(w);
            for(int i = 0; i < word.length(); ++i)
            {
                if(word.at(i).isLetter())
                {
                    QChar newChar('a' + static_cast<char>(QRandomGenerator::global()->bounded(26)));
                    word[i] = word.at(i).isLower() ? newChar : newChar.toUpper();
                }
            }
            originalWords[w] = word;
        }
    }

    ui->txtTranslated->setPlainText(originalWords.join(QChar(' ')));
}

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

void TextTranslateDialog::keyPressEvent(QKeyEvent * event)
{
    if( event->key() == Qt::Key_Escape )
    {
        hide();
    }
    else
    {
        QDialog::keyPressEvent(event);
    }
}

void TextTranslateDialog::hideEvent(QHideEvent * event)
{
    Q_UNUSED(event);
    deleteLater();
}
