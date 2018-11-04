#ifndef DMHPAYLOAD_H
#define DMHPAYLOAD_H

#include "dmhglobal.h"
#include <memory>

class QString;
class DMHPayload_Private;

class DMHSHARED_EXPORT DMHPayload
{
public:
    DMHPayload();
    DMHPayload(const QString& data);
    ~DMHPayload();

    QString getImageFile() const;
    void setImageFile(const QString& imageFile);
    QString getAudioFile() const;
    void setAudioFile(const QString& audioFile);
    QString getCommand() const;
    void setCommand(const QString& command);
    QString getPayload() const;
    void setPayload(const QString& payload);

    void fromString(const QString& data);
    QString toString() const;

private:
    // No copy constructor allowed
    DMHPayload(const DMHPayload& other);

    std::unique_ptr<DMHPayload_Private> d;
};

#endif // DMHPAYLOAD_H
