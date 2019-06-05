#ifndef UNDOBASE_H
#define UNDOBASE_H

#include <QUndoCommand>

class QPaintDevice;
class QDomDocument;
class QDomElement;
class QDir;
class Map;

class UndoBase : public QUndoCommand
{
public:
    UndoBase(Map& map, const QString & text);

    virtual void apply( bool preview, QPaintDevice* target ) const = 0;

    virtual void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const = 0;
    virtual void inputXML(const QDomElement &element, bool isImport) = 0;

    virtual int getType() const;

protected:
    Map& _map;
};

#endif // UNDOBASE_H
