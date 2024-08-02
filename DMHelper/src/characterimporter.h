#ifndef CHARACTERIMPORTER_H
#define CHARACTERIMPORTER_H

#include "characterv2.h"
#include <QObject>
#include <QUuid>
#include <QMessageBox>
#include <QMap>

class Campaign;
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
    void characterCreated(CampaignObjectBase* character);

public slots:
    void importCharacter(Campaign* campaign, bool isCharacter = true);
    void updateCharacter(Characterv2* character);
    void updateCharacters(QList<Characterv2*> characters);

    void campaignChanged();

protected:
    void scanStats(QJsonArray statsArray, QJsonArray bonusStatsArray, QJsonArray overrideStatsArray, Characterv2& character);
    void scanSingleStat(int statId, QJsonArray statsArray, QJsonArray bonusStatsArray, QJsonArray overrideStatsArray, Characterv2& character);
    int getStatValue(QJsonArray statValueArray, int statIdValue);
    void setStatMods(Characterv2& character);
    QString getStatName(int statId);
    void zeroSavingThrows();
    void zeroSkills();
    void addHalfProficiencies();
    void addProficienciesSkillMods();
    void scanModifiers(QJsonObject modifiersObject, const QString& key, Characterv2& character);
    void scanChoices(QJsonObject choicesObject, Characterv2& character);
    void addAction(QList<QVariant>& actionValues, const QString& actionName, const QString& actionDescription, int attackBonus, Dice damage);
    QString getNotesString(QJsonObject notesParent, const QString& key, const QString& title);
    QString getSpellString(QJsonObject rootObject);
    void parseSpellSource(QVector<QStringList>& spellVector, QJsonObject rootObject, QJsonArray spellSource, bool autoPrepared);
    QString getEquipmentName(QJsonObject rootObject, int itemId, int itemTypeId);
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
    Characterv2* _character;
    QList<Characterv2*> _characterList;
    bool _isCharacter;
    QMessageBox* _msgBox;

    QMap<int, int> _attributeSetValues;
    int _levelCount;
    int _totalArmor;
    int _unarmored;
    int _speedModifier;
    int _totalHP;
    bool _halfProficiency;
    QList<int> _overrideList;
    bool _overrideHP;
};

#endif // CHARACTERIMPORTER_H
