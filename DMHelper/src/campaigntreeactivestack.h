#ifndef CAMPAIGNTREEACTIVESTACK_H
#define CAMPAIGNTREEACTIVESTACK_H

#include <QObject>
#include <QList>
#include <QUuid>

class CampaignTreeActiveStack : public QObject
{
    Q_OBJECT
public:
    explicit CampaignTreeActiveStack(QObject *parent = nullptr);

signals:
    void activateItem(const QUuid& uuid);
    void backwardsAvailable(bool available);
    void forwardsAvailable(bool available);

public slots:
    void backwards();
    void forwards();
    void push(const QUuid& uuid);
    void clear();

protected:
    void checkAvailable();

    QList<QUuid> _activeStack;
    int _index;
};

#endif // CAMPAIGNTREEACTIVESTACK_H
