#ifndef SOUNDBOARDBUTTON_H
#define SOUNDBOARDBUTTON_H

#include <QObject>
#include <QPushButton>

class AudioTrack;
class Campaign;

class SoundboardButton : public QPushButton
{
    Q_OBJECT
public:
    explicit SoundboardButton(Campaign* campaign = nullptr, const QString& iconResource = QString(), AudioTrack* track = nullptr, QWidget *parent = nullptr);
    virtual ~SoundboardButton();

    QString getIconResource() const;
    AudioTrack* getTrack() const;

signals:
    void iconResourceChanged(const QString& iconResource);
    void trackChanged(AudioTrack* track);

public slots:
    void setIconResource(const QString& iconResource);
    void setTrack(AudioTrack* track);

protected slots:
    void handleClick();
    void playingChanged(bool playing);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

    void showContextMenu(const QPoint &pos);
    void changeIcon();
    void changeTrack();
    void updateIcon(bool playing);

    Campaign* _campaign;
    QString _iconResource;
    AudioTrack* _track;
};

#endif // SOUNDBOARDBUTTON_H
