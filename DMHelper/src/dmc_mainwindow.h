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
//class QWebEngineView;
class QLabel;
class QPushButton;

class DMC_MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DMC_MainWindow(QWidget *parent = nullptr);
    ~DMC_MainWindow();

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

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

    QPixmap _labelPixmap;
    //QWebEngineView* _webView;

};

#endif // DMC_MAINWINDOW_H