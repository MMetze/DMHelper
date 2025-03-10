#ifndef SOUNDBOARDGROUP_H
#define SOUNDBOARDGROUP_H

#include <QObject>

class SoundboardTrack;
class AudioTrack;
class Campaign;
class QDomElement;
class QDomDocument;
class QDir;

class SoundboardGroup : public QObject
{
    Q_OBJECT
public:
    explicit SoundboardGroup(const QString& groupName, QObject *parent = nullptr);
    explicit SoundboardGroup(Campaign& campaign, const QDomElement& element, bool isImport);
    virtual ~SoundboardGroup();

    // Parallel to CampaignObjectBase
    void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport);

    QString getGroupName() const;
    int getVolume() const;
    bool getMute() const;

    QList<SoundboardTrack*> getTracks() const;
    void addTrack(SoundboardTrack* track);
    void addTrack(AudioTrack* track);
    void removeTrack(SoundboardTrack* track);

signals:
    void groupNameChanged(const QString& groupName);
    void volumeChanged(int volume);
    void muteChanged(bool mute);

public slots:
    void setGroupName(const QString& groupName);
    void setVolume(int volume);
    void setMute(bool mute);

protected:
    void internalPostProcessXML(Campaign& campaign, const QDomElement& element, bool isImport);

    QString _groupName;
    int _volume;
    bool _mute;
    QList<SoundboardTrack*> _tracks;
};

#endif // SOUNDBOARDGROUP_H
