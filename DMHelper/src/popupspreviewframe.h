#ifndef POPUPSPREVIEWFRAME_H
#define POPUPSPREVIEWFRAME_H

#include <QFrame>
#include "audiotrack.h"

namespace Ui {
class PopupsPreviewFrame;
}

class Campaign;
class CampaignObjectBase;
class Popup;

class PopupsPreviewFrame : public QFrame
{
    Q_OBJECT

public:
    explicit PopupsPreviewFrame(QWidget *parent = nullptr);
    ~PopupsPreviewFrame();

    void setCampaign(Campaign* campaign);

public slots:
    void trackAdded(CampaignObjectBase* trackObject);

protected slots:
    void toggleOverlayFrame();
    void populatePopups();
    void clearPopups();

    void audioPopupChanged(AudioTrack::AudioTrackStatus newState);
    void updateFrameVisibility();

private:
    Ui::PopupsPreviewFrame *ui;

    Campaign* _campaign;
    QHash<Popup*, QFrame*> _popups;
};

#endif // POPUPSPREVIEWFRAME_H
