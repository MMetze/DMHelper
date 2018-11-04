#ifndef DMHOBJECTBASE_PRIVATE_H
#define DMHOBJECTBASE_PRIVATE_H

#include <QString>
#include <QDomElement>

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
    virtual ~DMHObjectBase_Private();

    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory);
    virtual void inputXML(const QDomElement &element);
    virtual void postProcessXML(const QDomElement &element);

    int getID() const;
    void setID(int id);

    // STATIC FUNCTIONS
    static void setBaseId(int baseId);
    static void resetBaseId();
    static int createId();
    static int _id_global;

private:
    int _id;

};

#endif // DMHOBJECTBASE_PRIVATE_H
