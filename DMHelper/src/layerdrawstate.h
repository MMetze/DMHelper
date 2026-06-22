#ifndef LAYERDRAWSTATE_H
#define LAYERDRAWSTATE_H

#include "layerdrawobject.h"
#include <QObject>

class QDomElement;
class QDomDocument;
class QDir;

class LayerDrawState : public QObject
{
    Q_OBJECT
public:
    explicit LayerDrawState(QObject *parent = nullptr);
    virtual ~LayerDrawState();

    void inputXML(const QDomElement &element, bool isImport);
    void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport);

    // Object management
    int indexOf(const QUuid& id) const;
    int indexOf(LayerDrawObject* object) const;
    int appendObject(LayerDrawObject* object);
    int insertObject(int index, LayerDrawObject* object);
    LayerDrawObject* takeObject(const QUuid& id);

    QList<LayerDrawObject*> getObjects() const;

signals:
    void objectAdded(LayerDrawObject* object, int index);
    void objectRemoved(LayerDrawObject* object, int index);

protected:
    QList<LayerDrawObject*> _drawObjects;
};


#endif // LAYERDRAWSTATE_H
