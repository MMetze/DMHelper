#ifndef REMOTEAUDIOPLAYER_H
#define REMOTEAUDIOPLAYER_H

#include <QObject>
#include <QList>
#include <QUuid>

class AudioTrack;
class QDomElement;
class RemoteAudioPlayer_FileWrapper;

class RemoteAudioPlayer : public QObject
{
    Q_OBJECT
public:
    explicit RemoteAudioPlayer(const QString& cacheDirectory, QObject *parent = nullptr);
    virtual ~RemoteAudioPlayer() override;

    void parseAudioString(const QString& audioString);

public slots:
    void fileRequestStarted(int requestId, const QString& fileMD5);
    void fileRequestCompleted(int requestId, const QString& fileMD5, const QByteArray& data);

signals:
    void requestFile(const QString& md5String);

private:
    void setChecked(bool checked);
    void clearUnchecked();
    RemoteAudioPlayer_FileWrapper* findTrack(const QUuid& id);

    QList<RemoteAudioPlayer_FileWrapper*> _tracks;
    QString _cacheDirectory;
};

class RemoteAudioPlayer_FileWrapper : public QObject
{
    Q_OBJECT
public:
    explicit RemoteAudioPlayer_FileWrapper(const QString& cacheDirectory, const QDomElement& element);
    ~RemoteAudioPlayer_FileWrapper();

    AudioTrack* getTrack() const;
    void setTrack(AudioTrack* track);

    QUuid getId() const;
    void setId(QUuid id);

    int getStatus() const;
    void setStatus(int status);

    bool getChecked() const;
    void setChecked(bool checked);

signals:
    void requestFile(const QString& md5String);

private:
    AudioTrack* _track;
    QUuid _id;
    int _status;
    bool _checked;
};


#endif // REMOTEAUDIOPLAYER_H
