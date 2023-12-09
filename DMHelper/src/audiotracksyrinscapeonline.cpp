#include "audiotracksyrinscapeonline.h"
#include "dmconstants.h"
#include <QDesktopServices>
#include <QMessageBox>
#include <QUrlQuery>
#include <QIcon>
#include <QDebug>

AudioTrackSyrinscapeOnline::AudioTrackSyrinscapeOnline(const QString& trackName, const QUrl& trackUrl, QObject *parent) :
    AudioTrackUrl(trackName, trackUrl, parent),
    _manager(nullptr)
{
}

AudioTrackSyrinscapeOnline::~AudioTrackSyrinscapeOnline()
{
    stop();
}

QIcon AudioTrackSyrinscapeOnline::getDefaultIcon()
{
    return QIcon(":/img/data/icon_syrinscape.png");
}

int AudioTrackSyrinscapeOnline::getAudioType() const
{
    return DMHelper::AudioType_SyrinscapeOnline;
}

void AudioTrackSyrinscapeOnline::doGet(QUrl url)
{
    /* This will need to change to use network handling like the updater, I think. */
    if (!_manager) {
        _manager = new QNetworkAccessManager(this);
        connect(_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(syrinscapeRequestFinished(QNetworkReply*)));
    }
    qDebug() << "Trying to play the syrinscape online audio track: " << url;
    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    _manager->get(request);
}

bool AudioTrackSyrinscapeOnline::checkRequestError(QNetworkReply *reply, QString action)
{
    if (!reply)
    {
        QMessageBox::critical(nullptr,
                              QString("Syrinscape Online Failure"),
                              QString("An unexpected and unknown error was encountered trying to ") + action + QString(" a Syrinscape online track"));
        return false;
    }
    if(reply->error() != QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();
        qDebug() << bytes;
        if(reply->error() == QNetworkReply::HostNotFoundError)
        {
            QMessageBox::critical(nullptr,
                                  QString("Syrinscape Online Failure"),
                                  QString("A network error was encountered trying to ") + action + QString(" a Syrinscape online track. It was not possible to reach the server!"));
        }
        else
        {
            QMessageBox::critical(nullptr,
                                  QString("Syrinscape Online Failure"),
                                  QString("A network error was encountered trying to ") + action + QString(" a Syrinscape online track") + QChar::LineFeed + QChar::LineFeed + reply->errorString());
        }
        return false;
    }
    return true;
}

void AudioTrackSyrinscapeOnline::syrinscapeRequestFinished(QNetworkReply *reply)
{
    QString requestType;
    QString url = reply->url().url();
    if (url.contains(QString("play"))) {
        requestType = QString("play");
    } else if (url.contains("stop")) {
        requestType = QString("stop");
    } else {
        requestType = QString("unknown");
    }
    checkRequestError(reply, requestType);
    reply->deleteLater();
    QByteArray bytes = reply->readAll();
    // Generally... there's not much else to do here... if there wasn't an error... we're done.
}

void AudioTrackSyrinscapeOnline::play()
{
    QUrl playUrl = getUrl();
    doGet(playUrl);
    return;
}

void AudioTrackSyrinscapeOnline::stop()
{
    QString url = getUrl().url().replace(QString("play"), QString("stop"));
    QUrl stopUrl = QUrl(url);
    doGet(stopUrl);
    return;
}

void AudioTrackSyrinscapeOnline::setMute(bool mute)
{
    Q_UNUSED(mute);
}

void AudioTrackSyrinscapeOnline::setVolume(float volume)
{
    Q_UNUSED(volume);
}

void AudioTrackSyrinscapeOnline::setRepeat(bool repeat)
{
    Q_UNUSED(repeat);
}
