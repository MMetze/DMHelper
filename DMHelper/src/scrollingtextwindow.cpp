#include "scrollingtextwindow.h"
#include "ui_scrollingtextwindow.h"
#include "encounterscrollingtext.h"
#include <QMessageBox>
#include <QDebug>
#include <QPainter>

const int SCROLLING_TIMER_PERIOD = 30;

ScrollingTextWindow::ScrollingTextWindow(const EncounterScrollingText& encounter, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ScrollingTextWindow),
    _encounter(encounter),
    _targetRect(),
    _textPos(),
    _previousHeight(-1),
    _timerId(-1),
    _elapsed(),
    _rawBackground(),
    _scaledBackground(),
    _textImage()
{
    ui->setupUi(this);
    connect(ui->btnClose,SIGNAL(clicked(bool)),this,SLOT(close()));

    prepareScene();

    connect(ui->btnPlay, SIGNAL(toggled(bool)), this, SLOT(togglePlay(bool)));
    connect(ui->btnRewind, SIGNAL(clicked(bool)), this, SLOT(rewind()));
}

ScrollingTextWindow::~ScrollingTextWindow()
{
    delete ui;
}

void ScrollingTextWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    prepareImages();
    drawScene();
}

void ScrollingTextWindow::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    prepareImages();
    drawScene();
}

void ScrollingTextWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    qreal elapsedtime = _elapsed.restart();
    _textPos.ry() -= _encounter.getScrollSpeed() * elapsedtime / 1000.0;
    drawScene();
}

void ScrollingTextWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    if(_timerId != -1)
    {
        killTimer(_timerId);
        _timerId = -1;
    }

    deleteLater();
}

void ScrollingTextWindow::rewind()
{
    _textPos.setY(static_cast<qreal>(_scaledBackground.height()));
    drawScene();
}

void ScrollingTextWindow::togglePlay(bool checked)
{
    if(checked)
    {
        _elapsed.start();
        _timerId = startTimer(SCROLLING_TIMER_PERIOD, Qt::PreciseTimer);
    }
    else if(_timerId != -1)
    {
        killTimer(_timerId);
        _timerId = -1;
    }
}

void ScrollingTextWindow::prepareScene()
{
    _rawBackground = QImage();
    QString imageFileName = _encounter.getImgFile();
    if(!imageFileName.isEmpty())
    {
        if(!_rawBackground.load(_encounter.getImgFile()))
        {
            QMessageBox::critical(this, QString("Invalid image file"), QString("[ScrollingTextWindow] Image file could not be loaded"), QMessageBox::Ok);
            qDebug() << "[ScrollingTextWindow] Image file could not be loaded";
        }
    }
}

void ScrollingTextWindow::prepareImages()
{
    QRect labelRect = ui->lblImage->rect();
    labelRect.setWidth(labelRect.width() - (ui->lblImage->frameWidth() * 2));
    labelRect.setHeight(labelRect.height() - (ui->lblImage->frameWidth() * 2));
    _scaledBackground = QPixmap::fromImage(_rawBackground.scaled(labelRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QFont font(_encounter.getFontFamily(),
               _encounter.getFontSize(),
               _encounter.getFontBold() ? QFont::Bold : QFont::Normal,
               _encounter.getFontItalics());

    _targetRect = _scaledBackground.isNull() ? labelRect : _scaledBackground.rect();
    QSize targetSize = _scaledBackground.isNull() ? labelRect.size() : _scaledBackground.size();

    int textWidth = _targetRect.width() * _encounter.getImageWidth() / 100;
    _targetRect.setWidth(textWidth);
    _targetRect.setX((_targetRect.width() - textWidth) / 2);
    _textPos.setX(static_cast<qreal>(targetSize.width() - textWidth) / 2.0);
    if(_previousHeight <= 0)
        _textPos.setY(static_cast<qreal>(targetSize.height()));
    else
        _textPos.setY((_textPos.y() / static_cast<qreal>(_previousHeight)) * static_cast<qreal>(targetSize.height()));
    _previousHeight = targetSize.height();

    QFontMetrics fontMetrics(font);
    QRect boundingRect = fontMetrics.boundingRect(_targetRect, (Qt::AlignmentFlag)_encounter.getAlignment() | Qt::TextWordWrap, _encounter.getText());
    _targetRect.setHeight(boundingRect.height());

    _textImage = QPixmap(_targetRect.size());
    _textImage.fill(Qt::transparent);
    QPainter painter(&_textImage);
    painter.setFont(font);
    painter.setPen(_encounter.getFontColor());
    painter.drawText(_targetRect, (Qt::AlignmentFlag)_encounter.getAlignment() | Qt::TextWordWrap, _encounter.getText());
}

void ScrollingTextWindow::drawScene()
{
    QPixmap targetPix = _scaledBackground;
    QPainter painter(&targetPix);
    painter.drawPixmap(_textPos.toPoint(), _textImage, _textImage.rect());
    ui->lblImage->setPixmap(targetPix);
}
