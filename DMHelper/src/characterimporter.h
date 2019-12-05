#ifndef CHARACTERIMPORTER_H
#define CHARACTERIMPORTER_H

#include <QObject>
#include <QUuid>

class Campaign;
class Character;
class QNetworkReply;
class QNetworkAccessManager;

class CharacterImporter : public QObject
{
    Q_OBJECT
public:
    explicit CharacterImporter(QObject *parent = nullptr);
    ~CharacterImporter();

signals:
    void characterImported(QUuid characterId);

public slots:
    void importCharacter(Campaign* campaign);
    QUuid oldImportCharacter(Campaign& campaign);

    void campaignChanged();

protected:
    void scanModifiers(QJsonObject modifiersObject, const QString& key, Character& character);
    bool interpretReply(QNetworkReply* reply);
    bool interpretImageReply(QNetworkReply* reply);

protected slots:
    void replyFinished(QNetworkReply *reply);
    void imageReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *_manager;
    Campaign* _campaign;
    Character* _character;
};

#endif // CHARACTERIMPORTER_H
