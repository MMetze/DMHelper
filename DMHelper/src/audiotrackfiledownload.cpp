#include "audiotrackfiledownload.h"
#include <QFile>
#include <QDebug>

AudioTrackFileDownload::AudioTrackFileDownload(const QString& md5String, const QString& trackName, const QUrl& trackUrl, QObject *parent) :
    AudioTrackFile(trackName, trackUrl, parent),
    _md5(md5String)
{
}

AudioTrackFileDownload::~AudioTrackFileDownload()
{
}

void AudioTrackFileDownload::play()
{
    if(QFile::exists(_md5))
    {
        setUrl(_md5);
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

    if(!QFile::exists(_md5))
    {
        if(data.size() <= 0)
        {
           qDebug() << "[AudioTrackFileDownload] ERROR: empty data string received!";
           return;
        }

        QFile outputFile(_md5);
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
