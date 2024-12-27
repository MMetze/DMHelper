#ifndef TEMPLATERESOURCELAYOUT_H
#define TEMPLATERESOURCELAYOUT_H

#include <QHBoxLayout>
#include "combatant.h"

class TemplateResourceLayout : public QHBoxLayout
{
    Q_OBJECT
public:
    TemplateResourceLayout(const QString& key, const ResourcePair& value);
    TemplateResourceLayout(const QString& key, int listIndex, const QString& listKey, const ResourcePair& value);
    ~TemplateResourceLayout();

signals:
    void resourceValueChanged(const QString& key, const ResourcePair& value);
    void resourceListValueChanged(const QString& key, int listIndex, const QString& listKey, const QVariant& value);
    void addResource();
    void removeResource(TemplateResourceLayout* layout);

protected slots:
    void handleResourceChanged();
    void handleAddResource();
    void handleRemoveResource();
    void handleEditResource();

protected:
    virtual bool eventFilter(QObject *object, QEvent *event) override;
    void cleanupLayout();
    void createCheckboxes();
    void emitChange();

    QString _key;
    int _listIndex;
    QString _listKey;
    ResourcePair _value;
};

#endif // TEMPLATERESOURCELAYOUT_H
