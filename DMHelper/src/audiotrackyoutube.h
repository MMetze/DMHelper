#ifndef AUDIOTRACKYOUTUBE_H
#define AUDIOTRACKYOUTUBE_H

#include "audiotrackurl.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

#ifdef Q_OS_WIN
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#endif
#include <vlc/vlc.h>

class AudioTrackYoutube : public AudioTrackUrl
{
    Q_OBJECT
public:
    explicit AudioTrackYoutube(const QString& trackName = QString(), const QUrl& trackUrl = QUrl(), QObject *parent = nullptr);
    virtual ~AudioTrackYoutube() override;

    virtual int getAudioType() const override;
    virtual void eventCallback(const struct libvlc_event_t *p_event);

public slots:
    virtual void play() override;
    virtual void stop() override;
    virtual void setMute(bool mute) override;
    virtual void setVolume(int volume) override;
    virtual void setRepeat(bool repeat) override;

signals:

protected slots:
    void urlRequestFinished(QNetworkReply *reply);

protected:
    virtual void timerEvent(QTimerEvent *event) override;

    virtual void findDirectUrl(const QString& youtubeId);
    bool handleReplyDirect(const QByteArray& data);
    void playDirectUrl();
    void internalStopCheck(int status);
    QString extractYoutubeIDFromUrl();

    QNetworkAccessManager* _manager;
    QString _urlString;
    libvlc_instance_t* _vlcInstance;
    libvlc_media_list_player_t* _vlcListPlayer;
    libvlc_media_player_t *_vlcPlayer;
    int _stopStatus;
    int _lastVolume;
    int _timerId;
    bool _repeat;
};

#endif // AUDIOTRACKYOUTUBE_H
