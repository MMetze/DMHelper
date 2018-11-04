#ifndef DMHOBJECTBASE_H
#define DMHOBJECTBASE_H

#include "dmhglobal.h"
#include <memory>
#include <QObject>

class DMHObjectBase_Private;
class QDomDocument;
class QDomElement;
class QDir;

const int DMH_GLOBAL_INVALID_ID = -1;

class DMHSHARED_EXPORT DMHObjectBase : public QObject
{
    Q_OBJECT
public:

    explicit DMHObjectBase(QObject *parent = nullptr);
    DMHObjectBase(const DMHObjectBase& other);
    virtual ~DMHObjectBase();

    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory);
    virtual void inputXML(const QDomElement &element);
    virtual void postProcessXML(const QDomElement &element);

    int getID() const;

    // STATIC FUNCTIONS
    static void setBaseId(int baseId);
    static void resetBaseId();
    static int createId();

signals:

public slots:

protected:
    void setID(int id);

private:
    std::unique_ptr<DMHObjectBase_Private> d;

};

#endif // DMHOBJECTBASE_H
