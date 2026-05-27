#ifndef DMHMESSAGEBOX_H
#define DMHMESSAGEBOX_H

#include <QMessageBox>

class QWidget;
class QString;

class DMHMessageBox
{
public:
    static QMessageBox::StandardButton critical(QWidget* parent,
                                                const QString& title,
                                                const QString& text,
                                                QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                                QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    static QMessageBox::StandardButton warning(QWidget* parent,
                                               const QString& title,
                                               const QString& text,
                                               QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                               QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    static QMessageBox::StandardButton information(QWidget* parent,
                                                   const QString& title,
                                                   const QString& text,
                                                   QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                                   QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    static QMessageBox::StandardButton question(QWidget* parent,
                                                const QString& title,
                                                const QString& text,
                                                QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
                                                QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

private:
    static QMessageBox::StandardButton showDialog(QWidget* parent,
                                                  const QString& title,
                                                  const QString& text,
                                                  QMessageBox::StandardButtons buttons,
                                                  QMessageBox::StandardButton defaultButton);
};

#endif // DMHMESSAGEBOX_H
