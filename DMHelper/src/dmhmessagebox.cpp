#include "dmhmessagebox.h"

#include <QIcon>

namespace
{
    static const char* DMH_MESSAGEBOX_WINDOW_ICON = ":/img/data/dmhelper_small.png";
}

QMessageBox::StandardButton DMHMessageBox::critical(QWidget* parent,
                                                    const QString& title,
                                                    const QString& text,
                                                    QMessageBox::StandardButtons buttons,
                                                    QMessageBox::StandardButton defaultButton)
{
    return showDialog(parent, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton DMHMessageBox::warning(QWidget* parent,
                                                   const QString& title,
                                                   const QString& text,
                                                   QMessageBox::StandardButtons buttons,
                                                   QMessageBox::StandardButton defaultButton)
{
    return showDialog(parent, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton DMHMessageBox::information(QWidget* parent,
                                                       const QString& title,
                                                       const QString& text,
                                                       QMessageBox::StandardButtons buttons,
                                                       QMessageBox::StandardButton defaultButton)
{
    return showDialog(parent, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton DMHMessageBox::question(QWidget* parent,
                                                    const QString& title,
                                                    const QString& text,
                                                    QMessageBox::StandardButtons buttons,
                                                    QMessageBox::StandardButton defaultButton)
{
    return showDialog(parent, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton DMHMessageBox::showDialog(QWidget* parent,
                                                      const QString& title,
                                                      const QString& text,
                                                      QMessageBox::StandardButtons buttons,
                                                      QMessageBox::StandardButton defaultButton)
{
    QMessageBox messageBox(parent);
    messageBox.setWindowTitle(title);
    messageBox.setText(text);
    messageBox.setIconPixmap(QIcon(DMH_MESSAGEBOX_WINDOW_ICON).pixmap(64, 64));
    messageBox.setWindowIcon(QIcon(DMH_MESSAGEBOX_WINDOW_ICON));
    messageBox.setStandardButtons(buttons);

    if(defaultButton != QMessageBox::NoButton)
        messageBox.setDefaultButton(defaultButton);

    return static_cast<QMessageBox::StandardButton>(messageBox.exec());
}
