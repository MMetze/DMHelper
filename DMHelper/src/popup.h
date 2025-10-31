#ifndef POPUP_H
#define POPUP_H

#include <QObject>

class QBoxLayout;

class Popup : public QObject
{
    Q_OBJECT
public:
    explicit Popup(QObject *parent = nullptr);

    // Local interface
    virtual bool isDMOnly() const;
    virtual void prepareFrame(QBoxLayout* frameLayout, int insertIndex) = 0;

signals:
    void dirty();
    void triggerUpdate();
};

#endif // POPUP_H
