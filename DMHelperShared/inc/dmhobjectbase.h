#ifndef DMHOBJECTBASE_H
#define DMHOBJECTBASE_H

#include "dmhglobal.h"
#include <memory>
#include <QObject>
#include <QUuid>

class DMHObjectBase_Private;
class QDomDocument;
class QDomElement;
class QDir;

const int DMH_GLOBAL_INVALID_ID = -1;

//class DMHSHARED_EXPORT DMHObjectBase : public QObject
class DMHObjectBase : public QObject
{
    Q_OBJECT
public:

    explicit DMHObjectBase(QObject *parent = nullptr);
    DMHObjectBase(const DMHObjectBase& other);
    virtual ~DMHObjectBase();

    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport);
    virtual void inputXML(const QDomElement &element, bool isImport);

    QUuid getID() const;
    int getIntID() const;

signals:

public slots:

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport);

    void setID(QUuid id);
    void setIntID(int id);

    // To be used with extreme caution!!
    void renewID();

private:
    std::unique_ptr<DMHObjectBase_Private> d;

    // Allow the campaign to access protected functions, particularly renewID
    friend class Campaign;
};

#endif // DMHOBJECTBASE_H
