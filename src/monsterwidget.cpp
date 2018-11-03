#include "monsterwidget.h"
#include "battledialogmodel.h"
#include "battledialogmodelmonsterbase.h"
#include "monsterclass.h"
#include "combatant.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QMouseEvent>
#include <QLineEdit>
#include <QIntValidator>
#include <QDebug>

MonsterWidget::MonsterWidget(BattleDialogModelMonsterBase* monster, QWidget *parent) :
    CombatantWidget(parent),
    _monster(monster),
    _pairName(0),
    _pairArmorClass(0),
    _lblHP(0),
    _edtHP(0),
    _chkVisible(0),
    _chkKnown(0)
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(2);

    QHBoxLayout* hLayout = new QHBoxLayout(this);

    QVBoxLayout* vLayoutChecks = new QVBoxLayout();
    // Visibility icons
    QHBoxLayout* hLayoutVisible = new QHBoxLayout();
    QLabel* iconVisible = new QLabel();
    //iconVisible->resize(DMHelper::CHARACTER_ICON_WIDTH, DMHelper::CHARACTER_ICON_WIDTH);
    QImage imgVisible;
    imgVisible.load(":/img/data/small_icon_eye.png");
    iconVisible->setPixmap(QPixmap::fromImage(imgVisible));
    iconVisible->setStyleSheet("background-image: url();");
    _chkVisible = new QCheckBox();
    _chkVisible->setChecked(true);
    hLayoutVisible->addWidget(iconVisible);
    hLayoutVisible->addWidget(_chkVisible);
    vLayoutChecks->addLayout(hLayoutVisible);

    QHBoxLayout* hLayoutKnown = new QHBoxLayout();
    QLabel* iconKnown= new QLabel();
    //iconKnown->resize(DMHelper::CHARACTER_ICON_WIDTH, DMHelper::CHARACTER_ICON_WIDTH);
    QImage imgKnown;
    imgKnown.load(":/img/data/small_icon_secret.png");
    iconKnown->setPixmap(QPixmap::fromImage(imgKnown));
    iconKnown->setStyleSheet("background-image: url();");
    _chkKnown = new QCheckBox();
    _chkKnown->setChecked(true);
    hLayoutKnown->addWidget(iconKnown);
    hLayoutKnown->addWidget(_chkKnown);
    vLayoutChecks->addLayout(hLayoutKnown);

    //vLayoutChecks->addLayout(hLayoutVisible);
    //vLayoutChecks->addLayout(hLayoutKnown);
    //vLayoutChecks->addWidget(iconKnown);
    //vLayoutChecks->addWidget(_chkVisible);
    //vLayoutChecks->addWidget(_chkKnown);

    hLayout->addLayout(vLayoutChecks);

    // Overall summary
    _lblIcon = new QLabel();
    loadImage();
    hLayout->addWidget(_lblIcon);

    // Bottom Line
    _lblHP = new QLabel(QString("HP:"), this);
    _lblHP->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hLayout->addWidget(_lblHP);
    _edtHP = new QLineEdit(QString::number(_monster->getHitPoints()), this);
    _edtHP->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _edtHP->setValidator(new QIntValidator(-10,1000,this));
    connect(_edtHP,SIGNAL(editingFinished()),this,SLOT(localHitPointsChanged()));
    _edtHP->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    _edtHP->setMaximumWidth(DMHelper::CHARACTER_WIDGET_HEIGHT*2);
    _edtHP->setMinimumWidth(DMHelper::CHARACTER_WIDGET_HEIGHT*2);
    _edtHP->setMaximumHeight(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5);
    _edtHP->setMinimumHeight(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5);
    QFont font = _edtHP->font();
    font.setPointSize(font.pointSize() * 1.5);
    font.setBold(true);
    _edtHP->setFont(font);
    hLayout->addWidget(_edtHP);

    _lblInitName = new QLabel(QString("Init:"), this);
    _lblInitName->resize(_lblInitName->fontMetrics().width(_lblInitName->text()), _lblInitName->height());
    _lblInitName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hLayout->addWidget(_lblInitName);
    _edtInit = new QLineEdit(QString::number(_monster->getInitiative()), this);
    _edtInit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    _edtInit->setValidator(new QIntValidator(0,100,this));
    _edtInit->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    _edtInit->setMaximumWidth(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5);
    _edtInit->setMaximumHeight(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5);
    _edtInit->setMinimumWidth(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5);
    _edtInit->setMinimumHeight(DMHelper::CHARACTER_WIDGET_HEIGHT*1.5);
    font = _edtInit->font();
    font.setPointSize(font.pointSize() * 1.5);
    font.setBold(true);
    _edtInit->setFont(font);
    connect(_edtInit,SIGNAL(editingFinished()),this,SLOT(initiativeChanged()));
    hLayout->addWidget(_edtInit);

    QVBoxLayout* vLayout = new QVBoxLayout();

    _pairName = createPairLayout(QString("Name"),_monster->getName());
    vLayout->addLayout(_pairName);
    _pairArmorClass = createPairLayout(QString("AC"),QString::number(_monster->getArmorClass()));
    vLayout->addLayout(_pairArmorClass);

    // Combine the lines
    hLayout->addLayout(vLayout);

    hLayout->addStretch();

    setLayout(hLayout);

    if(_monster->getCombatant())
        connect(_monster->getCombatant(),SIGNAL(dirty()),this,SLOT(updateData()));
    else if (_monster->getMonsterClass())
        connect(_monster->getMonsterClass(),SIGNAL(dirty()),this,SLOT(updateData()));
    else
        qDebug() << "[Monster Widget] neither valid combatant nor monster class found!";

    connect(_chkVisible,SIGNAL(clicked(bool)),this,SIGNAL(isShownChanged(bool)));
    connect(_chkKnown,SIGNAL(clicked(bool)),this,SIGNAL(isKnownChanged(bool)));
}

BattleDialogModelCombatant* MonsterWidget::getCombatant()
{
    return _monster;
}

BattleDialogModelMonsterBase* MonsterWidget::getMonster()
{
    return _monster;
}

bool MonsterWidget::isShown()
{
    return _chkVisible->isChecked();
}

bool MonsterWidget::isKnown()
{
    return _chkKnown->isChecked();
}

void MonsterWidget::updateData()
{
    updatePairData(_pairName, _monster->getName());
    updatePairData(_pairArmorClass, QString::number(_monster->getArmorClass()));
    _edtHP->setText(QString::number(_monster->getHitPoints()));

    loadImage();

    update();
}

void MonsterWidget::setHighlighted(bool highlighted)
{
    CombatantWidget::setHighlighted(highlighted);

    setPairHighlighted(_pairName, highlighted);
    setPairHighlighted(_pairArmorClass, highlighted);
    setWidgetHighlighted(_lblHP, highlighted);
    setWidgetHighlighted(_edtHP, highlighted);
}

void MonsterWidget::localHitPointsChanged()
{
    _monster->setHitPoints(_edtHP->text().toInt());
    emit hitPointsChanged(_monster->getHitPoints());
}

void MonsterWidget::executeDoubleClick()
{
    if(_monster->getMonsterClass())
        emit clicked(_monster->getMonsterClass()->getName());
    else
        qDebug() << "[Monster Widget] no valid monster class found!";
}
