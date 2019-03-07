#ifndef CHARACTERIMPORTER_H
#define CHARACTERIMPORTER_H

#include <QObject>

class Campaign;
class Character;

class CharacterImporter : public QObject
{
    Q_OBJECT
public:
    explicit CharacterImporter(QObject *parent = nullptr);

signals:

public slots:
    int importCharacter(Campaign& campaign);

protected:
    void scanModifiers(QJsonObject modifiersObject, const QString& key, Character& character);
};

#endif // CHARACTERIMPORTER_H
