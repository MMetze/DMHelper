#ifndef REMOTERENDERER_H
#define REMOTERENDERER_H

#include <QObject>
#include <QUuid>
#include <QPixmap>

class QDomElement;
class CampaignObjectBase;
class CampaignObjectRenderer;
class RemoteRenderer_FileWrapper;


class RemoteRenderer : public QObject
{
    Q_OBJECT
public:
    explicit RemoteRenderer(const QString& cacheDirectory, QObject *parent = nullptr);
    virtual ~RemoteRenderer() override;

    void parseImageData(const QString& imageData, const QString& payloadData);
    void targetResized(const QSize& newSize);
    void loadBattle();
    bool isComplete() const;

public slots:
    void fileRequestStarted(int requestId, const QString& fileMD5, const QString& fileUuid);
    void fileRequestCompleted(int requestId, const QString& fileMD5, const QString& fileUuid, const QByteArray& data);

    void reset();

signals:
    void targetChanged();
    void requestFile(const QString& md5, const QString& uuid);
    void abortRequest(int requestID);
    void publishPixmap(QPixmap pixmap);
    void publishImage(QImage image);
    void payloadDataAvailable(const QString& md5, const QString& uuid, const QByteArray& data);

private slots:
    void setBackgroundData(const QString& md5, const QString& uuid, const QByteArray& data);
    void setFowData(const QString& md5, const QString& uuid, const QByteArray& data);
    void dataComplete();

private:
    void parsePayloadData(const QDomElement& element);
    void requestPayloadData();
    void payloadDataRequested(const QString& md5String, const QString& uuid, int fileType);
    void resetRendering();

    CampaignObjectBase* _activeObject;
    CampaignObjectRenderer* _renderer;

    RemoteRenderer_FileWrapper* _background;
    RemoteRenderer_FileWrapper* _fow;
    QList<RemoteRenderer_FileWrapper*> _payloadFiles;
    QPixmap _backgroundPmp;
    QPixmap _fowPmp;

    QString _cacheDirectory;
    QSize _targetSize;
};

class RemoteRenderer_FileWrapper : public QObject
{
    Q_OBJECT
public:
    explicit RemoteRenderer_FileWrapper(const QString& md5, const QString& uuid, int fileType);
    ~RemoteRenderer_FileWrapper();

    QString getMD5() const;
    void setMD5(const QString& md5);

    int getFileType() const;
    void setFileType(int fileType);

    QString getUuid() const;
    void setUuid(const QString& uuid);

    int getStatus() const;
    void setStatus(int status);
    bool isNotStarted() const;

public slots:
    void getFile();
    bool fileReceived(int requestID, const QString& md5, const QString& uuid, const QString& cacheDirectory, const QByteArray& data);

signals:
    void requestFile(const QString& md5, const QString& uuid);
    void abortRequest(int requestID);
    void dataAvailable(const QString& md5, const QString& uuid, const QByteArray& data);

private:
    QString _md5;
    int _fileType;
    QString _uuid;
    int _status;
};

#endif // REMOTERENDERER_H
