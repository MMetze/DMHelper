#ifndef DMC_MAINWINDOW_H
#define DMC_MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class DMC_MainWindow;
}

class DMC_OptionsContainer;
class DMC_ServerConnection;
class QCloseEvent;
class QResizeEvent;
class AudioTrack;

class DMC_MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DMC_MainWindow(QWidget *parent = nullptr);
    ~DMC_MainWindow();

protected:
  void closeEvent(QCloseEvent *event);
  void resizeEvent(QResizeEvent *event);

private slots:
    void muteToggled(bool checked);
    void enableAudio(AudioTrack* track);
    void openOptions();

private:
    Ui::DMC_MainWindow *ui;
    DMC_OptionsContainer* _settings;
    DMC_ServerConnection* _serverConnection;

};

#endif // DMC_MAINWINDOW_H
