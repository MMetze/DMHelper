#ifndef SOUNDBOARDFRAMEGROUPBOX_H
#define SOUNDBOARDFRAMEGROUPBOX_H

#include <QGroupBox>

namespace Ui {
class SoundBoardFrameGroupBox;
}

class SoundboardTrack;
class QGridLayout;
class Campaign;

class SoundBoardFrameGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    explicit SoundBoardFrameGroupBox(const QString& groupName, Campaign* campaign, QWidget *parent = nullptr);
    ~SoundBoardFrameGroupBox();

    bool isMuted() const;
    int getVolume() const;

public slots:
    void updateTrackLayout();
    void clearTracks();
    void addTrack(SoundboardTrack* track);
    void setMute(bool mute);
    void trackMuteChanged(bool mute);

signals:
    void muteChanged(bool mute);
    void overrideChildMute(bool mute);
    void volumeChanged(int volume);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

protected slots:
    virtual void toggleContents();
    virtual void toggleMute();

private:
    Ui::SoundBoardFrameGroupBox *ui;

    QGridLayout* _groupLayout;
    QList<SoundboardTrack*> _trackWidgets;
    bool _localMute;

    Campaign* _campaign;
};

#endif // SOUNDBOARDFRAMEGROUPBOX_H
