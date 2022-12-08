#ifndef CHARACTERIMPORTHEROFORGE_H
#define CHARACTERIMPORTHEROFORGE_H

#include <QObject>

class CharacterImportHeroForgeData;
class QNetworkReply;
class QNetworkAccessManager;
class QDomElement;

class CharacterImportHeroForge : public QObject
{
    Q_OBJECT
public:
    explicit CharacterImportHeroForge(QObject *parent = nullptr);

public slots:
    void runImport();

signals:
    void importComplete(QList<CharacterImportHeroForgeData*> data);

protected slots:
    void urlRequestFinished(QNetworkReply *reply);

protected:
    void handleRequestFinished(QNetworkReply *reply);
    bool parseReply(const QByteArray& data);

    QNetworkAccessManager* _manager;
    QList<CharacterImportHeroForgeData*> _importData;
};

#endif // CHARACTERIMPORTHEROFORGE_H
