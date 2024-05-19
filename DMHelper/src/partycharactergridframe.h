#ifndef PARTYCHARACTERGRIDFRAME_H
#define PARTYCHARACTERGRIDFRAME_H

#include <QFrame>
#include <QUuid>

namespace Ui {
class PartyCharacterGridFrame;
}

class Characterv2;

class PartyCharacterGridFrame : public QFrame
{
    Q_OBJECT

public:
    explicit PartyCharacterGridFrame(Characterv2& character, QWidget *parent = nullptr);
    virtual ~PartyCharacterGridFrame();

    void readCharacter();

    QUuid getId() const;

signals:
    void characterSelected(const QUuid& characterId);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void syncDndBeyond();

private:
    Ui::PartyCharacterGridFrame *ui;

    Characterv2& _character;
};

#endif // PARTYCHARACTERGRIDFRAME_H
