#ifndef AUDIOTRACKFILEDOWNLOAD_H
#define AUDIOTRACKFILEDOWNLOAD_H

#include "audiotrackfile.h"

class AudioTrackFileDownload : public AudioTrackFile
{
    Q_OBJECT
public:
    explicit AudioTrackFileDownload(const QString& md5String, const QString& trackName, const QUrl& trackUrl = QUrl(), QObject *parent = nullptr);
    virtual ~AudioTrackFileDownload() override;

public slots:
    virtual void play() override;
    virtual void fileReceived(const QString& md5String, const QByteArray& data);

signals:
    void requestFile(const QString& md5String);

protected:
    QString _md5;
};

#endif // AUDIOTRACKFILEDOWNLOAD_H
