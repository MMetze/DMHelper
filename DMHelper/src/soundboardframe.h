#ifndef SOUNDBOARDFRAME_H
#define SOUNDBOARDFRAME_H

#include <QFrame>

namespace Ui {
class SoundboardFrame;
}

class Campaign;
class QVBoxLayout;
class SoundBoardFrameGroupBox;
class SoundboardGroup;
class AudioTrack;

class SoundboardFrame : public QFrame
{
    Q_OBJECT

public:
    explicit SoundboardFrame(QWidget *parent = nullptr);
    ~SoundboardFrame();

public slots:
    void setCampaign(Campaign* campaign);
    void addTrackToTree(AudioTrack* track);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

    void updateTrackLayout();

protected slots:
    void addGroup();

    void addSound();
    void addYoutube();
    void addSyrinscape();

    void removeSound();

    void addTrack(const QUrl& url);

private:
    void addGroupToLayout(SoundboardGroup* group);

    Ui::SoundboardFrame *ui;

    QVBoxLayout* _layout;
    QList<SoundBoardFrameGroupBox*> _groupList;
    bool _mouseDown;
    QPoint _mouseDownPos;
    Campaign* _campaign;
};

#endif // SOUNDBOARDFRAME_H
