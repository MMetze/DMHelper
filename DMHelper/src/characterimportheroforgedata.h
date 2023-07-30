#ifndef CHARACTERIMPORTHEROFORGEDATA_H
#define CHARACTERIMPORTHEROFORGEDATA_H

#include <QObject>
#include <QImage>

class QDomElement;
class QNetworkAccessManager;
class QNetworkReply;

class CharacterImportHeroForgeData : public QObject
{
    Q_OBJECT
public:
    explicit CharacterImportHeroForgeData(QObject *parent = nullptr);

    void clear();

    bool readImportData(const QDomElement& element);
    void getData();

    QImage getFrontImage() const;
    QImage getPerspectiveImage() const;
    QImage getTopImage() const;
    QImage getThumbImage() const;
    QString getMesh() const;
    QString getMeta() const;
    QString getName() const;

signals:
    void dataReady(CharacterImportHeroForgeData* data);

protected slots:
    void urlRequestFinished(QNetworkReply *reply);

protected:
    void readReply(QNetworkReply* reply);
    bool isComplete();

    QString _imgFrontPath;
    QImage _imgFront;
    QString _imgPerspectivePath;
    QImage _imgPerspective;
    QString _imgTopPath;
    QImage _imgTop;
    QString _imgThumbPath;
    QImage _imgThumb;

    QString _meshPath;
    QString _mesh;
    QString _metaPath;
    QString _meta;

    QString _name;

    QNetworkAccessManager* _manager;

};

#endif // CHARACTERIMPORTHEROFORGEDATA_H
