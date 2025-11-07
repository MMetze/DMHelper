#ifndef POPUPAUDIO_H
#define POPUPAUDIO_H

#include <QObject>
#include "popup.h"

class AudioTrackFile;
class QPushButton;

class PopupAudio : public Popup
{
    Q_OBJECT
public:
    explicit PopupAudio(AudioTrackFile* track, QObject *parent = nullptr);
    virtual ~PopupAudio() override;

    // Local interface
    virtual void prepareFrame(QBoxLayout* frameLayout, int insertIndex) override;

    AudioTrackFile* getTrack() const;
    QPushButton* getPlayButton(QBoxLayout* frameLayout, int insertIndex);

protected slots:
    void playButtonToggled(bool checked);
    void muteButtonToggled(bool checked);
    void clearButtons();

protected:
    AudioTrackFile* _track;

    QPushButton* _btnPlay;
    QPushButton* _btnMute;
    QPushButton* _btnRepeat;
};

#endif // POPUPAUDIO_H
