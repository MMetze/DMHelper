#include "bestiarypopulatetokensdialog.h"
#include "ui_bestiarypopulatetokensdialog.h"
#include "optionscontainer.h"
#include "tokeneditor.h"
#include "bestiary.h"
#include "monsterclass.h"
#include "dmhwaitingdialog.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QMessageBox>
#include <QFileDialog>
#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

//#define DEBUG_POPULATE_TOKENS

BestiaryPopulateTokensDialog::BestiaryPopulateTokensDialog(const OptionsContainer& options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BestiaryPopulateTokensDialog),
    _manager(nullptr),
    _activeReply(nullptr),
    _waitingDlg(nullptr),
    _editor(nullptr),
    _tokenDir(),
    _searchString(options.getTokenSearchString()),
    _currentMonster(),
    _monsterList(),
    _totalPopulated(0),
    _totalMonsters(0)
{
    ui->setupUi(this);

    ui->chkFill->setChecked(options.getTokenBackgroundFill());
    ui->btnFillColor->setColor(options.getTokenBackgroundFillColor());
    ui->btnFillColor->setRotationVisible(false);

    ui->chkTransparent->setChecked(options.getTokenTransparent());
    ui->btnTransparentColor->setColor(options.getTokenTransparentColor());
    ui->btnTransparentColor->setRotationVisible(false);
    ui->sliderFuzzy->setValue(options.getTokenTransparentLevel());

    ui->chkMask->setChecked(options.getTokenMaskApplied());
    ui->edtMaskImage->setText(options.getTokenMaskFile());
    ui->chkFrame->setChecked(options.getTokenFrameApplied());
    ui->edtFrameImage->setText(options.getTokenFrameFile());

    ui->frameToken->installEventFilter(this);

    _editor = new TokenEditor;
    _editor->applyOptionsToEditor(options);
    _editor->setSourceImage(QImage(":/img/data/icon_bestiary.png"));
    updateImage();

    connect(ui->chkFill, &QAbstractButton::toggled, _editor, &TokenEditor::setBackgroundFill);
    connect(ui->btnFillColor, &ColorPushButton::colorChanged, _editor, &TokenEditor::setBackgroundFillColor);
    connect(ui->chkTransparent, &QAbstractButton::toggled, _editor, &TokenEditor::setTransparent);
    connect(ui->btnTransparentColor, &ColorPushButton::colorChanged, _editor, &TokenEditor::setTransparentColor);
    connect(ui->sliderFuzzy, &QAbstractSlider::valueChanged, _editor, &TokenEditor::setTransparentLevel);
    connect(ui->chkMask, &QAbstractButton::toggled, _editor, &TokenEditor::setMaskApplied);
    connect(ui->edtMaskImage, &QLineEdit::textChanged, _editor, &TokenEditor::setMaskFile);
    connect(ui->btnBrowseMaskImage, &QAbstractButton::clicked, this, &BestiaryPopulateTokensDialog::browseMask);
    connect(ui->chkFrame, &QAbstractButton::toggled, _editor, &TokenEditor::setFrameApplied);
    connect(ui->edtFrameImage, &QLineEdit::textChanged, _editor, &TokenEditor::setFrameFile);
    connect(ui->btnBrowseFrameImage, &QAbstractButton::clicked, this, &BestiaryPopulateTokensDialog::browseFrame);

    /*
    connect(ui->chkFill, &QAbstractButton::toggled, this, &BestiaryPopulateTokensDialog::updateImage);
    connect(ui->btnFillColor, &ColorPushButton::colorChanged, this, &BestiaryPopulateTokensDialog::updateImage);
    connect(ui->chkTransparent, &QAbstractButton::toggled, this, &BestiaryPopulateTokensDialog::updateImage);
    connect(ui->btnTransparentColor, &ColorPushButton::colorChanged, this, &BestiaryPopulateTokensDialog::updateImage);
    connect(ui->sliderFuzzy, &QAbstractSlider::valueChanged, this, &BestiaryPopulateTokensDialog::updateImage);
    connect(ui->chkMask, &QAbstractButton::toggled, this, &BestiaryPopulateTokensDialog::updateImage);
    connect(ui->edtMaskImage, &QLineEdit::textChanged, this, &BestiaryPopulateTokensDialog::updateImage);
    connect(ui->chkFrame, &QAbstractButton::toggled, this, &BestiaryPopulateTokensDialog::updateImage);
    connect(ui->edtFrameImage, &QLineEdit::textChanged, this, &BestiaryPopulateTokensDialog::updateImage);
*/
    connect(_editor, &TokenEditor::imageDirty, this, &BestiaryPopulateTokensDialog::updateImage);

    connect(ui->btnPopulate, &QAbstractButton::clicked, this, &BestiaryPopulateTokensDialog::populateTokens);
}

BestiaryPopulateTokensDialog::~BestiaryPopulateTokensDialog()
{
    delete ui;
}

void BestiaryPopulateTokensDialog::populateTokens()
{
    if(Bestiary::Instance() == nullptr)
    {
        qDebug() << "[BestiaryPopulateTokensDialog] ERROR: Bestiary instance is null. Cannot populate tokens.";
        return;
    }

    // Check each monster to see if it has a token, keeping a list of those without a token
    _monsterList.clear();
    QStringList fullMonsterList = Bestiary::Instance()->getMonsterList();
    for(const QString& monsterName : fullMonsterList)
    {
        MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(monsterName);
        if((monsterClass) && (monsterClass->getIconCount() == 0))
            _monsterList.append(monsterName);
    }

    if(_monsterList.isEmpty())
    {
        qDebug() << "[BestiaryPopulateTokensDialog] No monsters found missing tokens - no need to populate tokens.";
        QMessageBox::information(this, tr("No Tokens to Populate"), tr("All monsters already have tokens! No need to populate tokens."));
        return;
    }

    QString tokenPath = QFileDialog::getExistingDirectory(this, tr("Select Token Directory"), Bestiary::Instance()->getDirectory().absolutePath());
    if(tokenPath.isEmpty())
    {
        qDebug() << "[BestiaryPopulateTokensDialog] ERROR: Bestiary token population cancelled by user.";
        return;
    }

    _tokenDir.setPath(tokenPath);

    _waitingDlg = new DMHWaitingDialog(QString("Populating Bestiary tokens..."), this);
    _waitingDlg->setModal(true);
    _waitingDlg->show();

    _totalPopulated = 0;
    _totalMonsters = _monsterList.size();

    if(!_manager)
        _manager = new QNetworkAccessManager(this);

    checkNextMonster();
}

void BestiaryPopulateTokensDialog::urlRequestFinished(QNetworkReply *reply)
{
    disconnect(_manager, &QNetworkAccessManager::finished, this, &BestiaryPopulateTokensDialog::urlRequestFinished);
    _activeReply = nullptr;

    if(!reply)
        return;

    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "[BestiaryPopulateTokensDialog] ERROR: network image URL reply not ok: " << reply->error();
        qDebug() << "[BestiaryPopulateTokensDialog] ERROR: " << reply->errorString();

        QMessageBox::critical(this, tr("Token Search Error"), tr("Error encountered retrieving link to token for ") + _currentMonster + tr(": ") + reply->errorString());

        reply->deleteLater();
        return;
    }

    QByteArray bytes = reply->readAll();
#ifdef DEBUG_POPULATE_TOKENS
    qDebug() << "[BestiaryPopulateTokensDialog] Token Search request received; payload " << bytes.size() << " bytes";
    qDebug() << "[BestiaryPopulateTokensDialog] Payload contents: " << QString(bytes.left(2000));
#endif

    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if(!doc.setContent(bytes, &errorMsg, &errorLine, &errorColumn))
    {
        qDebug() << "[BestiaryPopulateTokensDialog] ERROR identified reading data: unable to parse network reply XML at line " << errorLine << ", column " << errorColumn << ": " << errorMsg;
        qDebug() << "[BestiaryPopulateTokensDialog] Data: " << bytes;
        reply->deleteLater();
        return;
    }

    QDomElement root = doc.documentElement();
    if(root.isNull())
    {
        qDebug() << "[BestiaryFindTokenDialog] ERROR identified reading data: unable to find root element: " << doc.toString();
        reply->deleteLater();
        return;
    }

    QDomElement imageSetElement = root.firstChildElement(QString("imageSet"));
    if(!imageSetElement.isNull())
    {
#ifdef DEBUG_POPULATE_TOKENS
        qDebug() << "[BestiaryPopulateTokensDialog] Image Source: " << imageSetElement.attribute(QString("sourceName")) << ", URL: " << imageSetElement.attribute(QString("sourceURL"));
#endif

        QDomElement imageElement = imageSetElement.firstChildElement(QString("image"));
        if(!imageElement.isNull())
        {
            QDomElement imageURLListElement = imageElement.firstChildElement(QString("imageURLList"));
            if(!imageURLListElement.isNull())
            {
                QDomElement urlElement = imageURLListElement.firstChildElement(QString("url"));
                if(!urlElement.isNull())
                {
                    QString urlText = urlElement.text();
                    if(!urlText.isEmpty())
                    {
#ifdef DEBUG_POPULATE_TOKENS
                        qDebug() << "[BestiaryPopulateTokensDialog] Found URL data for address: " << urlText;
#endif
                        connect(_manager, &QNetworkAccessManager::finished, this, &BestiaryPopulateTokensDialog::imageRequestFinished);
                        _activeReply = _manager->get(QNetworkRequest(QUrl(urlText)));
                    }
                }
            }
        }
    }

    reply->deleteLater();
}

void BestiaryPopulateTokensDialog::imageRequestFinished(QNetworkReply *reply)
{
    disconnect(_manager, &QNetworkAccessManager::finished, this, &BestiaryPopulateTokensDialog::imageRequestFinished);
    _activeReply = nullptr;

    if((!reply) || (!_editor) || (_currentMonster.isEmpty()))
        return;

    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "[BestiaryFindTokenDialog] ERROR: network image reply not ok: " << reply->error();
        qDebug() << "[BestiaryFindTokenDialog] ERROR: " << reply->errorString();
    }
    else
    {
        QByteArray bytes = reply->readAll();
        if(bytes.size() > 0)
        {
            QImage newImage;
            if(newImage.loadFromData(bytes))
            {
                _editor->setSourceImage(newImage);
                QImage finalImage = _editor->getFinalImage();

                if(!finalImage.isNull())
                {
                    int fileIndex = 0;
                    QString tokenFile = _currentMonster + QString(".png");
                    while(_tokenDir.exists(tokenFile))
                        tokenFile = _currentMonster + QString::number(fileIndex++) + QString(".png");

                    QString tokenPath = _tokenDir.absoluteFilePath(tokenFile);
                    finalImage.save(tokenPath);
                    if(Bestiary::Instance())
                    {
                        MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(_currentMonster);
                        if(monsterClass)
                            monsterClass->addIcon(tokenPath);
                        _totalPopulated++;
                    }
                }
            }
        }
    }

    checkNextMonster();

    reply->deleteLater();
}

void BestiaryPopulateTokensDialog::browseFrame()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select Frame Image"), QString(), tr("Image Files (*.png *.jpg *.bmp)"));
    if(!filename.isEmpty())
    {
        _editor->setFrameFile(filename);
        updateImage();
    }
}

void BestiaryPopulateTokensDialog::browseMask()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select Mask Image"), QString(), tr("Image Files (*.png *.jpg *.bmp)"));
    if(!filename.isEmpty())
    {
        _editor->setMaskFile(filename);
        updateImage();
    }
}

void BestiaryPopulateTokensDialog::updateImage()
{
    QImage image = _editor->getFinalImage();
    ui->lblToken->setPixmap(QPixmap::fromImage(image));
}


bool BestiaryPopulateTokensDialog::eventFilter(QObject *o, QEvent *e)
{
    /*
    if(o == ui->frameToken)
    {
        if(e->type() == QEvent::Resize)
        {
            QSize size = ui->frameToken->size();
            QMargins margins = ui->frameToken->contentsMargins();
            int sideLength = qMin(size.width() - margins.left() - margins.right() - (ui->lblToken->lineWidth() * 2) - 10,
                                  size.height() - margins.top() - margins.bottom() - (ui->lblToken->lineWidth() * 2) - 10);
            ui->lblToken->setFixedSize(sideLength, sideLength);
        }
    }
*/

    return QDialog::eventFilter(o, e);
}

void BestiaryPopulateTokensDialog::checkNextMonster()
{
    if(_monsterList.isEmpty())
    {
        qDebug() << "[BestiaryPopulateTokensDialog] All " << _totalPopulated << " tokens populated!";
        QMessageBox::information(this, tr("Tokens Populated"), tr("All tokens have been populated!"));
        cancelRequests();
        accept();
    }
    else
    {
        _currentMonster = _monsterList.takeFirst();

        if(_waitingDlg)
            _waitingDlg->setStatus(tr("Populating token (") + QString::number(_totalPopulated + 1) + tr("/") + QString::number(_totalMonsters) + tr(": ") + _currentMonster + tr("..."));

        postRequest();
    }
}

void BestiaryPopulateTokensDialog::postRequest()
{
    if((!_manager) || (_currentMonster.isEmpty()))
        return;

#ifdef DEBUG_POPULATE_TOKENS
    QUrl serviceUrl = QUrl("https://api.dmhh.net/searchimage?version=3.0&debug=true");
#else
    QUrl serviceUrl = QUrl("https://api.dmhh.net/searchimage?version=3.0");
#endif
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("searchString", _searchString + QString(" ") + _currentMonster);
    postData.addQueryItem("bestFit", QString("true"));

#ifdef DEBUG_POPULATE_TOKENS
    qDebug() << "[BestiaryPopulateTokensDialog] Posting search request: " << postData.toString(QUrl::FullyEncoded).toUtf8();
#endif

    connect(_manager, &QNetworkAccessManager::finished, this, &BestiaryPopulateTokensDialog::urlRequestFinished);
    _activeReply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
}

void BestiaryPopulateTokensDialog::cancelRequests()
{
    disconnect(_manager, &QNetworkAccessManager::finished, this, &BestiaryPopulateTokensDialog::urlRequestFinished);
    disconnect(_manager, &QNetworkAccessManager::finished, this, &BestiaryPopulateTokensDialog::imageRequestFinished);

    if(_waitingDlg)
    {
        _waitingDlg->accept();
        _waitingDlg->deleteLater();
        _waitingDlg = nullptr;
    }

    if(_activeReply)
    {
        _activeReply->abort();
        _activeReply->deleteLater();
        _activeReply = nullptr;
    }
}
