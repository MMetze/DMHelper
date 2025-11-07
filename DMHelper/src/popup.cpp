#include "popup.h"

Popup::Popup(QObject *parent) :
    QObject{parent}
{}

bool Popup::isDMOnly() const
{
    return true;
}
