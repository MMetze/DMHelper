#ifndef BATTLEDIALOGEVENT_H
#define BATTLEDIALOGEVENT_H

#include <QObject>

class QDomElement;

class BattleDialogEvent : public QObject
{
    Q_OBJECT
public:
    explicit BattleDialogEvent(QObject *parent = nullptr);
    BattleDialogEvent(const QDomElement& element, QObject *parent = nullptr);
    BattleDialogEvent(const BattleDialogEvent& other);
    virtual ~BattleDialogEvent();

    virtual int getType() const = 0;

    virtual void outputXML(QDomElement &element, bool isExport);

signals:

public slots:
};

#endif // BATTLEDIALOGEVENT_H
