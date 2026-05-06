#ifndef SPELLBOOKTEMPLATEDIALOG_H
#define SPELLBOOKTEMPLATEDIALOG_H

#include <QDialog>
#include "templateframe.h"

class Spellv2;
class QLineEdit;

namespace Ui {
class SpellbookTemplateDialog;
}

// Designer-driven spellbook editor. Mirrors BestiaryTemplateDialog but without
// the per-monster token gallery / publish controls — spells have no icon set.
class SpellbookTemplateDialog : public QDialog, public TemplateFrame
{
    Q_OBJECT

public:
    explicit SpellbookTemplateDialog(QWidget *parent = nullptr);
    ~SpellbookTemplateDialog();

    void loadSpellUITemplate(const QString& templateFile);

    Spellv2* getSpell() const;

signals:
    void spellChanged();
    void dialogClosed();

public slots:
    void setSpell(Spellv2* spell, bool edit = true);
    void setSpell(const QString& spellName, bool edit);
    void setSpell(const QString& spellName);
    void createNewSpell();
    void deleteCurrentSpell();

    void previousSpell();
    void nextSpell();

    void dataChanged();

protected slots:
    void spellRenamed();

protected:
    // From QWidget
    virtual void showEvent(QShowEvent* event) override;
    virtual void hideEvent(QHideEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

    // From TemplateFrame
    virtual QObject* getFrameObject() override;
    virtual void postLoadConfiguration(QWidget* owner, QWidget* uiWidget) override;

private:
    QLineEdit* getValueEdit(const QString& key);

    Ui::SpellbookTemplateDialog *ui;
    QWidget *_uiWidget;

    Spellv2* _spell;
    bool _edit;
};

#endif // SPELLBOOKTEMPLATEDIALOG_H
