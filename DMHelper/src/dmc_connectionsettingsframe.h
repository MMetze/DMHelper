#ifndef DMC_CONNECTIONSETTINGSFRAME_H
#define DMC_CONNECTIONSETTINGSFRAME_H

#include <QFrame>

namespace Ui {
class DMC_ConnectionSettingsFrame;
}

class DMC_OptionsContainer;
class DMHNetworkManager;

class DMC_ConnectionSettingsFrame : public QFrame
{
    Q_OBJECT

public:
    explicit DMC_ConnectionSettingsFrame(QWidget *parent = nullptr);
    ~DMC_ConnectionSettingsFrame();

    void setSettings(DMC_OptionsContainer* settings);

public slots:
    void logMessage(const QString& message);

protected slots:
    void resetUrl();
    void createUser();
    void userCreated(int requestID, const QString& username, const QString& userId, const QString& email);
    void urlTextChanged(const QString& text);
    void inviteChanged(int index);
    void editInvites();

private:
    void populateInvites();

    Ui::DMC_ConnectionSettingsFrame *ui;

    DMC_OptionsContainer* _settings;
    DMHNetworkManager* _networkManager;
};

#endif // DMC_CONNECTIONSETTINGSFRAME_H
