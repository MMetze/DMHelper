#ifndef AUDIOFACTORY_H
#define AUDIOFACTORY_H

#include "objectfactory.h"

class AudioTrack;

class AudioFactory : public ObjectFactory
{
    Q_OBJECT
public:
    explicit AudioFactory(QObject *parent = nullptr);

public slots:
    virtual CampaignObjectBase* createObject(int objectType, int subType, const QString& objectName, bool isImport) override;
    virtual CampaignObjectBase* createObject(const QDomElement& element, bool isImport) override;

    AudioTrack* createTrackFromUrl(const QUrl& url, const QString& objectName);
    int identifyAudioSubtype(const QUrl& url);
};

#endif // AUDIOFACTORY_H
