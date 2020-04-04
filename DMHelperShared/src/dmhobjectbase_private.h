#ifndef DMHOBJECTBASE_PRIVATE_H
#define DMHOBJECTBASE_PRIVATE_H

#include <QString>
#include <QDomElement>
#include <QUuid>

//class DMHObjectBase;
class QDomDocument;
class QDomElement;
class QDir;

const int DMH_GLOBAL_BASE_ID = 1000;

class DMHObjectBase_Private
{    
public:
    // Allow full access to the Campaign class
    //friend class DMHObjectBase;

    explicit DMHObjectBase_Private();
    DMHObjectBase_Private(const DMHObjectBase_Private& other);
    ~DMHObjectBase_Private();

    QDomElement outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport);
    void inputXML(const QDomElement &element, bool isImport);
    void postProcessXML(const QDomElement &element, bool isImport);

    QUuid getID() const;
    int getIntID() const;
    void setID(QUuid id);
    void setIntID(int id);

    // STATIC FUNCTIONS
    //static void setBaseId(int baseId);
    //static void resetBaseId();
    //static int createId();
    //static int _id_global;

    void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport);

private:
    QUuid _id;
    int _intId;

};

#endif // DMHOBJECTBASE_PRIVATE_H
