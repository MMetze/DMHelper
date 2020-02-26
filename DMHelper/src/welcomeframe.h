#ifndef WELCOMEFRAME_H
#define WELCOMEFRAME_H

#include <QFrame>

namespace Ui {
class WelcomeFrame;
}

class MRUHandler;

class WelcomeFrame : public QFrame
{
    Q_OBJECT

public:
    explicit WelcomeFrame(MRUHandler* mruHandler, QWidget *parent = nullptr);
    ~WelcomeFrame();

    void setMRUHandler(MRUHandler* mruHandler);

public slots:
    void openUsersGuide();
    void openGettingStarted();
    void openSampleCampaign();

signals:
    void openCampaignFile(QString campaignFile);

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void openCampaign(const QString& campaignText);

private:
    void openDoc(const QString& docName);
    void openLink(const QString& linkText);

    Ui::WelcomeFrame *ui;

    MRUHandler* _mruHandler;
};

#endif // WELCOMEFRAME_H
