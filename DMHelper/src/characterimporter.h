#ifndef CHARACTERIMPORTER_H
#define CHARACTERIMPORTER_H

#include <QObject>
#include <QUuid>
#include <QMessageBox>
#include <QMap>

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
    void importCharacter(Campaign* campaign, bool isCharacter = true);
    void updateCharacter(Character* character);
    void updateCharacters(QList<Character*> characters);

    void campaignChanged();

protected:
    void scanModifiers(QJsonObject modifiersObject, const QString& key, Character& character);
    void scanChoices(QJsonObject choicesObject, Character& character);
    QString getNotesString(QJsonObject notesParent, const QString& key, const QString& title);
    bool interpretReply(QNetworkReply* reply);
    bool interpretImageReply(QNetworkReply* reply);

    void startImport(QString characterId = QString());
    void finishImport();

protected slots:
    void initializeValues();

    void replyFinished(QNetworkReply *reply);
    void imageReplyFinished(QNetworkReply *reply);
    void messageBoxCancelled();

private:
    QNetworkAccessManager *_manager;
    QNetworkReply* _reply;
    Campaign* _campaign;
    Character* _character;
    QList<Character*> _characterList;
    bool _isCharacter;
    QMessageBox* _msgBox;

    QMap<int, int> _attributeSetValues;
    int _levelCount;
    int _totalArmor;
    int _totalHP;
    bool _halfProficiency;
};

#endif // CHARACTERIMPORTER_H
