#ifndef TEMPLATEOBJECTNOTIFIER_H
#define TEMPLATEOBJECTNOTIFIER_H

#include <QObject>

// Lightweight QObject helper owned by every TemplateObject so the non-QObject
// TemplateObject base can publish per-key change events without forcing the
// base into the QObject class hierarchy (which would conflict with subclasses
// that already inherit QObject themselves, e.g. MonsterClassv2).
class TemplateObjectNotifier : public QObject
{
    Q_OBJECT
public:
    explicit TemplateObjectNotifier(QObject* parent = nullptr);

signals:
    void valueChanged(const QString& key);
};

#endif // TEMPLATEOBJECTNOTIFIER_H
