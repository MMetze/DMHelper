#include "bestiaryfindtokendialog.h"
#include "ui_bestiaryfindtokendialog.h"
#include "optionscontainer.h"
#include <QGridLayout>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QDomDocument>
#include <QDomElement>
#include <QPainter>
#include <QFileDialog>
#include <QInputDialog>
#include <QImageReader>
#include <QtMinMax>

//#define DEBUG_FINDTOKEN_IMPORT

const qreal TOKEN_ICON_SIZE = 256.0;

BestiaryFindTokenDialog::BestiaryFindTokenDialog(const QString& monsterName, const QString& searchString, const OptionsContainer& options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BestiaryFindTokenDialog),
    _monsterName(monsterName),
    _searchString(searchString),
    _manager(nullptr),
    _urlReply(nullptr),
    _editor(nullptr),
    _tokenGrid(nullptr),
    _tokenList()
{
    ui->setupUi(this);

    _tokenGrid = new QGridLayout;
    _tokenGrid->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    ui->scrollAreaWidgetContents->setLayout(_tokenGrid);

    ui->chkFill->setChecked(options.getTokenBackgroundFill());

    ui->chkTransparent->setChecked(options.getTokenTransparent());
    ui->btnTransparentColor->setColor(options.getTokenTransparentColor());
    ui->btnTransparentColor->setRotationVisible(false);
    ui->sliderFuzzy->setValue(options.getTokenTransparentLevel());

    ui->chkMask->setChecked(options.getTokenMaskApplied());
    ui->edtMaskImage->setText(options.getTokenMaskFile());
    ui->chkFrame->setChecked(options.getTokenFrameApplied());
    ui->edtFrameImage->setText(options.getTokenFrameFile());

    _editor = new TokenEditor;
    _editor->applyOptionsToEditor(options);

    connect(ui->chkFill, &QAbstractButton::toggled, _editor, &TokenEditor::setBackgroundFill);
    connect(ui->chkTransparent, &QAbstractButton::toggled, _editor, &TokenEditor::setTransparent);
    connect(ui->btnTransparentColor, &ColorPushButton::colorChanged, _editor, &TokenEditor::setTransparentColor);
    connect(ui->sliderFuzzy, &QAbstractSlider::valueChanged, _editor, &TokenEditor::setTransparentLevel);
    connect(ui->chkMask, &QAbstractButton::toggled, _editor, &TokenEditor::setMaskApplied);
    connect(ui->edtMaskImage, &QLineEdit::textChanged, _editor, &TokenEditor::setMaskFile);
    connect(ui->btnBrowseMaskImage, &QAbstractButton::clicked, this, &BestiaryFindTokenDialog::browseMask);
    connect(ui->chkFrame, &QAbstractButton::toggled, _editor, &TokenEditor::setFrameApplied);
    connect(ui->edtFrameImage, &QLineEdit::textChanged, _editor, &TokenEditor::setFrameFile);
    connect(ui->btnBrowseFrameImage, &QAbstractButton::clicked, this, &BestiaryFindTokenDialog::browseFrame);

    connect(ui->chkFill, &QAbstractButton::toggled, this, &BestiaryFindTokenDialog::updateLayoutImages);
    connect(ui->chkTransparent, &QAbstractButton::toggled, this, &BestiaryFindTokenDialog::updateLayoutImages);
    connect(ui->btnTransparentColor, &ColorPushButton::colorChanged, this, &BestiaryFindTokenDialog::updateLayoutImages);
    connect(ui->sliderFuzzy, &QAbstractSlider::valueChanged, this, &BestiaryFindTokenDialog::updateLayoutImages);
    connect(ui->chkMask, &QAbstractButton::toggled, this, &BestiaryFindTokenDialog::updateLayoutImages);
    connect(ui->edtMaskImage, &QLineEdit::textChanged, this, &BestiaryFindTokenDialog::updateLayoutImages);
    connect(ui->chkFrame, &QAbstractButton::toggled, this, &BestiaryFindTokenDialog::updateLayoutImages);
    connect(ui->edtFrameImage, &QLineEdit::textChanged, this, &BestiaryFindTokenDialog::updateLayoutImages);

    connect(ui->btnCustomize, &QAbstractButton::clicked, this, &BestiaryFindTokenDialog::customizeSearch);
    connect(ui->sliderZoom, &QAbstractSlider::valueChanged, this, &BestiaryFindTokenDialog::rescaleData);

    startSearch(_searchString + QString(" ") + monsterName);
}

BestiaryFindTokenDialog::~BestiaryFindTokenDialog()
{
    disconnect(_manager, &QNetworkAccessManager::finished, this, &BestiaryFindTokenDialog::imageRequestFinished);
    disconnect(_manager, &QNetworkAccessManager::finished, this, &BestiaryFindTokenDialog::urlRequestFinished);
    qDeleteAll(_tokenList);
    delete _manager;
    delete ui;
}

QList<QImage> BestiaryFindTokenDialog::retrieveSelection(bool decorated)
{
    QList<QImage> resultList;

    foreach(TokenData* data, _tokenList)
    {
        if((data) && (!data->_pixmap.isNull()) && (data->_button) && (data->_button->isChecked()))
        {
            QImage resultImage = decorated ? decorateFullImage(data->_pixmap, data->_background) : data->_pixmap.toImage();
            if(!resultImage.isNull())
                resultList.append(resultImage);
        }
    }

    return resultList;
}

bool BestiaryFindTokenDialog::isEditingToken() const
{
    return ui->chkEditTokens->isChecked();
}

TokenEditor* BestiaryFindTokenDialog::getEditor()
{
    return _editor;
}

void BestiaryFindTokenDialog::urlRequestFinished(QNetworkReply *reply)
{
    disconnect(_manager, &QNetworkAccessManager::finished, this, &BestiaryFindTokenDialog::urlRequestFinished);

    if(!reply)
        return;

    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "[BestiaryFindTokenDialog] ERROR: network image URL reply not ok: " << reply->error();
        qDebug() << "[BestiaryFindTokenDialog] ERROR: " << reply->errorString();
        return;
    }

    connect(_manager, &QNetworkAccessManager::finished, this, &BestiaryFindTokenDialog::imageRequestFinished);

    QByteArray bytes = reply->readAll();
    qDebug() << "[BestiaryFindTokenDialog] Token Search request received; payload " << bytes.size() << " bytes";
#ifdef DEBUG_FINDTOKEN_IMPORT
    qDebug() << "[BestiaryFindTokenDialog] Payload contents: " << QString(bytes.left(2000));
#endif

    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if(!doc.setContent(bytes, &errorMsg, &errorLine, &errorColumn))
    {
        qDebug() << "[BestiaryFindTokenDialog] ERROR identified reading data: unable to parse network reply XML at line " << errorLine << ", column " << errorColumn << ": " << errorMsg;
        qDebug() << "[BestiaryFindTokenDialog] Data: " << bytes;
        return;
    }

    QDomElement root = doc.documentElement();
    if(root.isNull())
    {
        qDebug() << "[BestiaryFindTokenDialog] ERROR identified reading data: unable to find root element: " << doc.toString();
        return;
    }

    QDomElement imageSetElement = root.firstChildElement(QString("imageSet"));
    while(!imageSetElement.isNull())
    {
#ifdef DEBUG_FINDTOKEN_IMPORT
        qDebug() << "[BestiaryFindTokenDialog] Image Source: " << imageSetElement.attribute(QString("sourceName")) << ", URL: " << imageSetElement.attribute(QString("sourceURL"));
#endif

        QDomElement imageElement = imageSetElement.firstChildElement(QString("image"));
        while(!imageElement.isNull())
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
                        TokenData* tokenData = new TokenData(urlText);
                #ifdef DEBUG_FINDTOKEN_IMPORT
                        qDebug() << "[BestiaryFindTokenDialog] Found URL data for address; " << tokenData->_tokenAddress;
                #endif
                        _tokenList.append(tokenData);
                        tokenData->_reply = _manager->get(QNetworkRequest(QUrl(tokenData->_tokenAddress)));
                    }
                }
            }
            imageElement = imageElement.nextSiblingElement(QString("image"));
        }
        imageSetElement = imageSetElement.nextSiblingElement(QString("imageSet"));
    }
}

void BestiaryFindTokenDialog::imageRequestFinished(QNetworkReply *reply)
{
    if(!reply)
        return;

    TokenData* data = getDataForReply(reply);
    if(!data)
        return;

    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "[BestiaryFindTokenDialog] ERROR: network image reply not ok: " << reply->error() << " for address: " << data->_tokenAddress;
        qDebug() << "[BestiaryFindTokenDialog] ERROR: " << reply->errorString();
        return;
    }

    QByteArray bytes = reply->readAll();
    if(bytes.size() <= 0)
        return;

    if(!data->_pixmap.loadFromData(bytes))
        return;

    int tokenSize = TOKEN_ICON_SIZE * qPow(2.0, static_cast<qreal>(ui->sliderZoom->value()) / 10.0);
    data->_scaledPixmap = data->_pixmap.scaled(tokenSize, tokenSize, Qt::KeepAspectRatio);

    QImage editImage = data->_scaledPixmap.toImage();
    data->_background = editImage.pixelColor(0, 0);

    updateLayout();
}

void BestiaryFindTokenDialog::browseFrame()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, QString("Select image frame..."));
    if((filename.isEmpty()) || (!QImageReader(filename).canRead()))
        return;

    ui->edtFrameImage->setText(filename);
}

void BestiaryFindTokenDialog::browseMask()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, QString("Select image mask..."));
    if((filename.isEmpty()) || (!QImageReader(filename).canRead()))
        return;

    ui->edtMaskImage->setText(filename);
}

void BestiaryFindTokenDialog::resizeEvent(QResizeEvent *event)
{
    updateLayout();
    QDialog::resizeEvent(event);
}

void BestiaryFindTokenDialog::startSearch(const QString& searchString)
{
    if(!_manager)
        _manager = new QNetworkAccessManager(this);

    connect(_manager, &QNetworkAccessManager::finished, this, &BestiaryFindTokenDialog::urlRequestFinished);

#ifdef DEBUG_FINDTOKEN_IMPORT
    QUrl serviceUrl = QUrl("https://api.dmhh.net/searchimage?version=3.0&debug=true");
#else
    QUrl serviceUrl = QUrl("https://api.dmhh.net/searchimage?version=3.0");
#endif
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("searchString", searchString);

#ifdef DEBUG_FINDTOKEN_IMPORT
    qDebug() << "[BestiaryFindTokenDialog] Posting search request: " << postData.toString(QUrl::FullyEncoded).toUtf8();
#endif

    _urlReply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
}

void BestiaryFindTokenDialog::abortSearches()
{
    disconnect(_manager, &QNetworkAccessManager::finished, this, &BestiaryFindTokenDialog::urlRequestFinished);
    disconnect(_manager, &QNetworkAccessManager::finished, this, &BestiaryFindTokenDialog::imageRequestFinished);

    if(_urlReply)
    {
        if(!_urlReply->isFinished())
            _urlReply->abort();
        _urlReply->deleteLater();
        _urlReply = nullptr;
    }

    foreach(TokenData* data, _tokenList)
    {
        if((data) && (data->_reply))
        {
            if(!data->_reply->isFinished())
                data->_reply->abort();
            data->_reply->deleteLater();
            data->_reply = nullptr;
        }
    }

    qDeleteAll(_tokenList);
    _tokenList.clear();
}

void BestiaryFindTokenDialog::customizeSearch()
{
    QString newSearch = QInputDialog::getText(nullptr, QString("Custom Search String"), QString("Search: "), QLineEdit::Normal, _searchString + QString(" ") + _monsterName);
    if(newSearch.isEmpty())
        return;

    abortSearches();
    clearGrid();
    startSearch(newSearch);
}

void BestiaryFindTokenDialog::updateLayout()
{
    clearGrid();

    int tokenSize = TOKEN_ICON_SIZE * qPow(2.0, static_cast<qreal>(ui->sliderZoom->value()) / 10.0);
    int tokeFrameSpacing = qMax(static_cast<int>(qSqrt(tokenSize)), 3);
    int tokenFrameSize = tokenSize + qMax(tokenSize / 10, 2);

    int columnCount = ui->scrollArea->width() / (tokenFrameSize + tokeFrameSpacing);

    _tokenGrid->setContentsMargins(tokeFrameSpacing, tokeFrameSpacing, tokeFrameSpacing, tokeFrameSpacing);
    _tokenGrid->setSpacing(tokeFrameSpacing);

    int column = 0;
    int row = 0;

    foreach(TokenData* data, _tokenList)
    {
        if((data) && (!data->_pixmap.isNull()))
        {
            QPushButton* tokenButton = new QPushButton(this);
            tokenButton->setMinimumSize(tokenFrameSize, tokenFrameSize);
            tokenButton->setMaximumSize(tokenFrameSize, tokenFrameSize);
            tokenButton->setIconSize(QSize(tokenSize, tokenSize));
            tokenButton->setIcon(QIcon(decoratePixmap(data->_scaledPixmap, data->_background)));
            tokenButton->setCheckable(true);
            data->_button = tokenButton;
            _tokenGrid->addWidget(tokenButton, row, column);
            if(++column >= columnCount)
            {
                ++row;
                column = 0;
            }
        }
    }

    update();
}

void BestiaryFindTokenDialog::updateLayoutImages()
{
    foreach(TokenData* data, _tokenList)
    {
        if((data) && (data->_button) && (!data->_scaledPixmap.isNull()))
            data->_button->setIcon(QIcon(decoratePixmap(data->_scaledPixmap, data->_background)));
    }

    update();
}

void BestiaryFindTokenDialog::clearGrid()
{
    if(!_tokenGrid)
        return;

    foreach(TokenData* data, _tokenList)
    {
        if(data)
            data->_button = nullptr;
    }

    // Delete the grid entries
    QLayoutItem *child = nullptr;
    while((child = _tokenGrid->takeAt(0)) != nullptr)
    {
        delete child->widget();
        delete child;
    }

    ui->scrollAreaWidgetContents->update();
}

void BestiaryFindTokenDialog::rescaleData()
{
    int tokenSize = TOKEN_ICON_SIZE * qPow(2.0, static_cast<qreal>(ui->sliderZoom->value()) / 10.0);

    foreach(TokenData* data, _tokenList)
    {
        if((data) && (!data->_pixmap.isNull()))
            data->_scaledPixmap = data->_pixmap.scaled(tokenSize, tokenSize, Qt::KeepAspectRatio);
    }

    updateLayout();
}

TokenData* BestiaryFindTokenDialog::getDataForReply(QNetworkReply *reply)
{
    if(!reply)
        return nullptr;

    foreach(TokenData* data, _tokenList)
    {
        if((data) && (data->_reply == reply))
            return data;
    }

    return nullptr;
}

QPixmap BestiaryFindTokenDialog::decoratePixmap(QPixmap pixmap, const QColor& background)
{
    return QPixmap::fromImage(decorateFullImage(pixmap, background));
}

QImage BestiaryFindTokenDialog::decorateFullImage(QPixmap pixmap, const QColor& background)
{
    if(!_editor)
        return pixmap.toImage();

    _editor->setSourceImage(pixmap.toImage());
    _editor->setBackgroundFillColor(background);

    return _editor->getFinalImage();
}

bool BestiaryFindTokenDialog::fuzzyColorMatch(QRgb first, QRgb second)
{
    return ((qAbs(qRed(first) - qRed(second)) <= ui->sliderFuzzy->value()) &&
            (qAbs(qGreen(first) - qGreen(second)) <= ui->sliderFuzzy->value()) &&
            (qAbs(qBlue(first) - qBlue(second)) <= ui->sliderFuzzy->value()));
}
