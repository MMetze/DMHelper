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
    pairName(nullptr),
    pairArmorClass(nullptr),
    _edtHP(nullptr)
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

    if(_character)
    {
        QHBoxLayout* hLayoutTop = new QHBoxLayout(this);

        QVBoxLayout* vLayout = new QVBoxLayout();

        // Top Line
        pairName = createPairLayout(QString("Name"),_character->getName());

        // Bottom Line
        QHBoxLayout* hBottomLine = new QHBoxLayout();
        pairArmorClass = createPairLayout(QString("AC"),QString::number(_character->getArmorClass()));
        hBottomLine->addLayout(pairArmorClass);

        QLabel* lblHP = new QLabel(QString("HP:"), this);
        lblHP->resize(lblHP->fontMetrics().width(lblHP->text()) * 2, lblHP->height());
        lblHP->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        hBottomLine->addWidget(lblHP);
        _edtHP = new QLineEdit(QString::number(_character->getHitPoints()), this);
        _edtHP->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        _edtHP->setValidator(new QIntValidator(0,100,this));
        _edtHP->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        _edtHP->setMaximumWidth(static_cast<int>(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5));
        _edtHP->setMaximumHeight(static_cast<int>(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5));
        _edtHP->setMinimumWidth(static_cast<int>(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5));
        _edtHP->setMinimumHeight(static_cast<int>(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5));
        connect(_edtHP,SIGNAL(editingFinished()),this,SLOT(hitPointsEdited()));
        hBottomLine->addWidget(_edtHP);
        _edtHP->setValidator(new QIntValidator(-100,100000,this));

        // Combine the lines
        vLayout->addLayout(pairName);
        vLayout->addLayout(hBottomLine);

        // Overall summary
        _lblIcon = new QLabel();
        loadImage();
        hLayoutTop->addWidget(_lblIcon);

        _lblInitName = new QLabel(QString("Init:"), this);
        _lblInitName->resize(_lblInitName->fontMetrics().width(_lblInitName->text()), _lblInitName->height());
        _lblInitName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        hLayoutTop->addWidget(_lblInitName);
        _edtInit = new QLineEdit(QString::number(_character->getInitiative()), this);
        _edtInit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        _edtInit->setValidator(new QIntValidator(0,100,this));
        _edtInit->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        _edtInit->setMaximumWidth(static_cast<int>(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5));
        _edtInit->setMaximumHeight(static_cast<int>(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5));
        _edtInit->setMinimumWidth(static_cast<int>(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5));
        _edtInit->setMinimumHeight(static_cast<int>(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5));
        QFont font = _edtInit->font();
        font.setPointSize(static_cast<int>(font.pointSize() * 1.5));
        font.setBold(true);
        _edtInit->setFont(font);
        connect(_edtInit,SIGNAL(editingFinished()),this,SLOT(initiativeChanged()));
        hLayoutTop->addWidget(_edtInit);
        _edtInit->setValidator(new QIntValidator(-100,100000,this));

        hLayoutTop->addLayout(vLayout);

        hLayoutTop->addStretch();

        setLayout(hLayoutTop);

        if(_character->getCombatant())
            connect(_character->getCombatant(),SIGNAL(dirty()),this,SLOT(updateData()));
        else
            qDebug() << "[Character Widget] no valid combatant found!";
    }
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
    if(!_character)
        return;

    updatePairData(pairName, _character->getName());
    updatePairData(pairArmorClass, QString::number(_character->getArmorClass()));
    _edtHP->setText(QString::number(_character->getHitPoints()));

    loadImage();

    update();
}

void CharacterWidget::setHighlighted(bool highlighted)
{
    CombatantWidget::setHighlighted(highlighted);

    setPairHighlighted(pairName, highlighted);
    setPairHighlighted(pairArmorClass, highlighted);
}

void CharacterWidget::hitPointsEdited()
{
    if((_edtHP)&&(_character))
    {
        _character->setHitPoints(_edtHP->text().toInt());
    }
}

void CharacterWidget::executeDoubleClick()
{
    if((_character) && (_character->getCharacter()))
        emit clicked(_character->getCharacter()->getID());
    else
        qDebug() << "[Character Widget] no valid character found!";
}

