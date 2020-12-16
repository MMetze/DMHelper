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

    virtual void activateObject(CampaignObjectBase* object) override;
    virtual void deactivateObject() override;

    void setTrack(AudioTrack* track);

signals:
    void trackTypeChanged(int type);
    void playChanged(bool checked);
    void repeatChanged(bool checked);
    void muteChanged(bool checked);
    void volumeChanged(int volume);

public slots:
    void setPlay(bool checked);
    void setRepeat(bool checked);
    void setMute(bool checked);
    void setVolume(int volume);

private slots:
    void unclick();

/*
signals:
    void trackSelected(AudioTrack* track);

public slots:
    void addTrack(const QUrl& url);
    void removeTrack();

    void setCampaign(Campaign* campaign);

private slots:
    void addLocalFile();
    void addGlobalUrl();
    void addSyrinscape();
    void itemSelected(QListWidgetItem *item);
*/
private:
/*
    AudioTrack* getCurrentTrack();
    void addTrackToList(AudioTrack* track);
    void enableButtons(bool enable);
*/

    Ui::AudioTrackEdit *ui;

    //Campaign* _campaign;
    AudioTrack* _track;
};

#endif // AUDIOTRACKEDIT_H
