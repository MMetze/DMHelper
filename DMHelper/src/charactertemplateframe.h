#ifndef CHARACTERTEMPLATEFRAME_H
#define CHARACTERTEMPLATEFRAME_H

#include "campaignobjectframe.h"
#include "templateframe.h"
#include "characterv2.h"

namespace Ui {
class CharacterTemplateFrame;
}

class OptionsContainer;
class QLineEdit;
class QTextEdit;
class QGridLayout;

class CharacterTemplateFrame : public CampaignObjectFrame, public TemplateFrame
{
    Q_OBJECT

public:
    explicit CharacterTemplateFrame(OptionsContainer* options, QWidget *parent = nullptr);
    ~CharacterTemplateFrame();

    // Public interface from CampaignObjectFrame
    virtual void activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer) override;
    virtual void deactivateObject() override;

    void setCharacter(Characterv2* character);
    void setHeroForgeToken(const QString& token);

signals:
    void publishCharacterImage(QImage img);
    void characterChanged();
    void heroForgeTokenChanged(const QString& token);

public slots:
    // Publish slots from CampaignObjectFrame
    virtual void loadCharacterUITemplate(const QString& templateFile);
    virtual void publishClicked(bool checked) override;
    virtual void setRotation(int rotation) override;
    virtual void setBackgroundColor(const QColor& color) override;

protected:
    // From CampaignObjectFrame
    virtual bool eventFilter(QObject *object, QEvent *event) override;
    virtual void mousePressEvent(QMouseEvent * event) override;
    virtual void mouseReleaseEvent(QMouseEvent * event) override;

    // From TemplateFrame
    virtual QObject* getFrameObject() override;

private slots:
    void readCharacterData();
    void handlePublishClicked();
    void editCharacterIcon();
    void syncDndBeyond();
    void importHeroForge();
    void updateCharacterName();
    void handlePreviousToken();
    void handleNextToken();
    void handleAddToken();
    void handleReloadImage();
    void handleClearImage();
    void editConditions();
    void clearConditions();

private:
    void loadCharacterImage();
    void enableDndBeyondSync(bool enabled);
    void setTokenIndex(int index);
    void updateConditionLayout();
    void clearConditionGrid();
    void addCondition(const QString& conditionId);

    Ui::CharacterTemplateFrame *ui;
    QWidget *_uiWidget;

    OptionsContainer* _options;
    Characterv2* _character;
    bool _mouseDown;
    bool _reading;
    int _rotation;
    int _currentToken;
    QString _heroForgeToken;
    QGridLayout* _conditionGrid;
};

#endif // CHARACTERTEMPLATEFRAME_H
