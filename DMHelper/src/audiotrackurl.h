#ifndef AUDIOTRACKURL_H
#define AUDIOTRACKURL_H

#include "audiotrack.h"

class AudioTrackUrl : public AudioTrack
{
    Q_OBJECT
public:
    explicit AudioTrackUrl(const QString& trackName = QString(), const QUrl& trackUrl = QUrl(), QObject *parent = nullptr);

    // From CampaignObjectBase
    virtual void copyValues(const CampaignObjectBase* other) override;

    virtual QUrl getUrl() const override;
    virtual void setUrl(const QUrl& trackUrl) override;

signals:

protected:
    QUrl _url;
};

#endif // AUDIOTRACKURL_H
