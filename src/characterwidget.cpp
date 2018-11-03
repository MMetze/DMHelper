#include "characterwidget.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include "battledialogmodelcharacter.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QMouseEvent>
#include <QLineEdit>
#include <QIntValidator>
#include <QDebug>

CharacterWidget::CharacterWidget(BattleDialogModelCharacter* character, QWidget *parent) :
    CombatantWidget(parent),
    _character(character),
    pairName(0),
    pairArmorClass(0)
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(2);
    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);
    if(parent)
    {
        resize(parent->width(), DMHelper::CHARACTER_WIDGET_HEIGHT);
    }
    else
    {
        resize(width(), DMHelper::CHARACTER_WIDGET_HEIGHT);
    }

    QHBoxLayout* hLayoutTop = new QHBoxLayout(this);

    QVBoxLayout* vLayout = new QVBoxLayout();

    // Top Line
    pairName = createPairLayout(QString("Name"),_character->getName());
    pairArmorClass = createPairLayout(QString("AC"),QString::number(_character->getArmorClass()));

    // Combine the lines
    vLayout->addLayout(pairName);
    vLayout->addLayout(pairArmorClass);

    // Overall summary
    _lblIcon = new QLabel();
    loadImage();
    hLayoutTop->addWidget(_lblIcon);

    _lblInitName = new QLabel(QString("Init:"), this);
    _lblInitName ->resize(_lblInitName->fontMetrics().width(_lblInitName->text()), _lblInitName->height());
    _lblInitName ->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hLayoutTop->addWidget(_lblInitName);
    _edtInit = new QLineEdit(QString::number(_character->getInitiative()), this);
    _edtInit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    _edtInit->setValidator(new QIntValidator(0,100,this));
    _edtInit->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    _edtInit->setMaximumWidth(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5);
    _edtInit->setMaximumHeight(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5);
    _edtInit->setMinimumWidth(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5);
    _edtInit->setMinimumHeight(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5);
    QFont font = _edtInit->font();
    font.setPointSize(font.pointSize() * 1.5);
    font.setBold(true);
    _edtInit->setFont(font);
    connect(_edtInit,SIGNAL(editingFinished()),this,SLOT(initiativeChanged()));
    hLayoutTop->addWidget(_edtInit);

    hLayoutTop->addLayout(vLayout);

    hLayoutTop->addStretch();

    setLayout(hLayoutTop);

    if(_character->getCombatant())
        connect(_character->getCombatant(),SIGNAL(dirty()),this,SLOT(updateData()));
    else
        qDebug() << "[Character Widget] no valid combatant found!";
}

BattleDialogModelCombatant* CharacterWidget::getCombatant()
{
    return _character;
}

BattleDialogModelCharacter* CharacterWidget::getCharacter()
{
    return _character;
}

void CharacterWidget::updateData()
{
    updatePairData(pairName, _character->getName());
    updatePairData(pairArmorClass, QString::number(_character->getArmorClass()));

    loadImage();

    update();
}

void CharacterWidget::setHighlighted(bool highlighted)
{
    CombatantWidget::setHighlighted(highlighted);

    setPairHighlighted(pairName, highlighted);
    setPairHighlighted(pairArmorClass, highlighted);
}

void CharacterWidget::executeDoubleClick()
{
    if(_character->getCharacter())
        emit clicked(_character->getCharacter()->getID());
    else
        qDebug() << "[Character Widget] no valid character found!";
}

