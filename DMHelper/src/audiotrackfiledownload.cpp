#include "audiotrackfiledownload.h"
#include <QFile>
#include <QDebug>

AudioTrackFileDownload::AudioTrackFileDownload(const QString& md5String, const QString& cacheDirectory, const QString& trackName, const QUrl& trackUrl, QObject *parent) :
    AudioTrackFile(trackName, trackUrl, parent),
    _md5(md5String),
    _cacheDirectory(cacheDirectory)
{
}

AudioTrackFileDownload::~AudioTrackFileDownload()
{
}

void AudioTrackFileDownload::play()
{
    QString fullFileName = _cacheDirectory + QString("/") + _md5;
    if(QFile::exists(fullFileName))
    {
        setUrl(fullFileName);
        AudioTrackFile::play();
    }
    else
    {
        emit requestFile(_md5);
    }
}

void AudioTrackFileDownload::fileReceived(const QString& md5String, const QByteArray& data)
{
    if(_md5 != md5String)
        return;

    QString fullFileName = _cacheDirectory + QString("/") + _md5;
    if(!QFile::exists(fullFileName))
    {
        if(data.size() <= 0)
        {
           qDebug() << "[AudioTrackFileDownload] ERROR: empty data string received!";
           return;
        }

        QFile outputFile(fullFileName);
        if(!outputFile.open(QIODevice::WriteOnly))
        {
            qDebug() << "[AudioTrackFileDownload] ERROR: not able to open output file for writing: " << outputFile.error() << ", " << outputFile.errorString();
            return;
        }

        outputFile.write(data);
        outputFile.close();
    }

    play();
}
