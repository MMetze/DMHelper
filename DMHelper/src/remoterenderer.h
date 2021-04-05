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
    void fileRequestStarted(int requestId, const QString& fileMD5);
    void fileRequestCompleted(int requestId, const QString& fileMD5, const QByteArray& data);

signals:
    void requestFile(const QString& md5String);
    void abortRequest(int requestID);
    void publishPixmap(QPixmap pixmap);
    void publishImage(QImage image);
    void payloadDataAvailable(const QString& md5String, const QByteArray& data);

private slots:
    void setBackgroundData(const QString& md5String, const QByteArray& data);
    void setFowData(const QString& md5String, const QByteArray& data);
    void dataComplete();

private:
    void parsePayloadData(const QDomElement& element);
    void requestPayloadData();
    void payloadDataRequested(const QString& md5String, int fileType);

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
    explicit RemoteRenderer_FileWrapper(const QString& md5, int fileType);
    ~RemoteRenderer_FileWrapper();

    QString getMD5() const;
    void setMD5(const QString& md5);

    int getFileType() const;
    void setFileType(int fileType);

    QUuid getId() const;
    void setId(QUuid id);

    int getStatus() const;
    void setStatus(int status);
    bool isNotStarted() const;

public slots:
    void getFile();
    void fileReceived(const QString& md5String, const QString& cacheDirectory, const QByteArray& data);

signals:
    void requestFile(const QString& md5String);
    void abortRequest(int requestID);
    void dataAvailable(const QString& md5String, const QByteArray& data);

private:
    QString _md5;
    int _fileType;
    QUuid _id;
    int _status;
};

#endif // REMOTERENDERER_H
