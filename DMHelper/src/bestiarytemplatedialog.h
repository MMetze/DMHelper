#ifndef BESTIARYTEMPLATEDIALOG_H
#define BESTIARYTEMPLATEDIALOG_H

#include <QDialog>
#include "templateframe.h"
#include "bestiaryfindtokendialog.h"

class MonsterClassv2;
class MonsterAction;
class Bestiary;
class TokenEditor;
class OptionsContainer;
class QLineEdit;
class QVBoxLayout;

namespace Ui {
class BestiaryTemplateDialog;
}

class BestiaryTemplateDialog : public QDialog, public TemplateFrame
{
    Q_OBJECT

public:
    explicit BestiaryTemplateDialog(QWidget *parent = nullptr);
    ~BestiaryTemplateDialog();

    void loadMonsterUITemplate(const QString& templateFile);

    MonsterClassv2* getMonster() const;

    void setOptions(OptionsContainer* options);

signals:
    void monsterChanged();
    void publishMonsterImage(QImage img, const QColor& color);
    void dialogClosed();

public slots:
    void setMonster(MonsterClassv2* monster, bool edit = true);
    void setMonster(const QString& monsterName, bool edit = true);
    void createNewMonster();
    void deleteCurrentMonster();

    void previousMonster();
    void nextMonster();

    void dataChanged();

protected slots:
    void monsterRenamed();
    void handlePublishButton();
    void handleBackgroundColorChanged(const QColor& color);

    void handlePreviousToken();
    void handleAddToken();
    void handleEditToken();
    void handleSearchToken();
    void handleReloadImage();
    void handleClearImage();
    void handleNextToken();

    void handlePopulateTokens();
    void loadMonsterImage();

protected:
    // From QWidget
    virtual bool eventFilter(QObject* object, QEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void showEvent(QShowEvent* event) override;
    virtual void hideEvent(QHideEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

    // From TemplateFrame
    virtual QObject* getFrameObject() override;

    void createTokenFiles(BestiaryFindTokenDialog* dialog);

private:
    QString selectToken();
    void setTokenIndex(int index);

    void connectSpecialSignals();
    QLineEdit* getValueEdit(const QString& key);

    Ui::BestiaryTemplateDialog *ui;
    QWidget *_uiWidget;

    OptionsContainer* _options;
    MonsterClassv2* _monster;
    int _currentToken;
    bool _edit;
    bool _mouseDown;

    // Token search data
    QString _searchString;

};

#endif // BESTIARYTEMPLATEDIALOG_H
