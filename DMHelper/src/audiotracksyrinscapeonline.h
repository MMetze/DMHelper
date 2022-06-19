#ifndef AUDIOTRACKSYRINSCAPEONLINE_H
#define AUDIOTRACKSYRINSCAPEONLINE_H

#include "audiotrackurl.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

class AudioTrackSyrinscapeOnline : public AudioTrackUrl
{
    Q_OBJECT
public:
    explicit AudioTrackSyrinscapeOnline(const QString& trackName = QString(), const QUrl& trackUrl = QUrl(), QObject *parent = nullptr);
    virtual ~AudioTrackSyrinscapeOnline() override;

    virtual int getAudioType() const override;

public slots:
    virtual void play() override;
    virtual void stop() override;
    virtual void setMute(bool mute) override;
    virtual void setVolume(int volume) override;
    virtual void setRepeat(bool repeat) override;

protected slots:
    void syrinscapeRequestFinished(QNetworkReply *reply);
    void doGet(QUrl url);


protected:
    bool checkRequestError(QNetworkReply *reply, QString action);

    QNetworkAccessManager* _manager;

signals:

};

#endif // AUDIOTRACKSYRINSCAPEONLINE_H
