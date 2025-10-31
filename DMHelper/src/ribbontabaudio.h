#ifndef RIBBONTABAUDIO_H
#define RIBBONTABAUDIO_H

#include "ribbonframe.h"

namespace Ui {
class RibbonTabAudio;
}

class RibbonTabAudio : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonTabAudio(QWidget *parent = nullptr);
    ~RibbonTabAudio();

    virtual PublishButtonRibbon* getPublishRibbon() override;

public slots:
    void setTrackType(int type);
    void setTrackStatus(int status);
    void setRepeat(bool checked);
    void setMute(bool checked);
    void setVolume(float volume);

signals:
    void playClicked();
    void pauseClicked();
    void stopClicked();
    void repeatClicked(bool checked);
    void muteClicked(bool checked);
    void volumeChanged(float volume);

protected slots:
    void handleVolumeChanged(int volume);

protected:
    virtual void showEvent(QShowEvent *event) override;

    void updateButtonStatus();

private:
    Ui::RibbonTabAudio *ui;

    int _trackType;
    int _trackStatus;
};

#endif // RIBBONTABAUDIO_H
