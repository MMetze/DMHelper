#include "chaserwidget.h"
#include "chaser.h"
#include "combatant.h"
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QGraphicsTextItem>
#include <QIntValidator>

ChaserWidget::ChaserWidget(ChaserCombatant* chaser, QWidget *parent) :
    QFrame(parent),
    nameEdit(0),
    speedEdit(0),
    dashEdit(0),
    positionEdit(0),
    initiativeEdit(0),
    exhaustion1(0),
    exhaustion2(0),
    exhaustion3(0),
    exhaustion4(0),
    exhaustion5(0),
    _chaser(chaser)
{
    buildWidget();
}

ChaserCombatant* ChaserWidget::getChaser() const
{
    return _chaser;
}

void ChaserWidget::resetStyleSheet()
{
    setLineWidth(2);

    if(_chaser->isPursuer())
    {
        setStyleSheet("background-color: rgb(128, 255, 128);");
    }
    else
    {
        setStyleSheet("background-color: rgb(255, 128, 128);");
    }
}

void ChaserWidget::readData()
{
    nameEdit->setText(_chaser->getName());
    speedEdit->setText(QString::number(_chaser->getSpeed()));
    initiativeEdit->setText(QString::number(_chaser->getInitiative()));
    dashEdit->setText(QString::number(_chaser->getDashCount()));
    positionEdit->setText(QString::number(_chaser->getPosition()));

    exhaustion1->setChecked(_chaser->getExhaustion() >= 1);
    exhaustion2->setChecked(_chaser->getExhaustion() >= 2);
    exhaustion3->setChecked(_chaser->getExhaustion() >= 3);
    exhaustion4->setChecked(_chaser->getExhaustion() >= 4);
    exhaustion5->setChecked(_chaser->getExhaustion() >= 5);
}

void ChaserWidget::positionEdited()
{
    int editPosition = positionEdit->text().toInt();
    if( _chaser->getPosition() != editPosition)
    {
        _chaser->setPosition(editPosition);
        emit positionChanged(editPosition);
    }
}

void ChaserWidget::initiativeEdited()
{
    int editInitiative = initiativeEdit->text().toInt();
    if( _chaser->getInitiative() != editInitiative)
    {
        _chaser->setInitiative(editInitiative);
        emit initiativeChanged(editInitiative);
    }
}

void ChaserWidget::exhaustionClicked()
{
    QObject* obj = sender();
    if(obj != 0)
    {
        if(obj == exhaustion5)
        {
            _chaser->setExhaustion(5);
        }
        else if(obj == exhaustion4)
        {
            _chaser->setExhaustion(4);
        }
        else if(obj == exhaustion3)
        {
            _chaser->setExhaustion(3);
        }
        else if(obj == exhaustion2)
        {
            _chaser->setExhaustion(2);
        }
        else if(obj == exhaustion1)
        {
            _chaser->setExhaustion(1);
        }
        else
        {
            _chaser->setExhaustion(0);
        }

        readData();
        emit exhaustionChanged(_chaser->getExhaustion());
    }
}

void ChaserWidget::dashEdited()
{
    _chaser->setDashCount(dashEdit->text().toInt());
}

void ChaserWidget::speedEdited()
{
    _chaser->setSpeed(speedEdit->text().toInt());
}

void ChaserWidget::buildWidget()
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);

    resetStyleSheet();

    QHBoxLayout* hLayoutTop = new QHBoxLayout();

    QLabel* nameLabel = new QLabel(QString("Name:"), this);
    nameLabel->setMaximumWidth(nameLabel->fontMetrics().width(nameLabel->text()));
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hLayoutTop->addWidget(nameLabel);

    nameEdit = new QLineEdit(this);
    nameEdit->setMaximumWidth(100);
    nameEdit->setFocusPolicy(Qt::ClickFocus);
    nameEdit->setReadOnly(true);
    hLayoutTop->addWidget(nameEdit);

    QVBoxLayout* vLayoutEdits = new QVBoxLayout();
    QHBoxLayout* hLayoutEditsTop = new QHBoxLayout();
    QHBoxLayout* hLayoutEditsBottom = new QHBoxLayout();

    QLabel* speedLabel = new QLabel(QString("Speed:"), this);
    speedLabel->setMaximumWidth(speedLabel->fontMetrics().width(speedLabel->text()));
    speedLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hLayoutEditsTop->addWidget(speedLabel);

    speedEdit = new QLineEdit(this);
    speedEdit->setMaximumWidth(30);
    speedEdit->setFocusPolicy(Qt::ClickFocus);
    speedEdit->setValidator(new QIntValidator(0, 1000000));
    connect(speedEdit,SIGNAL(editingFinished()),this,SLOT(speedEdited()));
    hLayoutEditsTop->addWidget(speedEdit);

    QLabel* initiativeLabel = new QLabel(QString("Init:"), this);
    initiativeLabel->setMaximumWidth(initiativeLabel->fontMetrics().width(initiativeLabel->text()));
    initiativeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hLayoutEditsTop->addWidget(initiativeLabel);

    initiativeEdit = new QLineEdit(this);
    initiativeEdit->setMaximumWidth(30);
    initiativeEdit->setFocusPolicy(Qt::ClickFocus);
    initiativeEdit->setValidator(new QIntValidator(0, 1000000));
    connect(initiativeEdit,SIGNAL(editingFinished()),this,SLOT(initiativeEdited()));
    hLayoutEditsTop->addWidget(initiativeEdit);

    QLabel* dashLabel = new QLabel(QString("Dash:"), this);
    dashLabel->setMaximumWidth(speedLabel->fontMetrics().width(speedLabel->text()));
    dashLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hLayoutEditsBottom->addWidget(dashLabel);

    dashEdit = new QLineEdit(this);
    dashEdit->setMaximumWidth(30);
    dashEdit->setFocusPolicy(Qt::ClickFocus);
    dashEdit->setValidator(new QIntValidator(0, 1000000));
    connect(dashEdit,SIGNAL(editingFinished()),this,SLOT(dashEdited()));
    hLayoutEditsBottom->addWidget(dashEdit);

    QLabel* positionLabel = new QLabel(QString("Pos:"), this);
    positionLabel->setMaximumWidth(positionLabel->fontMetrics().width(positionLabel->text()));
    positionLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hLayoutEditsBottom->addWidget(positionLabel);

    positionEdit = new QLineEdit(this);
    positionEdit->setMaximumWidth(30);
    positionEdit->setFocusPolicy(Qt::ClickFocus);
    positionEdit->setValidator(new QIntValidator(0, 1000000));
    connect(positionEdit,SIGNAL(editingFinished()),this,SLOT(positionEdited()));
    hLayoutEditsBottom->addWidget(positionEdit);

    vLayoutEdits->addLayout(hLayoutEditsTop);
    vLayoutEdits->addLayout(hLayoutEditsBottom);

    hLayoutTop->addLayout(vLayoutEdits);

    exhaustion1 = new QCheckBox(this);
    exhaustion1->setFocusPolicy(Qt::ClickFocus);
    connect(exhaustion1,SIGNAL(clicked()),this,SLOT(exhaustionClicked()));
    hLayoutTop->addWidget(exhaustion1);

    exhaustion2 = new QCheckBox(this);
    exhaustion2->setFocusPolicy(Qt::ClickFocus);
    connect(exhaustion2,SIGNAL(clicked()),this,SLOT(exhaustionClicked()));
    hLayoutTop->addWidget(exhaustion2);

    exhaustion3 = new QCheckBox(this);
    exhaustion3->setFocusPolicy(Qt::ClickFocus);
    connect(exhaustion3,SIGNAL(clicked()),this,SLOT(exhaustionClicked()));
    hLayoutTop->addWidget(exhaustion3);

    exhaustion4 = new QCheckBox(this);
    exhaustion4->setFocusPolicy(Qt::ClickFocus);
    connect(exhaustion4,SIGNAL(clicked()),this,SLOT(exhaustionClicked()));
    hLayoutTop->addWidget(exhaustion4);

    exhaustion5 = new QCheckBox(this);
    exhaustion5->setFocusPolicy(Qt::ClickFocus);
    connect(exhaustion5,SIGNAL(clicked()),this,SLOT(exhaustionClicked()));
    hLayoutTop->addWidget(exhaustion5);

    readData();

    setLayout(hLayoutTop);
}
