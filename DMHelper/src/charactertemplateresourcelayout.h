#ifndef CHARACTERTEMPLATERESOURCELAYOUT_H
#define CHARACTERTEMPLATERESOURCELAYOUT_H

#include <QHBoxLayout>
#include "combatant.h"

class CharacterTemplateResourceLayout : public QHBoxLayout
{
    Q_OBJECT
public:
    CharacterTemplateResourceLayout(const QString& key, const ResourcePair& value);
    CharacterTemplateResourceLayout(const QString& key, int listIndex, const QString& listKey, const ResourcePair& value);
    ~CharacterTemplateResourceLayout();

signals:
    void resourceValueChanged(const QString& key, const ResourcePair& value);
    void resourceListValueChanged(const QString& key, int listIndex, const QString& listKey, const QVariant& value);

protected slots:
    void handleResourceChanged();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void cleanupLayout();
    void createCheckboxes();
    void emitChange();

    QString _key;
    int _listIndex;
    QString _listKey;
    ResourcePair _value;
};

#endif // CHARACTERTEMPLATERESOURCELAYOUT_H
