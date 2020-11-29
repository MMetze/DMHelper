#ifndef SOUNDBOARDGROUP_H
#define SOUNDBOARDGROUP_H

#include <QObject>
#include "audiotrack.h"

class SoundboardGroup : public QObject
{
    Q_OBJECT
public:
    explicit SoundboardGroup(const QString& groupName, QObject *parent = nullptr);

    QString getGroupName() const;

    QList<AudioTrack*> getTracks() const;
    void addTrack(AudioTrack* track);
    void removeTrack(AudioTrack* track);

signals:
    void groupNameChanged(const QString& groupName);

public slots:
    void setGroupName(const QString& groupName);

private:
    QString _groupName;
    QList<AudioTrack*> _tracks;
};

#endif // SOUNDBOARDGROUP_H
