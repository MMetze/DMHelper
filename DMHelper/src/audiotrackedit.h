#ifndef AUDIOTRACKEDIT_H
#define AUDIOTRACKEDIT_H

#include "campaignobjectframe.h"

namespace Ui {
class AudioTrackEdit;
}

class Campaign;
class AudioTrack;
class QListWidgetItem;

class AudioTrackEdit : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit AudioTrackEdit(QWidget *parent = nullptr);
    virtual ~AudioTrackEdit() override;

    virtual void activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer) override;
    virtual void deactivateObject() override;

    void setTrack(AudioTrack* track);

signals:
    void trackTypeChanged(int type);
    void trackStatusChanged(int status);
    void repeatChanged(bool checked);
    void muteChanged(bool checked);
    void volumeChanged(float volume);

public slots:
    void play();
    void pause();
    void stop();
    void setRepeat(bool checked);
    void setMute(bool checked);
    void setVolume(float volume);

protected slots:
//    void unclick();

protected:
    Ui::AudioTrackEdit *ui;

    AudioTrack* _track;
};

#endif // AUDIOTRACKEDIT_H
