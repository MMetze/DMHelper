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
    ~AudioTrackEdit();

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

private:
    AudioTrack* getCurrentTrack();
    void addTrackToList(AudioTrack* track);
    void enableButtons(bool enable);

    Ui::AudioTrackEdit *ui;

    Campaign* _campaign;
};

#endif // AUDIOTRACKEDIT_H
