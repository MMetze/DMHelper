#include "bestiaryfindtokendialog.h"
#include "ui_bestiaryfindtokendialog.h"
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

//#define DEBUG_FINDTOKEN_IMPORT

const int TOKEN_FRAME_SPACING = 16;
const int TOKEN_ICON_SIZE = 256;
const int TOKEN_FRAME_SIZE = TOKEN_ICON_SIZE + (TOKEN_ICON_SIZE / 10);

BestiaryFindTokenDialog::BestiaryFindTokenDialog(const QString& monsterName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BestiaryFindTokenDialog),
    _monsterName(monsterName),
    _manager(nullptr),
    _urlReply(nullptr),
    _maskImage(),
    _frameImage(),
    _tokenGrid(nullptr),
    _tokenList()
{
    ui->setupUi(this);

    _tokenGrid = new QGridLayout;
    _tokenGrid->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    _tokenGrid->setContentsMargins(TOKEN_FRAME_SPACING, TOKEN_FRAME_SPACING, TOKEN_FRAME_SPACING, TOKEN_FRAME_SPACING);
    _tokenGrid->setSpacing(TOKEN_FRAME_SPACING);
    ui->scrollAreaWidgetContents->setLayout(_tokenGrid);

    ui->btnOriginalImage->setChecked(true);
    ui->btnColor->setColor(Qt::white);
    ui->btnColor->setRotationVisible(false);

    connect(ui->btnOriginalImage, &QAbstractButton::toggled, this, &BestiaryFindTokenDialog::updateLayoutImages);
    connect(ui->btnTransparentColor, &QAbstractButton::toggled, this, &BestiaryFindTokenDialog::updateLayoutImages);
    connect(ui->btnColor, &ColorPushButton::colorChanged, this, &BestiaryFindTokenDialog::updateLayoutImages);
    connect(ui->sliderFuzzy, &QAbstractSlider::valueChanged, this, &BestiaryFindTokenDialog::updateLayoutImages);
    connect(ui->btnFrameAndMask, &QAbstractButton::toggled, this, &BestiaryFindTokenDialog::updateLayoutImages);
    connect(ui->btnBrowseFrameImage, &QAbstractButton::clicked, this, &BestiaryFindTokenDialog::browseFrame);
    connect(ui->btnBrowseMaskImage, &QAbstractButton::clicked, this, &BestiaryFindTokenDialog::browseMask);
    connect(ui->edtFrameImage, &QLineEdit::textChanged, this, &BestiaryFindTokenDialog::updateFrameMaskImages);
    connect(ui->edtMaskImage, &QLineEdit::textChanged, this, &BestiaryFindTokenDialog::updateFrameMaskImages);

    connect(ui->btnCustomize, &QAbstractButton::clicked, this, &BestiaryFindTokenDialog::customizeSearch);

    startSearch(QString("dnd 5e ") + monsterName);
}

BestiaryFindTokenDialog::~BestiaryFindTokenDialog()
{
    disconnect(_manager, &QNetworkAccessManager::finished, this, &BestiaryFindTokenDialog::imageRequestFinished);
    disconnect(_manager, &QNetworkAccessManager::finished, this, &BestiaryFindTokenDialog::urlRequestFinished);
    qDeleteAll(_tokenList);
    delete _manager;
    delete ui;
}

QList<QImage> BestiaryFindTokenDialog::retrieveSelection()
{
    QList<QImage> resultList;

    foreach(TokenData* data, _tokenList)
    {
        if((data) && (!data->_pixmap.isNull()) && (data->_button) && (data->_button->isChecked()))
        {
            QImage resultImage = decorateFullImage(data->_pixmap, data->_background);
            if(!resultImage.isNull())
                resultList.append(resultImage);
        }
    }

    return resultList;
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

    QDomElement urlsElement = root.firstChildElement(QString("imageUrls"));
    if(urlsElement.isNull())
    {
        qDebug() << "[BestiaryFindTokenDialog] ERROR identified reading data: unable to find the URLs element: " << doc.toString();
        return;
    }

    QDomElement urlElement = urlsElement.firstChildElement(QString("url"));
    while(!urlElement.isNull())
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

        urlElement = urlElement.nextSiblingElement(QString("url"));
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

    data->_scaledPixmap = data->_pixmap.scaled(TOKEN_ICON_SIZE, TOKEN_ICON_SIZE, Qt::KeepAspectRatio);

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
    QString newSearch = QInputDialog::getText(nullptr, QString("Custom Search String"), QString("Search: "), QLineEdit::Normal, QString("dnd 5e ") + _monsterName);
    if(newSearch.isEmpty())
        return;

    abortSearches();
    clearGrid();
    startSearch(newSearch);
}

void BestiaryFindTokenDialog::updateLayout()
{
    clearGrid();

    int columnCount = ui->scrollArea->width() / (TOKEN_FRAME_SIZE + TOKEN_FRAME_SPACING);

    int column = 0;
    int row = 0;

    foreach(TokenData* data, _tokenList)
    {
        if((data) && (!data->_pixmap.isNull()))
        {
            QPushButton* tokenButton = new QPushButton(this);
            tokenButton->setMinimumSize(TOKEN_FRAME_SIZE, TOKEN_FRAME_SIZE);
            tokenButton->setMaximumSize(TOKEN_FRAME_SIZE, TOKEN_FRAME_SIZE);
            tokenButton->setIconSize(QSize(TOKEN_ICON_SIZE, TOKEN_ICON_SIZE));
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
    if(ui->btnOriginalImage->isChecked())
    {
        return pixmap;
    }
    else if(ui->btnTransparentColor->isChecked())
    {
        QImage inputImage = pixmap.toImage();
        QImage transparentImage(inputImage.size(), QImage::Format_ARGB32_Premultiplied);

        QRgb transparentColor = ui->btnColor->getColor().rgb();
        QRgb outputColor = qRgba(qRed(transparentColor), qGreen(transparentColor), qBlue(transparentColor), 0);

        for (int y = 0; y < inputImage.height(); y++)
        {
            const QRgb* inputLine = reinterpret_cast<const QRgb *>(inputImage.scanLine(y));
            QRgb* outputLine = reinterpret_cast<QRgb *>(transparentImage.scanLine(y));
            for(int x = 0; x < inputImage.width(); x++)
            {
                if(fuzzyColorMatch(inputLine[x], transparentColor))
                    outputLine[x] = outputColor;
                else
                    outputLine[x] = inputLine[x];
            }
        }

        return QPixmap::fromImage(transparentImage);
    }
    else if(ui->btnFrameAndMask->isChecked())
    {
        if((!_frameImage.isNull()) && (!_maskImage.isNull()))
        {
            QImage resultImage(TOKEN_ICON_SIZE, TOKEN_ICON_SIZE, QImage::Format_ARGB32_Premultiplied);
            resultImage.fill(background);

            QPainter p;
            p.begin(&resultImage);
                p.drawPixmap((TOKEN_ICON_SIZE - pixmap.width())/2, (TOKEN_ICON_SIZE - pixmap.height()) / 2, pixmap);
                p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                p.drawImage((TOKEN_ICON_SIZE - _maskImage.width())/2, (TOKEN_ICON_SIZE - _maskImage.height()) / 2, _maskImage);
                p.setCompositionMode(QPainter::CompositionMode_SourceOver);
                p.drawImage((TOKEN_ICON_SIZE - _frameImage.width())/2, (TOKEN_ICON_SIZE - _frameImage.height()) / 2, _frameImage);
            p.end();

            return QPixmap::fromImage(resultImage);
        }
    }

    return pixmap;
}

QImage BestiaryFindTokenDialog::decorateFullImage(QPixmap pixmap, const QColor& background)
{
    if(ui->btnOriginalImage->isChecked())
    {
        return pixmap.toImage();
    }
    else if(ui->btnTransparentColor->isChecked())
    {
        QImage inputImage = pixmap.toImage();
        QImage transparentImage(inputImage.size(), QImage::Format_ARGB32_Premultiplied);

        QRgb transparentColor = ui->btnColor->getColor().rgb();
        QRgb outputColor = qRgba(qRed(transparentColor), qGreen(transparentColor), qBlue(transparentColor), 0);

        for (int y = 0; y < inputImage.height(); y++)
        {
            const QRgb* inputLine = reinterpret_cast<const QRgb *>(inputImage.scanLine(y));
            QRgb* outputLine = reinterpret_cast<QRgb *>(transparentImage.scanLine(y));
            for(int x = 0; x < inputImage.width(); x++)
            {
                if(fuzzyColorMatch(inputLine[x], transparentColor))
                    outputLine[x] = outputColor;
                else
                    outputLine[x] = inputLine[x];
            }
        }

        return transparentImage;
    }
    else if(ui->btnFrameAndMask->isChecked())
    {
        int maxSize = qMax(pixmap.width(), pixmap.height());
        QImage frameImage = QImage(ui->edtFrameImage->text()).scaled(maxSize, maxSize, Qt::KeepAspectRatio);
        QImage maskImage = QImage(ui->edtMaskImage->text()).scaled(maxSize, maxSize, Qt::KeepAspectRatio);
        if((!frameImage.isNull()) && (!maskImage.isNull()))
        {
            QImage resultImage(maxSize, maxSize, QImage::Format_ARGB32_Premultiplied);
            resultImage.fill(background);

            QPainter p;
            p.begin(&resultImage);
                p.drawPixmap((maxSize - pixmap.width())/2, (maxSize - pixmap.height()) / 2, pixmap);
                p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                p.drawImage((maxSize - maskImage.width())/2, (maxSize - maskImage.height()) / 2, maskImage);
                p.setCompositionMode(QPainter::CompositionMode_SourceOver);
                p.drawImage((maxSize - frameImage.width())/2, (maxSize - frameImage.height()) / 2, frameImage);
            p.end();

            return resultImage;
        }
    }

    return pixmap.toImage();
}

void BestiaryFindTokenDialog::updateFrameMaskImages()
{
    _frameImage = QImage(ui->edtFrameImage->text()).scaled(TOKEN_ICON_SIZE, TOKEN_ICON_SIZE, Qt::KeepAspectRatio);
    _maskImage = QImage(ui->edtMaskImage->text()).scaled(TOKEN_ICON_SIZE, TOKEN_ICON_SIZE, Qt::KeepAspectRatio);

    if((!_frameImage.isNull()) && (!_maskImage.isNull()))
        updateLayoutImages();
}

bool BestiaryFindTokenDialog::fuzzyColorMatch(QRgb first, QRgb second)
{
    return ((qAbs(qRed(first) - qRed(second)) <= ui->sliderFuzzy->value()) &&
            (qAbs(qGreen(first) - qGreen(second)) <= ui->sliderFuzzy->value()) &&
            (qAbs(qBlue(first) - qBlue(second)) <= ui->sliderFuzzy->value()));
}
