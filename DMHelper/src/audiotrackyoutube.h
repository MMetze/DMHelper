#ifndef AUDIOTRACKYOUTUBE_H
#define AUDIOTRACKYOUTUBE_H

#include "audiotrackurl.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "dmh_vlc.h"

class AudioTrackYoutube : public AudioTrackUrl
{
    Q_OBJECT
public:
    explicit AudioTrackYoutube(const QString& trackName = QString(), const QUrl& trackUrl = QUrl(), QObject *parent = nullptr);
    virtual ~AudioTrackYoutube() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual QIcon getDefaultIcon() override;

    virtual int getAudioType() const override;
    virtual void eventCallback(const struct libvlc_event_t *p_event);

    virtual bool isPlaying() const override;
    virtual bool isRepeat() const override;
    virtual bool isMuted() const override;
    virtual float getVolume() const override;

public slots:
    virtual void play() override;
    virtual void stop() override;
    virtual void setMute(bool mute) override;
    virtual void setVolume(float volume) override;
    virtual void setRepeat(bool repeat) override;

signals:

protected slots:
    void urlRequestFinished(QNetworkReply *reply);

protected:
    // From QObject
    virtual void timerEvent(QTimerEvent *event) override;

    // From CampaignObjectBase
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    virtual void findDirectUrl(const QString& youtubeId);
    bool handleReplyDirect(const QByteArray& data);
    void playDirectUrl();
    void internalStopCheck(int status);
    QString extractYoutubeIDFromUrl();

    QNetworkAccessManager* _manager;
    QString _urlString;
    libvlc_media_player_t *_vlcPlayer;
    int _stopStatus;
    int _volume;
    int _timerId;
    bool _repeat;
    bool _mute;
};

#endif // AUDIOTRACKYOUTUBE_H
