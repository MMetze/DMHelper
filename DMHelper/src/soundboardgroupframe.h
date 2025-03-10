#ifndef SOUNDBOARDGROUPFRAME_H
#define SOUNDBOARDGROUPFRAME_H

#include <QFrame>
#include "soundboardgroup.h"

namespace Ui {
class SoundBoardGroupFrame;
}

class SoundboardTrackFrame;
class Campaign;

class SoundBoardGroupFrame : public QFrame
{
    Q_OBJECT

public:
    explicit SoundBoardGroupFrame(SoundboardGroup* group, Campaign* campaign, QWidget *parent = nullptr);
    ~SoundBoardGroupFrame();

    bool isMuted() const;
    int getVolume() const;
    SoundboardGroup* getGroup() const;

public slots:
    void updateTrackLayout();
    void addTrack(SoundboardTrack* track);
    void removeTrack(SoundboardTrack* track);
    void setMute(bool mute);
    void trackMuteChanged(bool mute);
    void handleRemove();

signals:
    void muteChanged(bool mute);
    void overrideChildMute(bool mute);
    void volumeChanged(int volume);
    void dirty();
    void removeGroup(SoundboardGroup* group);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

protected slots:
    virtual void toggleContents();
    virtual void toggleMute();

private:
    void addTrackToLayout(SoundboardTrack* track);

    Ui::SoundBoardGroupFrame *ui;

    QList<SoundboardTrackFrame*> _trackWidgets;
    bool _localMute;

    SoundboardGroup* _group;
    Campaign* _campaign;
};

#endif // SOUNDBOARDGROUPFRAME_H
