#include "bestiaryfindtokendialog.h"
#include "ui_bestiaryfindtokendialog.h"
#include <QGridLayout>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QDomDocument>
#include <QDomElement>

//#define DEBUG_FINDTOKEN_IMPORT

const int TOKEN_FRAME_SPACING = 16;
const int TOKEN_ICON_SIZE = 256;
const int TOKEN_FRAME_SIZE = TOKEN_ICON_SIZE + (TOKEN_ICON_SIZE / 10);

BestiaryFindTokenDialog::BestiaryFindTokenDialog(const QString& monsterName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BestiaryFindTokenDialog),
    _monsterName(monsterName),
    _manager(nullptr),
    _tokenGrid(nullptr),
    _tokenList()
{
    ui->setupUi(this);

    _tokenGrid = new QGridLayout;
    _tokenGrid->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    _tokenGrid->setContentsMargins(TOKEN_FRAME_SPACING, TOKEN_FRAME_SPACING, TOKEN_FRAME_SPACING, TOKEN_FRAME_SPACING);
    _tokenGrid->setSpacing(TOKEN_FRAME_SPACING);
    ui->scrollAreaWidgetContents->setLayout(_tokenGrid);

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
    postData.addQueryItem("searchString", QString("dnd 5e") + monsterName);

#ifdef DEBUG_FINDTOKEN_IMPORT
    qDebug() << "[BestiaryFindTokenDialog] Posting search request: " << postData.toString(QUrl::FullyEncoded).toUtf8();
#endif

    _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
}

BestiaryFindTokenDialog::~BestiaryFindTokenDialog()
{
    disconnect(_manager, &QNetworkAccessManager::finished, this, &BestiaryFindTokenDialog::imageRequestFinished);
    disconnect(_manager, &QNetworkAccessManager::finished, this, &BestiaryFindTokenDialog::urlRequestFinished);
    qDeleteAll(_tokenList);
    delete _manager;
    delete ui;
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

    updateLayout();
}

void BestiaryFindTokenDialog::updateLayout()
{
    clearGrid();

    int i = 0;
    int j = 0;

    foreach(TokenData* data, _tokenList)
    {
        if((data) && (!data->_pixmap.isNull()))
        {
            QPushButton* tokenButton = new QPushButton(this);
            tokenButton->setMinimumSize(TOKEN_FRAME_SIZE, TOKEN_FRAME_SIZE);
            tokenButton->setMaximumSize(TOKEN_FRAME_SIZE, TOKEN_FRAME_SIZE);
            tokenButton->setIconSize(QSize(TOKEN_ICON_SIZE, TOKEN_ICON_SIZE));
            tokenButton->setIcon(QIcon(QPixmap(data->_pixmap).scaled(TOKEN_ICON_SIZE, TOKEN_ICON_SIZE, Qt::KeepAspectRatio)));
            tokenButton->setCheckable(true);
            _tokenGrid->addWidget(tokenButton, i, j);
            if(++i >= 2)
            {
                ++j;
                i = 0;
            }
        }
    }

    update();
}

void BestiaryFindTokenDialog::clearGrid()
{
    if(!_tokenGrid)
        return;

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


