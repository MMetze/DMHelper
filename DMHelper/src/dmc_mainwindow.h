#ifndef DMC_MAINWINDOW_H
#define DMC_MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class DMC_MainWindow;
}

class DMC_OptionsContainer;
class DMC_ServerConnection;
class DMC_SettingsDialog;
class QCloseEvent;
class QResizeEvent;
class AudioTrack;
class QLabel;
class QPushButton;

class DMC_MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DMC_MainWindow(QWidget *parent = nullptr);
    virtual ~DMC_MainWindow() override;

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void connectToggled(bool checked);
    void muteToggled(bool checked);
    void enableAudio(AudioTrack* track);
    void openOptions();
    void setLabelPixmap(QPixmap pixmap);
    void setLabelImage(QImage image);
    void updatePixmap();

private:
    void setStandardButtonSize(QLabel& label, QPushButton& button, int frameHeight);

    Ui::DMC_MainWindow *ui;
    DMC_OptionsContainer* _settings;
    DMC_ServerConnection* _serverConnection;
    DMC_SettingsDialog* _settingsDlg;

    QPixmap _labelPixmap;


};

#endif // DMC_MAINWINDOW_H
