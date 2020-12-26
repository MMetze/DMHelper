#ifndef ENCOUNTERTEXTEDIT_H
#define ENCOUNTERTEXTEDIT_H

#include "campaignobjectframe.h"
#include "texteditformatterframe.h"

namespace Ui {
class EncounterTextEdit;
}

class EncounterText;

class EncounterTextEdit : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit EncounterTextEdit(QWidget *parent = nullptr);
    virtual ~EncounterTextEdit() override;

    virtual void activateObject(CampaignObjectBase* object) override;
    virtual void deactivateObject() override;

    void setKeys(const QList<QString>& keys);
    QList<QString> keys();

    EncounterText* getEncounter() const;
    void setEncounter(EncounterText* encounter);

    QString toHtml() const;
    QString toPlainText() const;

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void clear();
    void setHtml(const QString &text);
    void setPlainText(const QString &text);

    void setFont(const QString& fontFamily);
    void setFontSize(int fontSize);
    void setBold(bool bold);
    void setItalics(bool italics);
    void setColor(QColor color);
    void setAlignment(Qt::Alignment alignment);

    void hyperlinkClicked();

signals:
//    void textChanged();
    void anchorClicked(const QUrl &link);

    void fontFamilyChanged(const QString& fontFamily);
    void fontSizeChanged(int fontSize);
    void fontBoldChanged(bool fontBold);
    void fontItalicsChanged(bool fontItalics);
    void alignmentChanged(Qt::Alignment alignment);
    void colorChanged(QColor color);

    void setHyperlinkActive(bool active);

protected slots:
    void storeEncounter();
    void readEncounter();

    void takeFocus();

protected:
    Ui::EncounterTextEdit *ui;

    QList<QString> _keys;
    EncounterText* _encounter;
    TextEditFormatterFrame* _formatter;
};

#endif // ENCOUNTERTEXTEDIT_H
