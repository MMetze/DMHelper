#ifndef CHARACTERTEMPLATEFRAME_H
#define CHARACTERTEMPLATEFRAME_H

#include "campaignobjectframe.h"
#include "characterv2.h"

namespace Ui {
class CharacterTemplateFrame;
}

class OptionsContainer;
class QLineEdit;
class QTextEdit;

class CharacterTemplateFrame : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit CharacterTemplateFrame(OptionsContainer* options, QWidget *parent = nullptr);
    ~CharacterTemplateFrame();

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
    virtual void publishClicked(bool checked) override;
    virtual void setRotation(int rotation) override;

protected:
    virtual void mousePressEvent(QMouseEvent * event) override;
    virtual void mouseReleaseEvent(QMouseEvent * event) override;

private slots:
    void disconnectTemplate();
    void readCharacterData();
    void writeCharacterData();
    void handlePublishClicked();
    void editCharacterIcon();
    void syncDndBeyond();
    void importHeroForge();
    void updateCharacterName();

    void handleLineEditFinished(QLineEdit* lineEdit);
    void handleTextEditChanged(QTextEdit* textEdit);
    void handleResourceChanged(QFrame* resourceFrame);

private:
    void loadCharacterImage();
    void enableDndBeyondSync(bool enabled);

    Ui::CharacterTemplateFrame *ui;
    QWidget *_uiWidget;

    OptionsContainer* _options;
    Characterv2* _character;
    bool _mouseDown;
    bool _reading;
    int _rotation;
    QString _heroForgeToken;
};

#endif // CHARACTERTEMPLATEFRAME_H
