#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

#include <QObject>

class CampaignObjectBase;
class QDomElement;

class ObjectFactory : public QObject
{
    Q_OBJECT
public:
    explicit ObjectFactory(QObject *parent = nullptr);

signals:

public slots:
    virtual CampaignObjectBase* createObject(int objectType, int subType, const QString& objectName, bool isImport) = 0;
    virtual CampaignObjectBase* createObject(const QDomElement& element, bool isImport) = 0;

protected:
    enum OLD_ENCOUNTER_TYPES
    {
        EncounterType_Blank = 0,
        EncounterType_Text,
        EncounterType_Battle,
        EncounterType_Character,
        EncounterType_Map,
        EncounterType_ScrollingText,
        EncounterType_AudioTrack,
        EncounterType_WelcomeScreen
    };

};

#endif // OBJECTFACTORY_H
