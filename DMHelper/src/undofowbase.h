#ifndef UNDOFOWBASE_H
#define UNDOFOWBASE_H

#include <QUndoCommand>

class QPaintDevice;
class QDomDocument;
class QDomElement;
class QDir;
class LayerFow;

class UndoFowBase : public QUndoCommand
{
public:
    UndoFowBase(LayerFow* layer, const QString & text);
    virtual ~UndoFowBase() override;

    virtual void apply( bool preview, QPaintDevice* target ) const = 0;

    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const = 0;
    virtual void inputXML(const QDomElement &element, bool isImport) = 0;

    virtual bool isRemoved() const;
    virtual void setRemoved(bool removed);

    virtual int getType() const;
    virtual UndoFowBase* clone() const = 0;

    void setLayer(LayerFow* layer);

protected:
    LayerFow* _layer;
    bool _removed;
};

#endif // UNDOFOWBASE_H
