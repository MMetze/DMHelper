#ifndef OPTIONSACCESSOR_H
#define OPTIONSACCESSOR_H

#include <QSettings>

class OptionsAccessor : public QSettings
{
    Q_OBJECT
public:
    explicit OptionsAccessor(QObject *parent = nullptr);
    virtual ~OptionsAccessor() override;

signals:

};

#endif // OPTIONSACCESSOR_H
