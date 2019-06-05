#ifndef CHARACTERIMPORTER_H
#define CHARACTERIMPORTER_H

#include <QObject>
#include <QUuid>

class Campaign;
class Character;

class CharacterImporter : public QObject
{
    Q_OBJECT
public:
    explicit CharacterImporter(QObject *parent = nullptr);

signals:

public slots:
    QUuid importCharacter(Campaign& campaign);

protected:
    void scanModifiers(QJsonObject modifiersObject, const QString& key, Character& character);
};

#endif // CHARACTERIMPORTER_H
