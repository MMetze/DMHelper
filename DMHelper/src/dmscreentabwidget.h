#ifndef DMSCREENTABWIDGET_H
#define DMSCREENTABWIDGET_H

#include <QTabWidget>

namespace Ui {
class DMScreenTabWidget;
}

class QLabel;
class QDomElement;
class QLayout;

class DMScreenTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit DMScreenTabWidget(const QString& equipmentFile, QWidget *parent = nullptr);
    ~DMScreenTabWidget();

private:
    void readEquipment(const QString& equipmentFile);
    void readWeaponSection(const QDomElement& section, QLayout& layoutName, QLayout& layoutCost, QLayout& layoutDamage, QLayout& layoutWeight, QLayout& layoutProperties);
    void readArmorSection(const QDomElement& section, int& position);
    int countAllEquipment(const QDomElement& section, QString itemLabel, QString subitemLabel);
    void readSimpleSection(QDomElement& root, QLabel* sectionLabel, QString sectionName, QString itemLabel, QString subitemLabel, QLayout& layout1, QString name1, QLayout& layout2, QString name2);
    void readSimpleSection(QDomElement& root, QLabel* sectionLabel, QString sectionName, QString itemLabel, QString subitemLabel, QLayout& layout1, QString name1, QLayout& layout2, QString name2, QLayout& layout3, QString name3);
    void readSimpleSection(QDomElement& root, QLabel* sectionLabel, QString sectionName, QString itemLabel, QString subitemLabel, QLayout& layout1, QString name1, QLayout& layout2, QString name2, QLayout& layout3, QString name3, QLayout& layout4, QString name4);
    void readEquipmentSection(QDomElement& element, int itemCount, QString itemLabel, QString subitemLabel, QLayout& layout1, QString name1, QLayout& layout2, QString name2);
    void readEquipmentSection(QDomElement& element, int itemCount, QString itemLabel, QString subitemLabel, QLayout& layout1, QString name1, QLayout& layout2, QString name2, QLayout& layout3, QString name3);
    void readEquipmentSection(QDomElement& element, int itemCount, QString itemLabel, QString subitemLabel, QLayout& layout1, QString name1, QLayout& layout2, QString name2, QLayout& layout3, QString name3, QLayout& layout4, QString name4);
    QLabel* createLabel(const QString& label, bool centered = false, bool italics = false, bool title = false, bool indented = false);
    void readSectionTitle(QDomElement& element, QLabel* sectionTitle);
    void readColumnTitle(QDomElement& element, QString columnName, QLayout& sectionLayout);

    Ui::DMScreenTabWidget *ui;
};

#endif // DMSCREENTABWIDGET_H
