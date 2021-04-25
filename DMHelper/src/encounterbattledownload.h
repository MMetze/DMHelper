#ifndef ENCOUNTERBATTLEDOWNLOAD_H
#define ENCOUNTERBATTLEDOWNLOAD_H

#include "encounterbattle.h"

class EncounterBattleDownload : public EncounterBattle
{
    Q_OBJECT
public:
    explicit EncounterBattleDownload(const QString& cacheDirectory, QObject *parent = nullptr);

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    bool isComplete();

public slots:
    virtual void fileReceived(const QString& md5, const QString& uuid, const QByteArray& data);

signals:
    void requestFile(const QString& md5, const QString& uuid, int fileType);
    void encounterComplete();

protected:
    virtual void inputXMLBattle(const QDomElement &element, bool isImport) override;
    virtual void inputXMLEffects(const QDomElement &parentElement, bool isImport) override;

    QString _cacheDirectory;
};

#endif // ENCOUNTERBATTLEDOWNLOAD_H
